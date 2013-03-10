<?php

// $Id$

/**
 * @file
 * Provides theming
 */


/**
 * Display file data for the passed in filepath
 *
 * @param string $vars
 * @return string
 */
function theme_ffmpeg_wrapper_ui_file_data($vars) {
  // Get the ffmpeg file data
  $file_data = ffmpeg_wrapper_get_file_data($vars['filepath']);
  $meta = array(
    t('File: !file', array('!file' => check_plain(basename($vars['filepath'])))),
    t('Format: !format', array('!format' => $file_data['format'])),
    t('Duration: !duration', array('!duration' => $file_data['timecode'])),
    t('Total bitrate: !rate', array('!rate' => $file_data['bitrate'])),
  );
  $video = array(
    t('Video codec: !codec', array('!codec' => $file_data['video']['codec'])),
    t('Frame size: !s', array('!s' => $file_data['video']['s'])),
    t('Video bit rate: !br', array('!br' => $file_data['video']['br'])),
    NULL,
  );
  $audio = array(
    t('Audio codec: !codec', array('!codec' => $file_data['audio']['codec'])),
    t('Sample rate: !ar', array('!ar' => $file_data['audio']['ar'])),
    t('Audio bit rate: !br', array('!br' => $file_data['audio']['ab'])),
    t('Chanels: !ac', array('!ac' => $file_data['audio']['ac'])),
  );
  $file = array ( array(
    'data' => t('Filepath: %path', array('%path' => $vars['filepath'])),
    'colspan' => 4
    )
  );
  $filesize = array ( array(
    'data' => t('Filesize: %path', array('%path' => format_size(filesize($vars['filepath'])))),
    'colspan' => 4
    )
  );
  // Build the rows for the display
  $rows = array($meta, $video, $audio, $file, $filesize);
  return theme('table', array('header' => array(), 'rows' => $rows));
}