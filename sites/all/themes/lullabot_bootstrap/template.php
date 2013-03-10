<?php

$theme_path = drupal_get_path('theme', 'lullabot_bootstrap');
include_once($theme_path . '/includes/lullabot_bootstrap.inc');
include_once($theme_path . '/includes/modules/theme.inc');
include_once($theme_path . '/includes/modules/form.inc');
include_once($theme_path . '/includes/modules/menu.inc');
include_once($theme_path . '/includes/modules/views.inc');


/**
 * hook_theme() 
 */
function lullabot_bootstrap_theme() {
  return array(
    'lullabot_bootstrap_links' => array(
      'variables' => array('links' => array(), 'attributes' => array(), 'heading' => NULL),
    ),
    'lullabot_bootstrap_btn_dropdown' => array(
      'variables' => array('links' => array(), 'attributes' => array(), 'type' => NULL),
    ), 
  );
}

/**
 * Preprocess variables for html.tpl.php
 *
 * @see system_elements()
 * @see html.tpl.php
 */
function lullabot_bootstrap_preprocess_html(&$variables) {
   // Try to load the library, if the twitter_bootstrap_ui module is in use.
  if (module_exists('twitter_bootstrap_ui')) {
    $library = libraries_load('twitter_bootstrap', 'minified');
  }  
}

/**
 * Override theme_breadrumb().
 *
 * Print breadcrumbs as a list, with separators.
 */
function lullabot_bootstrap_breadcrumb($variables) {
  $breadcrumb = $variables['breadcrumb'];

  if (!empty($breadcrumb)) {
    $breadcrumbs = '<ul class="breadcrumb">';
    
    $count = count($breadcrumb) - 1;
    foreach ($breadcrumb as $key => $value) {
      if ($count != $key) {
        $breadcrumbs .= '<li>' . $value . '<span class="divider">/</span></li>';
      }
      else{
        $breadcrumbs .= '<li>' . $value . '</li>';
      }
    }
    $breadcrumbs .= '</ul>';
    
    return $breadcrumbs;
  }
}

/**
 * Preprocess variables for node.tpl.php
 *
 * @see node.tpl.php
 */
function lullabot_bootstrap_preprocess_node(&$variables) {
  if ($variables['teaser']) {
    $variables['classes_array'][] = 'row-fluid';
  }
}

/**
 * Preprocess variables for block.tpl.php
 *
 * @see block.tpl.php
 */
function lullabot_bootstrap_preprocess_block(&$variables, $hook) {
  $variables['title_attributes_array']['class'][] = 'block-title';
}


/**
 * Preprocess variables for page.tpl.php
 *
 * @see page.tpl.php
 */
function lullabot_bootstrap_preprocess_page(&$variables) {
  // Add information about the number of sidebars.
  if (!empty($variables['page']['sidebar_first']) && !empty($variables['page']['sidebar_second'])) {
    $variables['columns'] = 3;
  }
  elseif (!empty($variables['page']['sidebar_first'])) {
    $variables['columns'] = 2;
  }
  elseif (!empty($variables['page']['sidebar_second'])) {
    $variables['columns'] = 2;
  }
  else {
    $variables['columns'] = 1;
  }
  
  // Make sure the menu module is in use before setting menu vars.
  if (module_exists('menu')) {
    // Primary nav
    $variables['primary_nav'] = FALSE;
    if ($variables['main_menu']) {
      // Build links
      $tree = menu_tree_page_data(variable_get('menu_main_links_source', 'main-menu'));
      $variables['main_menu'] = lullabot_bootstrap_menu_navigation_links($tree);
    
      // Build list
      $variables['primary_nav'] = theme('lullabot_bootstrap_links', array(
        'links' => $variables['main_menu'],
        'attributes' => array(
          'id' => 'main-menu',
          'class' => array('nav'),
        ),
        'heading' => array(
          'text' => t('Main menu'),
          'level' => 'h2',
          'class' => array('element-invisible'),
        ),
      ));
    }
  
    // Secondary nav
    $variables['secondary_nav'] = FALSE;
    if ($variables['secondary_menu']) {
      $secondary_menu = menu_load(variable_get('menu_secondary_links_source', 'user-menu'));
    
      // Build links
      $tree = menu_tree_page_data($secondary_menu['menu_name']);
      $variables['secondary_menu'] = lullabot_bootstrap_menu_navigation_links($tree);
    
      // Build list
      $variables['secondary_nav'] = theme('lullabot_bootstrap_btn_dropdown', array(
        'links' => $variables['secondary_menu'],
        'label' => $secondary_menu['title'],
        'type' => 'success',
        'attributes' => array(
          'id' => 'user-menu',
          'class' => array('pull-right'),
        ),
        'heading' => array(
          'text' => t('Secondary menu'),
          'level' => 'h2',
          'class' => array('element-invisible'),
        ),
      ));
    }

    // Replace tabs with dropw down version
    $variables['tabs']['#primary'] = _lullabot_bootstrap_local_tasks($variables['tabs']['#primary']);
  }
}


/**
 * Returns the correct span class for a region
 */
