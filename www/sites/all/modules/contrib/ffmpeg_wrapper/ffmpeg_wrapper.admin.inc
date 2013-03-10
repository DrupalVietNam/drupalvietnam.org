<?php

/**
 * @files
 * Provides administration callbacks
 */


/**
 * Default administration form
 */
function ffmpeg_wrapper_admin() {
  // Always clear caches to catch any changes to the ffmpeg binary
  variable_del('ffmpeg_wrapper_codecs');
  variable_del('ffmpeg_wrapper_formats');

  $form['ffmpeg_wrapper'] = array(
    '#type' => 'fieldset',
    '#title' => t('FFmpeg binary path'),
  );

  $form['ffmpeg_wrapper']['ffmpeg_wrapper_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to the binary'),
    '#default_value' => variable_get('ffmpeg_wrapper_path', '/usr/bin/ffmpeg'),
    '#description' => t('Path to the FFmpeg binary. Either an absolute path
      or a path relative to your Drupal installation.'),
  );

  // Get the FFmpeg information
  $ffmpeg = new ffmpeg_wrapper();
  if (count($ffmpeg->errors)) {
    drupal_set_message(implode('<p>', $ffmpeg->errors), 'error');
  }
  // Only display if we can reach the binary
  else {
    $form['ffmpeg_wrapper']['ffmpeg_wrapper_version'] = array(
      '#markup' => '<pre>' . $ffmpeg->get_version('raw') . '</pre>',
    );

    $form['ffmpeg_wrapper_formats'] = array(
      '#type' => 'fieldset',
      '#title' => t('File formats'),
      '#description' =>  t('File formats that the installed version of FFmpeg supports.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['ffmpeg_wrapper_formats']['ffmpeg_wrapper_formats'] = array(
      '#markup' => ffmpeg_wrapper_formats_display(),
    );

    $form['ffmpeg_wrapper_codecs']= array(
      '#type' => 'fieldset',
      '#title' => t('Installed Codecs'),
      '#description' =>  t('FFmpeg was either compiled with these codecs, or
        these are the codecs available on your system'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['ffmpeg_wrapper_codecs']['ffmpeg_wrapper_codecs'] = array(
      '#markup' => ffmpeg_wrapper_codecs_display(),
    );
  }

  return system_settings_form($form);
}


/**
 * Validate the options on the ffmpeg form
 *
 * @param int $form_id
 * @param array $form_values
 */
function ffmpeg_wrapper_admin_validate($form, &$form_state) {
  // Make sure we've got the path to the ffmpeg binary
  if (! file_exists($form_state['values']['ffmpeg_wrapper_path'])) {
    form_set_error('ffmpeg_wrapper_path', t('FFmpeg binary was not found on the
      path you specified. Maybe try a different path?'));
  }
  elseif (! is_executable($form_state['values']['ffmpeg_wrapper_path'])) {
    form_set_error('ffmpeg_wrapper_path', t('The FFmpeg binary that you specified
      is not executable. Perhaps the webserver does not have rights to execute it?'));
  }
}