<?php
/**
 * @file
 * dpvn.features.filter.inc
 */

/**
 * Implements hook_filter_default_formats().
 */
function dpvn_filter_default_formats() {
  $formats = array();

  // Exported format: MarkDown
  $formats['markdown'] = array(
    'format' => 'markdown',
    'name' => 'MarkDown',
    'cache' => '1',
    'status' => '1',
    'weight' => '-10',
    'filters' => array(
      'filter_markdown' => array(
        'weight' => '0',
        'status' => '1',
        'settings' => array(),
      ),
    ),
  );

  return $formats;
}
