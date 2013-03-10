<?php

// $Id$

/**
 * @file
 * This is a ffmpeg controller class.
 */
class ffmpeg_wrapper {

  /**
   * Construct a new ffmpeg object
   *
   * @param $command
   *   String, default command to run
   */
  function __construct($command = '') {
    // Initialize errors
    $this->errors = array();
    // Do we have a set path?
    if (! is_executable(variable_get('ffmpeg_wrapper_path', '/dev/null'))) {
      if (! ffmpeg_wrapper_which_ffmpeg($this)) {
        return FALSE;
      }
    }
    // Set the path
    $this->path = variable_get('ffmpeg_wrapper_path', '/dev/null');

    if ($command) {
      $this->run_command($command);
    }

  }


  /**
   * Runs the requested command. Assumes input is sanitized.
   *
   * @NOTE SECURITY ISSUE WITH COMMAND LINE ACCESS. SORRY.
   */
  function run_command($command = '', $verbosity = 1) {

    // Check and see if anyone is overridding this ffmpeg binary
    // @TODO should this be an alter function?
    if ($function = variable_get('ffmpeg_wrapper_engine', FALSE)) {
      if (function_exists($function)) {
        return $function($command, $this);
      }
    }

    if ($command || empty($this->command)) {
      $this->command = $command;
    }

    // Purge any errors our output that might have already been created during
    // this thread
    $this->errors = array();
    $this->output = '';
    // Build the command
    $this->command = $this->path . " $this->command -v $verbosity ";
    // print_r($this->command);
    // Keep track of where this is being executed
    $this->cwd = getcwd();

    $descriptor_spec = array(
      0 => array('pipe', 'r'),
      1 => array('pipe', 'w'),
      2 => array('pipe', 'w')
    );

    $pipes = array();
    $process = proc_open($this->command, $descriptor_spec, $pipes, $this->cwd, NULL, array('binary_pipes' => TRUE));

    if (is_resource($process)) {
      // If a file is being transcoded we want to store progress in Drupal's
      // cache. We use this so an ajax status callback can get the current
      // transcoding status of this file. We can only do this if this is a
      // first class drupal file.
      if (! empty($this->file->fid)) {

        $stderr = '';
        $cid = 'ffmpeg_wrapper_transcoding_progress_' . $this->file->fid;

        $data = array(
          'file_duration' => $this->file_data['duration'],
          'completed' => 0,
        );
        stream_set_blocking($pipes[2], 0);
        while (! feof($pipes[2])) {
          $current = fgets($pipes[2]);

          if (empty($current)) {
            continue;
          }

          $stderr .= $current;

          // Current time maybe in the format (modern git version):
          // frame= 124 fps= 0 q=31.0 size= 514kB time=00:00:04.96 bitrate= 849.1kbits/s
          if (preg_match_all("/frame=.*time=([0-9\:]*?) bitrate/", $stderr, $matches)) {
            $data['completed'] = $this->parse_timecode($matches[1][count($matches[1]) - 1]);
          }
          // Current time maybe in the format (very old OSX ffmpeg version):
          // frame= 107 q=30.4 size= 463kB time=4.3 bitrate= 886.6kbits/s
          // frame= 1474 q=31.0 size= 3585kB time=59.0 bitrate= 498.2kbits/s
          elseif (preg_match_all("/frame=.*time=([0-9]*?)\.[0-9] bitrate/", $stderr, $matches)) {
            $data['completed'] = $matches[1][count($matches[1]) - 1];
          }

          // If we have updated status, set the cache
          if ($data['completed']) {
            cache_set($cid, $data);
          }

        }
        // Ensure that we are not storing an extra cache item in the DB
        cache_clear_all($cid, 'cache');
      }


      // Not transcoding, no need to save progress
      else {
        $stderr = stream_get_contents($pipes[2]);
      }

      fclose($pipes[0]);

      $this->output = stream_get_contents($pipes[1]);
      fclose($pipes[1]);

      $this->output .= $stderr;
      fclose($pipes[2]);

      $command_return = proc_close($process);
    }

    // Parse errors
    $this->error_check();

    // Return the output
    return $this->output;
  }