function _lullabot_bootstrap_content_span($columns = 1) {
  $class = FALSE;
  
  switch ($columns) {
    case 1:
      $class = 'span12';
      break;
    case 2:
      $class = 'span9';
      break;
    case 3:
      $class = 'span6';
      break;
  }
  
  return $class;
}



/**
 * Returns HTML for a query pager. Override of theme_pager().
 *
 * Overridden to change some class names. Bootstrap uses some of the same
 * classes for different types of elements, and offers other classes are used
 * for pagers. In particular, the 'pager' class (used in core) is for rounded
 * buttons, while Bootstrap uses the 'pagination' class for multi-button pagers
 * like these.
 *
 * Also format the current page number as a link so that it will be themed like
 * the other links around it.
 *
 * @param $vars
 *   An associative array containing:
 *   - tags: An array of labels for the controls in the pager.
 *   - element: An optional integer to distinguish between multiple pagers on
 *     one page.
 *   - parameters: An associative array of query string parameters to append to
 *     the pager links.
 *   - quantity: The number of pages in the list.
 *
 * @ingroup themeable
 */
function lullabot_bootstrap_pager($variables) {
  $tags = $variables['tags'];
  $element = $variables['element'];
  $parameters = $variables['parameters'];
  $quantity = $variables['quantity'];
  global $pager_page_array, $pager_total;

  // Calculate various markers within this pager piece:
  // Middle is used to "center" pages around the current page.
  $pager_middle = ceil($quantity / 2);
  // current is the page we are currently paged to
  $pager_current = $pager_page_array[$element] + 1;
  // first is the first page listed by this pager piece (re quantity)
  $pager_first = $pager_current - $pager_middle + 1;
  // last is the last page listed by this pager piece (re quantity)
  $pager_last = $pager_current + $quantity - $pager_middle;
  // max is the maximum page number
  $pager_max = $pager_total[$element];
  // End of marker calculations.

  // Prepare for generation loop.
  $i = $pager_first;
  if ($pager_last > $pager_max) {
    // Adjust "center" if at end of query.
    $i = $i + ($pager_max - $pager_last);
    $pager_last = $pager_max;
  }
  if ($i <= 0) {
    // Adjust "center" if at start of query.
    $pager_last = $pager_last + (1 - $i);
    $i = 1;
  }
  // End of generation loop preparation.

  $li_first = theme('pager_first', array('text' => (isset($tags[0]) ? $tags[0] : t('« first')), 'element' => $element, 'parameters' => $parameters));
  $li_previous = theme('pager_previous', array('text' => (isset($tags[1]) ? $tags[1] : t('‹ previous')), 'element' => $element, 'interval' => 1, 'parameters' => $parameters));
  $li_next = theme('pager_next', array('text' => (isset($tags[3]) ? $tags[3] : t('next ›')), 'element' => $element, 'interval' => 1, 'parameters' => $parameters));
  $li_last = theme('pager_last', array('text' => (isset($tags[4]) ? $tags[4] : t('last »')), 'element' => $element, 'parameters' => $parameters));

  if ($pager_total[$element] > 1) {
    if ($li_first) {
      $items[] = array(
        'class' => array('pager-first'),
        'data' => $li_first,
      );
    }
    if ($li_previous) {
      $items[] = array(
        'class' => array('prev'),
        'data' => $li_previous,
      );
    }

    // When there is more than one page, create the pager list.
    if ($i != $pager_max) {
      /* Don't use ellipsis
      if ($i > 1) {
        $items[] = array(
          'class' => array('pager-ellipsis'),
          'data' => '…',
        );
      }
      */
      // Now generate the actual pager piece.
      for (; $i <= $pager_last && $i <= $pager_max; $i++) {
        if ($i < $pager_current) {
          $items[] = array(
            'class' => array('pager-item'),
            'data' => theme('pager_previous', array('text' => $i, 'element' => $element, 'interval' => ($pager_current - $i), 'parameters' => $parameters)),
          );
        }
        if ($i == $pager_current) {
          $items[] = array(
            'class' => array('active'), // Add the active class
            'data' => l($i, '#', array('fragment' => '', 'external' => TRUE)),
          );
        }
        if ($i > $pager_current) {
          $items[] = array(
            'class' => array('pager-item'),
            'data' => theme('pager_next', array('text' => $i, 'element' => $element, 'interval' => ($i - $pager_current), 'parameters' => $parameters)),
          );
        }
      }
      /* Don't use ellipsis
      if ($i < $pager_max) {
        $items[] = array(
          'class' => array('pager-ellipsis'),
          'data' => '…',
        );
      }
      */
    }
    // End generation.
    if ($li_next) {
      $items[] = array(
        'class' => array('next'),
        'data' => $li_next,
      );
    }
    if ($li_last) {
      $items[] = array(
        'class' => array('pager-last'),
        'data' => $li_last,
      );
    }

    return '<div class="pagination">' . theme('item_list', array(
      'items' => $items,
      // pager class is used for rounded, bubbly boxes in Bootstrap
      //'attributes' => array('class' => array('pager')),
    )) . '</div>';
  }
}
