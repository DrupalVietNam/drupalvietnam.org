<?php
/**
 * Global settings file for Drupal.
 */

# $conf['pressflow_smart_start'] = TRUE;
$conf['stage_file_proxy_origin'] = 'http://drupalvietnam.org';
$conf['stage_file_proxy_hotlink'] = TRUE;
$conf['stage_file_proxy_use_imagecache_root'] = FALSE;
$conf['file_private_path'] = 'sites/drupalvietnam.org/files/private';
$conf['file_public_path'] = 'sites/drupalvietnam.org/files';

/**
 * In order to change your local settings (database info), don't modify this
 * file. You need to create a local.settings.php file with your own settings
 * that will be included.
 *
 * You can use example.settings.php as a starting point:
 *
 *     cp example.settings.php local.settings.php
 *
 * The local.settings.php will not be committed in the repository.
 */
if (file_exists(__DIR__ . '/local.settings.php')) {
  include __DIR__ . '/local.settings.php';
}