  /**
   * Check the output from FFmpeg for various strings
   */
  function error_check() {
    // Error conditions these are all pulled by hand at this point
    // @NOTE one has to be careful to have a specific match as there are strings
    // in the output from #ffmpeg which containg "error" which are not fatal
    $errors = array(
      '/Segmentation fault .*/i',
      '/Unsupported .*/i',
      // match: bad formats
      '/Unknown format .*/i',
      '/Unable for find a suitable output format for .*/i',
      '/Incorrect frame size .*/i',
      '/Unsupported codec .*/i',
      '/Could not write header .*/i',
      '/already exists. Overwrite/i',
      '/sh: [0-9a-zA-Z\/]*: not found .*/i',
      '/no such file or directory .*/i',
      '/.*does not support that sample rate.*/i',
      // match: Error while opening codec for output stream #0.1 - maybe
      // incorrect parameters such as bit_rate, rate, width or height
      '/Error while opening codec for output stream.*/i',
      '/Error while opening encoder.*/i',
      // match: I/O error occured
      '/I\/O error occured.*/i',
      // This usually follows the I/O error
      '/.*input file is truncated and\/or corrupted/i',
      '/Failed to add video hook function*/i',
      '/ffmpeg: unrecognized option*/i',
      '/Could not open */i',
      // Does the file exist?
      '/File.*already exists. Overwrite \?*/i',
      //'/At least one output file must be specified/i'
    );

    // Check for error conditions in the FFmpeg output
    foreach ($errors as $error) {
      preg_match($error, $this->output, $matches);
      if (count($matches)) {
        $this->errors[] = print_r($matches[0], TRUE);
      }
    }

    // Check to see that a viable file was created
    if (! empty($this->output_file)) {
      // Does the new file exists?
      if (! file_exists($this->output_file)) {
     //   $this->errors[] = t('FFmpeg failed to create an output file. This could be due to permission problems or an error from the encoding settings.');
      }
      // check to see that the file had data
      elseif (filesize($this->output_file) < 10 ) {
     //   $this->errors[] = t('FFmpeg created a file, but it does not have any data. This could be a problem with the encoding settings.');
      }
    }
  }


  /**
   * Returns the installed version of FFmpeg
   * @param $format
   *   one of raw, version
   * @return $version
   *   String, FFmpeg version
   */
  function get_version($format = 'version') {
    $output = $this->run_command('-version');

    // Just return the raw version info
    if ($format == 'raw') {
      return $output;
    }

    // Search for SVN string
    // FFmpeg version SVN-r20438, Copyright (c) 2000-2009 Fabrice Bellard, et al.
    $pattern = "/ffmpeg version SVN-r([0-9.]*)/i";
    if (preg_match($pattern, $output, $matches)) {
      return $matches[1];
    }

    // Some OSX versions are built from a very early CVS
    // I do not know what to do with this version- using 1 for now
    $pattern = "/ffmpeg version.*CVS.*Mac OSX universal/msUi";
    if (preg_match($pattern, $output, $matches)) {
      return 0;
    }

    // Search for git string
    // FFmpeg version git-N-29240-gefb5fa7, Copyright (c) 2000-2011 the FFmpeg developers.
    // ffmpeg version N-31145-g59bd0fe, Copyright (c) 2000-2011 the FFmpeg developers
    $pattern = "/ffmpeg version.*N-([0-9.]*)/i";
    if (preg_match($pattern, $output, $matches)) {
      // Versions above this seem to be ok
      if ($matches[1] >= 29240) {
        return array(
          'svn' => (int) $matches[1],
          'version' => NULL, // Return NULL as there appears to be no version number.
        );
      }
    }

    // Do we have a release?
    // ffmpeg version 0.4.9-pre1, build 4736, Copyright (c) 2000-2004 Fabrice Bellard
    $pattern = "/ffmpeg version ([0-9.]*)/i";
    if (preg_match($pattern, $output, $matches)) {
      return $matches[1];
    }

    // Do we have a build version?
    // ffmpeg version 0.4.9-pre1, build 4736, Copyright (c) 2000-2004 Fabrice Bellard
    $pattern = "/ffmpeg version.*, build ([0-9]*)/i";
    if (preg_match($pattern, $output, $matches)) {
      return $matches[1];
    }

    return FALSE;
  }


