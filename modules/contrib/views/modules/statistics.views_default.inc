<?php
// $Id: statistics.views_default.inc,v 1.3 2008/05/23 17:49:38 merlinofchaos Exp $
/**
 * @file
 * Contains default views on behalf of the statistics module.
 */

/**
 * Implementation of hook views_default_views().
 */
function statistics_views_default_views() {
  $view = new view;
  $view->name = 'popular';
  $view->description = t('Shows the most-viewed nodes on the site. This requires the statistics to be enabled at administer >> site configuration >> statistics.');
  $view->tag = t('default');
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = '0';
  $view->api_version = 2;
  $view->disabled = TRUE; // Edit this to true to make a default view disabled initially
  $view->display = array();
    $display = new views_display;
    $display->id = 'default';
    $display->display_title = t('Defaults');
    $display->display_plugin = 'default';
    $display->position = '1';
    $display->display_options = array(
    'style_plugin' => 'table',
    'style_options' => array(
      'override' => 0,
      'order' => 'desc',
      'columns' => array(
        'type' => 'type',
        'title' => 'title',
        'name' => 'name',
        'timestamp' => 'title',
        'totalcount' => 'totalcount',
      ),
      'info' => array(
        'type' => array(
          'sortable' => 0,
          'separator' => '',
        ),
        'title' => array(
          'sortable' => 0,
          'separator' => '',
        ),
        'name' => array(
          'sortable' => 0,
          'separator' => '',
        ),
        'timestamp' => array(
          'separator' => '',
        ),
        'totalcount' => array(
          'sortable' => 0,
          'separator' => '',
        ),
      ),
      'default' => '-1',
    ),
    'row_plugin' => 'fields',
    'row_options' => array(),
    'relationships' => array(),
    'fields' => array(
      'type' => array(
        'id' => 'type',
        'table' => 'node',
        'field' => 'type',
        'label' => t('Type'),
      ),
      'title' => array(
        'id' => 'title',
        'table' => 'node',
        'field' => 'title',
        'label' => t('Title'),
        'link_to_node' => TRUE,
      ),
      'name' => array(
        'id' => 'name',
        'table' => 'users',
        'field' => 'name',
        'label' => t('Author'),
        'link_to_user' => TRUE,
      ),
      'timestamp' => array(
        'id' => 'timestamp',
        'table' => 'history_user',
        'field' => 'timestamp',
        'label' => '',
        'comments' => 1,
        'relationship' => 'none',
        'link_to_node' => 0,
        'comment' => 1,
      ),
    ),
    'sorts' => array(
      'totalcount' => array(
        'id' => 'totalcount',
        'table' => 'node_counter',
        'field' => 'totalcount',
        'order' => 'DESC',
        'relationship' => 'none',
      ),
    ),
    'arguments' => array(),
    'filters' => array(
      'status' => array(
        'id' => 'status',
        'table' => 'node',
        'field' => 'status',
        'operator' => '=',
        'value' => '1',
        'group' => 0,
        'exposed' => FALSE,
        'expose' => array(
          'operator' => FALSE,
          'label' => '',
        ),
      ),
      'totalcount' => array(
        'id' => 'totalcount',
        'table' => 'node_counter',
        'field' => 'totalcount',
        'operator' => '>',
        'value' => array(
          'value' => '0',
          'min' => '',
          'max' => '',
        ),
        'group' => 0,
        'exposed' => FALSE,
        'expose' => array(
          'operator' => FALSE,
          'label' => '',
        ),
        'relationship' => 'none',
      ),
    ),
    'items_per_page' => '25',
    'title' => t('Popular content'),
    'header' => NULL,
    'header_format' => NULL,
    'footer' => NULL,
    'footer_format' => NULL,
    'empty' => NULL,
    'empty_format' => NULL,
    'use_pager' => TRUE,
    'pager_element' => 0,
    'offset' => 0,
    'access' => array(
      'type' => 'none',
      'role' => array(),
      'perm' => '',
    ),
    'use_more' => 1,
  );
  $view->display['default'] = $display;
    $display = new views_display;
    $display->id = 'page';
    $display->display_title = t('Popular (page)');
    $display->display_plugin = 'page';
    $display->position = '2';
    $display->display_options = array(
    'defaults' => array(
      'access' => TRUE,
      'title' => TRUE,
      'header' => TRUE,
      'header_format' => TRUE,
      'header_empty' => TRUE,
      'footer' => TRUE,
      'footer_format' => TRUE,
      'footer_empty' => TRUE,
      'empty' => TRUE,
      'empty_format' => TRUE,
      'use_ajax' => TRUE,
      'items_per_page' => TRUE,
      'offset' => TRUE,
      'use_pager' => TRUE,
      'pager_element' => TRUE,
      'use_more' => TRUE,
      'distinct' => TRUE,
      'link_display' => TRUE,
      'style_plugin' => TRUE,
      'style_options' => TRUE,
      'row_plugin' => TRUE,
      'row_options' => TRUE,
      'relationships' => TRUE,
      'fields' => TRUE,
      'sorts' => TRUE,
      'arguments' => TRUE,
      'filters' => TRUE,
    ),
    'relationships' => array(),
    'fields' => array(),
    'sorts' => array(),
    'arguments' => array(),
    'filters' => array(),
    'path' => 'popular/all',
    'menu' => array(
      'type' => 'default tab',
      'title' => t('Popular content'),
      'weight' => '-1',
    ),
    'tab_options' => array(
      'type' => 'normal',
      'title' => t('Popular content'),
      'weight' => '',
    ),
  );
  $view->display['page'] = $display;
    $display = new views_display;
    $display->id = 'page_1';
    $display->display_title = t('Today (page)');
    $display->display_plugin = 'page';
    $display->position = '3';
    $display->display_options = array(
    'defaults' => array(
      'access' => TRUE,
      'title' => TRUE,
      'header' => TRUE,
      'header_format' => TRUE,
      'header_empty' => TRUE,
      'footer' => TRUE,
      'footer_format' => TRUE,
      'footer_empty' => TRUE,
      'empty' => TRUE,
      'empty_format' => TRUE,
      'use_ajax' => TRUE,
      'items_per_page' => TRUE,
      'offset' => TRUE,
      'use_pager' => TRUE,
      'pager_element' => TRUE,
      'use_more' => TRUE,
      'distinct' => TRUE,
      'link_display' => TRUE,
      'style_plugin' => TRUE,
      'style_options' => TRUE,
      'row_plugin' => TRUE,
      'row_options' => TRUE,
      'relationships' => TRUE,
      'fields' => FALSE,
      'sorts' => FALSE,
      'arguments' => TRUE,
      'filters' => TRUE,
    ),
    'relationships' => array(),
    'sorts' => array(
      'daycount' => array(
        'id' => 'daycount',
        'table' => 'node_counter',
        'field' => 'daycount',
        'order' => 'DESC',
        'relationship' => 'none',
      ),
    ),
    'arguments' => array(),
    'filters' => array(),
    'path' => 'popular/today',
    'menu' => array(
      'type' => 'tab',
      'title' => t('Today\'s popular content'),
      'weight' => '0',
    ),
    'fields' => array(
      'type' => array(
        'id' => 'type',
        'table' => 'node',
        'field' => 'type',
        'label' => t('Type'),
      ),
      'title' => array(
        'id' => 'title',
        'table' => 'node',
        'field' => 'title',
        'label' => t('Title'),
        'link_to_node' => TRUE,
      ),
      'name' => array(
        'id' => 'name',
        'table' => 'users',
        'field' => 'name',
        'label' => t('Author'),
        'link_to_user' => TRUE,
      ),
      'timestamp' => array(
        'id' => 'timestamp',
        'table' => 'history_user',
        'field' => 'timestamp',
        'label' => '',
        'comments' => 1,
        'relationship' => 'none',
        'link_to_node' => 0,
        'comment' => 1,
      ),
      'daycount' => array(
        'id' => 'daycount',
        'table' => 'node_counter',
        'field' => 'daycount',
        'label' => t('Views today'),
        'set_precision' => FALSE,
        'precision' => 0,
        'decimal' => '.',
        'separator' => ',',
        'prefix' => '',
        'suffix' => '',
        'relationship' => 'none',
      ),
    ),
    'tab_options' => array(
      'type' => 'normal',
      'title' => t('Popular content'),
      'weight' => '0',
    ),
  );
  $view->display['page_1'] = $display;
    $display = new views_display;
    $display->id = 'block';
    $display->display_title = t('Popular (block)');
    $display->display_plugin = 'block';
    $display->position = '4';
    $display->display_options = array(
    'defaults' => array(
      'access' => TRUE,
      'title' => TRUE,
      'header' => TRUE,
      'header_format' => TRUE,
      'header_empty' => TRUE,
      'footer' => TRUE,
      'footer_format' => TRUE,
      'footer_empty' => TRUE,
      'empty' => TRUE,
      'empty_format' => TRUE,
      'use_ajax' => TRUE,
      'items_per_page' => FALSE,
      'offset' => FALSE,
      'use_pager' => FALSE,
      'pager_element' => FALSE,
      'use_more' => TRUE,
      'distinct' => TRUE,
      'link_display' => TRUE,
      'style_plugin' => FALSE,
      'style_options' => FALSE,
      'row_plugin' => FALSE,
      'row_options' => FALSE,
      'relationships' => TRUE,
      'fields' => FALSE,
      'sorts' => TRUE,
      'arguments' => TRUE,
      'filters' => TRUE,
    ),
    'relationships' => array(),
    'fields' => array(
      'title' => array(
        'id' => 'title',
        'table' => 'node',
        'field' => 'title',
        'label' => '',
        'link_to_node' => 1,
        'relationship' => 'none',
      ),
      'totalcount' => array(
        'id' => 'totalcount',
        'table' => 'node_counter',
        'field' => 'totalcount',
        'label' => '',
        'set_precision' => FALSE,
        'precision' => 0,
        'decimal' => '.',
        'separator' => ',',
        'prefix' => ' (',
        'suffix' => ')',
        'relationship' => 'none',
      ),
    ),
    'sorts' => array(),
    'arguments' => array(),
    'filters' => array(),
    'style_plugin' => 'list',
    'style_options' => array(
      'type' => 'ul',
    ),
    'row_plugin' => 'fields',
    'row_options' => array(
      'inline' => array(
        'title' => 'title',
        'totalcount' => 'totalcount',
      ),
      'separator' => '',
    ),
    'items_per_page' => 5,
    'offset' => 0,
    'use_pager' => '0',
    'pager_element' => 0,
    'block_description' => t('Popular content'),
    'title' => t('Popular content'),
  );
  $view->display['block'] = $display;
    $display = new views_display;
    $display->id = 'block_1';
    $display->display_title = t('Today (block)');
    $display->display_plugin = 'block';
    $display->position = '5';
    $display->display_options = array(
    'defaults' => array(
      'access' => TRUE,
      'title' => FALSE,
      'header' => TRUE,
      'header_format' => TRUE,
      'header_empty' => TRUE,
      'footer' => TRUE,
      'footer_format' => TRUE,
      'footer_empty' => TRUE,
      'empty' => TRUE,
      'empty_format' => TRUE,
      'use_ajax' => TRUE,
      'items_per_page' => FALSE,
      'offset' => FALSE,
      'use_pager' => FALSE,
      'pager_element' => FALSE,
      'use_more' => TRUE,
      'distinct' => TRUE,
      'link_display' => FALSE,
      'style_plugin' => FALSE,
      'style_options' => FALSE,
      'row_plugin' => FALSE,
      'row_options' => FALSE,
      'relationships' => TRUE,
      'fields' => FALSE,
      'sorts' => FALSE,
      'arguments' => TRUE,
      'filters' => TRUE,
    ),
    'relationships' => array(),
    'fields' => array(
      'title' => array(
        'id' => 'title',
        'table' => 'node',
        'field' => 'title',
        'label' => '',
        'link_to_node' => 1,
        'relationship' => 'none',
      ),
      'daycount' => array(
        'id' => 'daycount',
        'table' => 'node_counter',
        'field' => 'daycount',
        'label' => '',
        'set_precision' => FALSE,
        'precision' => 0,
        'decimal' => '.',
        'separator' => ',',
        'prefix' => ' (',
        'suffix' => ')',
        'relationship' => 'none',
      ),
    ),
    'sorts' => array(
      'daycount' => array(
        'id' => 'daycount',
        'table' => 'node_counter',
        'field' => 'daycount',
        'order' => 'DESC',
        'relationship' => 'none',
      ),
    ),
    'arguments' => array(),
    'filters' => array(),
    'link_display' => 'page_1',
    'style_plugin' => 'list',
    'style_options' => array(
      'type' => 'ul',
    ),
    'row_plugin' => 'fields',
    'row_options' => array(
      'inline' => array(
        'title' => 'title',
        'daycount' => 'daycount',
      ),
      'separator' => '',
    ),
    'items_per_page' => 5,
    'offset' => 0,
    'use_pager' => '0',
    'pager_element' => 0,
    'block_description' => t('Today\'s popular content'),
    'title' => t('Today\'s popular content'),
  );
  $view->display['block_1'] = $display;

  $views[$view->name] = $view;
  return $views;
}
