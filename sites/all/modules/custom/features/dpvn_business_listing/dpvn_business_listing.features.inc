<?php
/**
 * @file
 * dpvn_business_listing.features.inc
 */

/**
 * Implements hook_views_api().
 */
function dpvn_business_listing_views_api() {
  list($module, $api) = func_get_args();
  if ($module == "views" && $api == "views_default") {
    return array("version" => "3.0");
  }
}

/**
 * Implements hook_node_info().
 */
function dpvn_business_listing_node_info() {
  $items = array(
    'business' => array(
      'name' => t('Business Listing'),
      'base' => 'node_content',
      'description' => t('Business listing.'),
      'has_title' => '1',
      'title_label' => t('Business'),
      'help' => '',
    ),
  );
  return $items;
}