  /**
   * Acquires all of the codes in use
   * @return unknown_type
   */
  function get_codecs() {
    // Can we utilize cached data?
    if ($codecs = variable_get('ffmpeg_wrapper_codecs', FALSE)) {
      return $codecs;
    }

    $data = array();

    // We know where the codecs are by looking at the output of
    // ffmpeg -formats or ffmpeg -codecs depending of
    // version SVN-r > 20561 or version > 0.5
    $ffmpeg_version = $this->get_version();
    if (((int) $ffmpeg_version >= 20561) || ((int) $ffmpeg_version < 1000 && (float) $ffmpeg_version > 0.5)) {
      $codecs = $this->run_command('-codecs');
      // Remove the leading space
      $codecs_formats_pos = stripos($codecs, 'Codecs:') + 8;
      $codecs_formats_pos_end = stripos($codecs, 'Note,');
    }
    else {
      $codecs = $this->run_command('-formats');
      // Remove the leading space
      $codecs_formats_pos = stripos($codecs, 'Codecs:') + 8;
      $codecs_formats_pos_end = stripos($codecs, 'Supported file protocols:');
    }
    $codecs = drupal_substr($codecs, $codecs_formats_pos, ($codecs_formats_pos_end - $codecs_formats_pos));

    // Convert to array
    $codecs = explode("\n", $codecs);

    $rows = array();
    foreach ($codecs as $codec) {
      // Match the decode, encode, type, S|D|T options (see: http://lists.mplayerhq.hu/pipermail/ffmpeg-user/2006-January/002003.html)
      // S = Draw horizontal band (decoder can use the draw_horiz_band callback)
      // D = DR1 (uses direct rendering method 1)
      // T = Truncated (input bitstream might be truncated at a random location instead of on frame boundary)

      // name
      $pattern ='/[ ]*([D ])([E ])([ VA])([S ])([ D])([ T])[ ]*([a-zA-Z0-9_,]*)[ ]*([a-zA-Z0-9,_ ]*)/';
      if (preg_match($pattern, $codec, $matches)) {
        list ($decode, $encode, $type, $s, $d, $t, $format) = array_slice($matches, 1);
        if ($type) {
          $data[$format] = array(
            'format' => $format,
            'type' => ($type == 'A' ? t('Audio') : t('Video')),
            'encode' => ($encode == 'E' ? TRUE : FALSE),
            'decode' => ($decode == 'D' ? TRUE : FALSE),
          );
        }
      }
    }
    $this->codecs = $data;
    variable_set('ffmpeg_wrapper_codecs', $data);
    return $data;
  }

  /**
   * Acquire formats that can be processed
   * @return array
   */
  function get_formats($reset = FALSE) {
    // Can we utilize cached data?
    if (! $reset && $formats = variable_get('ffmpeg_wrapper_formats', FALSE)) {
      $this->formats = $formats;
      return $formats;
    }

    $formats = $this->run_command('-formats');

    // Slice up the format output
    $startpos = stripos($formats, 'File formats:');
    // Depending of version SVN-r > 20561 or version > 0.5 string changes
    $ffmpeg_version = $this->get_version();

    if (((int) $ffmpeg_version >= 20561) || ((int) $ffmpeg_version < 1000 && (float) $ffmpeg_version > 0.5)) {
      $endpos = stripos($formats, 'FFmpeg version');
    }
    else {
      $endpos = stripos($formats, 'Codecs:');
    }

    $formats = drupal_substr($formats, $startpos, $endpos - $startpos);
    // remove the header
    $formats = str_replace('File formats:', '', $formats);

    $rows = array();
    $data = array();
    foreach (explode("\n", $formats) as $format) {
      // match the decode, encode, format, description
      $pattern ='/[ ]*([D ])([E ])[ ]*([a-zA-Z0-9_,]*)[ ]*([^\$]*)/';
      if (preg_match($pattern, $format, $matches)) {
        // Get the matches into variable format, note that $matches[0] is not wanted
        list ($decode, $encode, $types, $description) = array_slice($matches, 1);
        // There can be multiple types per format
        $types = explode(',', $types);
        foreach ($types as $type) {
          if ($type) {
            if (isset($data[$type])) {
              $data[$type] = array();
            }
            $data[$type]['format'] = drupal_strtolower($type);
            $data[$type]['description'] = $description;

            // It can be the case that we have duplicate types
            // provided different codecs. Make sure that we do
            // not overwrite an existing 'Yes'
            if (! (isset($data[$type]['decode']) && $decode != 'D')) {
              $data[$type]['decode'] = ($decode == 'D' ? TRUE : FALSE);
            }
            if (! (isset($data[$type]['encode']) && $encode != 'E')) {
              $data[$type]['encode'] = ($encode == 'E' ? TRUE : FALSE);
            }
          }
        }
      }
    }

    $this->formats = $data;
    // Cache data
    variable_set('ffmpeg_wrapper_formats', $data);
    return $data;
  }


  /**
   * Return meta data for this file. Escapes filepath
   *
   * @param $file
   *   String, path to file
   * @return $file
   *   Array, contains meta data
   */
  function get_file_data($filepath) {
    if (! empty($this->file_data)) {
      return $this->file_data;
    }
    if (file_exists($filepath)) {
      // Do we need to escape the path? Check for ' and "
      $pattern = "/^'|\".*'|\"$/";
      if (! preg_match($pattern, $filepath)) {
        $filepath = escapeshellarg($filepath);
      }

      $this->run_command("-i $filepath");

      // Format
      $pattern = '/Input #0, (.*),/';
      preg_match($pattern, $this->output, $matches);
      $file['format'] = isset($matches[1]) ? $matches[1] : 'N/A';

      // File duration
      $file['duration'] = $this->get_file_duration(NULL, NULL, $this->output);
      $file['timecode'] = $this->get_file_duration(NULL, TRUE, $this->output);

      // Bit rate
      $pattern = "/bitrate: ([0-9].*\/s)/";
      preg_match($pattern, $this->output, $matches);
      $file['bitrate'] = isset($matches[1]) ? $matches[1] : 'N/A';

      // Audio ----------------------------------------
      // Format is: codec, sample rate, channels, flt, bitrate
      $pattern = "/Audio: (.*), ([0-9]*) Hz, (stereo|mono|[0-9]).*?,.*?([0-9]* kb\/s)/";
      // Format is: codec, sample rate, channels
      $pattern2 = "/Audio: (.*), ([0-9]*) Hz, (stereo|mono)/";

      $file['audio']['codec'] = '';
      $file['audio']['ar'] = '';
      $file['audio']['ac'] = '';
      $file['audio']['ab'] = '';

      if (preg_match($pattern, $this->output, $matches)) {
        $file['audio']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        $file['audio']['ar'] = !empty($matches[2]) ? $matches[2] : 'N/A';
        $file['audio']['ac'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['audio']['ab'] = !empty($matches[4]) ? $matches[4] : 'N/A';
      }
      else if (preg_match($pattern2, $this->output, $matches)) {
        $file['audio']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        $file['audio']['ar'] = !empty($matches[2]) ? $matches[2] : 'N/A';
        $file['audio']['ac'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['audio']['ab'] = 'N/A';
      }
      else {
        $this->errors[] = t('Was unable to get any audio data back from FFmpeg.');
      }

      // Ensure that we have an integer value for channels
      if (isset($file['audio']['ac'])) {
        switch ($file['audio']['ac']) {
          case 'mono':
            $file['audio']['ac'] = 1;
          break;
          case 'stereo':
            $file['audio']['ac'] = 2;
          break;
        }
      }

      // VIDEO ----------------------------------------
      // The formating of video can be difficult. We use different patterns to
      // look for the video information for different versions of ffmpeg

      // Pattern that takes into account for a very specific issue with mpeg4 with
      // recent compiled versions of ffmpeg
      // Video: mpeg4 (Advanced Simple Profile), yuv420p, 768x432 [SAR 1:1 DAR 16:9], 198 kb/s, 25 fps, 25 tbr, 2500 tbn, 1k tbc
      $pattern0 = "/Video: ([^,]+) \(Advanced Simple Profile\), ([^,]+), ([0-9x]+)[^,]*, ([0-9]*.*\/s|[A-Za-z]+[^,]*), ([0-9\.]*)/";

      // Pattern that takes into account a codec's color space (example: yuv420p)
      // eg: Video: mpeg1video, yuv420p, 320x240 [PAR 1:1 DAR 4:3], 990 kb/s, 30.00 tb(r)
      // the above is: codec, color space, frame size, bitrate, frame rate
      $pattern1 = "/Video: ([^,]+), ([^,]+), ([0-9x]+)[^,]*, ([0-9]*.*\/s|[A-Za-z]+[^,]*), ([0-9\.]*)/";

      // Pattern that omits video bitrate but not color space.
      // eg: Video: mpeg4, yuv420p, 640x480 [PAR 1:1 DAR 4:3], 23.98 tb(r)
      // the above is: codec, color space, frame size, frame rate
      $pattern2 = "/Video: ([^,]+), ([^0-9][^,]*), ([0-9x]+)[^,]*, ([0-9\.]*)/";

      // Pattern that omits a codec's color space and video bitrate
      // eg: Video: mpeg4, 640x480, 29.97 tb(r)
      // the above is: codec, frame size, frame rate
      $pattern3 = "/Video: ([^,]+), ([0-9x]+)[^,]*, ([0-9\.]*)/";

      // Pattern that looks for OSX specific output
      // eg: Stream #0.1(eng), 25.00 fps(r): Video: mpeg4, yuv420p, 768x432
      // the above is: frame rate, codec, colour space, frame size
      $pattern4 = "/Video: ([^,]+), ([^,]+), ([0-9x]+)[^,]/";

      // Search for the data
      if (preg_match($pattern0, $this->output, $matches)) {
        $file['video']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        // $file['video']['type'] = $matches[2];
        $file['video']['s'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['video']['br'] = !empty($matches[4]) ? $matches[4] : 'N/A';
      }
      elseif (preg_match($pattern1, $this->output, $matches)) {
        $file['video']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        // $file['video']['type'] = $matches[2];
        $file['video']['s'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['video']['br'] = !empty($matches[4]) ? $matches[4] : 'N/A';
      }
      elseif (preg_match($pattern2, $this->output, $matches)) {
        $file['video']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        $file['video']['s'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['video']['br'] = 'N/A';
      }
      elseif (preg_match($pattern3, $this->output, $matches)) {
        $file['video']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        $file['video']['s'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['video']['br'] = 'N/A';
      }
      elseif (preg_match($pattern4, $this->output, $matches)) {
        $file['video']['codec'] = !empty($matches[1]) ? $matches[1] : 'N/A';
        $file['video']['s'] = !empty($matches[3]) ? $matches[3] : 'N/A';
        $file['video']['br'] = 'N/A';
      }
      else {
        $this->errors[] = t('Was unable to get any video data back from FFmpeg.');
      }
      $this->file_data = $file;
      return $file;
    }
  }


  /**
   * Returns file duration
   *
   * @param $path
   *   String, path to file
   * @param $timecode
   *   Boolean, return time format or unix time
   * @param $output
   *   String, output from FFmpeg, useful for not running FFmpeg again
   * @return $time
   *   String, unix time or timecode
   */
  function get_file_duration($path = NULL, $timecode = NULL, $output = NULL) {
    // Should we use the default file?
    if (! $path && ! $output && ! empty($this->input_file)) {
      $path = $this->input_file;
    }
    // Was output from FFmpeg passed in?
    if (! $output) {
      // Get duration from ffmpeg
      // Need quotes around the path parameter in case filename has spaces.
      $ffmpeg_output = ffmpeg_wrapper_run_command("-i \"$path\"");
      $this->run_command("-i \"$path\"");
      $output = $this->output;
    }
    // Pattern to find the file duration
    $pattern = "/Duration: ([0-9]+:[0-9]+:[0-9]+)\.[0-9]+/";
    if (preg_match($pattern, $output, $matches)) {
      if ($timecode) {
        return $matches[1];
      }
      return $this->parse_timecode($matches[1]);
    }
    return FALSE;
  }


  /**
   * Utility function to parse a timecode string
   *
   * @param type $string
   *   HH:MM:SS
   * @return type
   */
  private function parse_timecode($string) {
    // Time is in HH:MM:SS. strtotime returns this value + the current
    // unix time. We subtract today to get the total number in seconds
    return strtotime($string) - strtotime('TODAY');
  }


  /**
   * Are the necessary codecs installed to decode this file?
   * @param $path
   *   String, path to file
   * @return
   *   Boolean
   */
  function can_decode_file($path) {
    // Setup the types of codecs/formats that we need to check
    $types = array('audio' => array(), 'video' => array());

    // Get supported formats
    $formats = $this->get_formats();
    // Get the format and codec information on this file
    $file_data = $this->get_file_data($path);
     // If the file has multiple formats, all formats
     // must be in $file_types Otherwise, it is
     // not supported by ffmpeg.
    $decode = FALSE;
    foreach (explode(',', $file_data['format']) as $type) {
      if (isset($formats[drupal_strtolower($type)]['decode'])) {
        if ($formats[drupal_strtolower($type)]['decode']) {
          $decode = TRUE;
          break;
        }
      }
    }

    if (! $decode) {
      $this->errors[] = t('The file could not be transcoded because this
        version of FFmpeg can not decode this kind of file');
      return FALSE;
    }

    // If we have na values for the audio and video codecs, the file can not be decoded
    // @TODO is this really TRUE? The logic below does not seem to indicate this,
    //       however, I might not understand what the 'na' value really means-
    //       na could be "not available" or "not applicable"
    if ($file_data['audio']['codec'] == 'N/A' && $file_data['video']['codec'] == 'N/A') {
      $this->errors[] = t('The file could not be transcoded because the audio
        and video codes do not exist');
      return FALSE;
    }

    // Now get installed codecs to compare file codecs
    $codecs = $this->get_codecs();

    // Can we decode this audio type?
    if (isset($codecs[$file_data['audio']['codec']]['decode']) || $file_data['audio']['codec'] == 'N/A' ) {
      // Are we only handing back audio?
      if ($types['audio'] && ! $types['video']) {
        return TRUE;
      }
      $can_audio_decode = TRUE;
    }
    else {
      $this->errors[] = t('The file could not be transcoded because the audio codec is not supported');
    }

    // Can we decode this video type?
    if ($codecs[$file_data['video']['codec']]['decode'] || $file_data['video']['codec'] == 'N/A') {
      // are we only handing back video?
      if ($types['video'] && ! $types['audio']) {
        return TRUE;
      }
      $can_video_decode = TRUE;
    }
    else {
      $this->errors[] = t('The file could not be transcoded because the video codec is not supported');
    }

    // If we can decode both audio and video, the file is ok
    if ($can_audio_decode && $can_video_decode) {
      return TRUE;
    }

    // @TODO need additional debugging information here
    return FALSE;
  }


  /**
   * Utility function to track progress during a transcoding operation
   *
   * @param type $pipes
   */
   private function transcoding_progress($pipes) {
     $stderr = '';
     $progress = 0;
     $current_time = 0;

     // Cache ID
     $cid = 'ffmpeg_wrapper_transcoding_progress_' . $this->file->fid;
     // Get the total length of this file in seconds

     $file_duration = strtotime($this->get_file_duration());
     // $file_duration = 0;

     stream_set_blocking($pipes[2], 0);

     while (! feof($pipes[2])) {
        $current = fgets($pipes[2]);
        $stderr .= $current;

        if (empty($current) || $current == $last) {
          continue;
        }

        //$duration = ($duration) ? $duration : $this->parse_duration($stderr, FALSE);

        // Search for the time string in the output:
        // frame= 3179 fps=282 q=31.0 Lsize=    5738kB time=00:02:07.11 bitrate= 369.8kbits/s
        if (preg_match_all("/time=([0-9]+:[0-9]+:[0-9]+).*bitrate/i", $stderr, $matches)) {
          $current_time = $this->parse_timecode($matches[1]);
          $progress = round($current_time / $file_duration * 100);
          cache_set($cid, $progress);
        }

        $last = $current;
      }
      // Ensure that we are not storing an extra cache item in the DB
      // cache_clear_all($cid, 'cache');

      return $stderr;
  }

}