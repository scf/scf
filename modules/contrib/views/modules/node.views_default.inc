<?php
// $Id: node.views_default.inc,v 1.5 2008/05/27 23:40:09 merlinofchaos Exp $
/**
 * @file
 * Contains default views on behalf of the node module.
 */

/**
 * Implementation of hook views_default_views().
 */
function node_views_default_views() {
  $views = array();

  // Frontpage view
  $view = new view;
  $view->name = 'frontpage';
  $view->description = t('Emulates the default Drupal front page; you may set the default home page path to this view to make it your front page.');
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
    $display->display_options = array (
    'style_plugin' => 'default',
    'style_options' =>
    array (
    ),
    'row_plugin' => 'node',
    'row_options' =>
    array (
      'teaser' => 1,
      'links' => 1,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
      'sticky' =>
      array (
        'id' => 'sticky',
        'table' => 'node',
        'field' => 'sticky',
        'order' => 'ASC',
      ),
      'created' =>
      array (
        'id' => 'created',
        'table' => 'node',
        'field' => 'created',
        'order' => 'ASC',
        'relationship' => 'none',
        'granularity' => 'second',
      ),
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
      'promote' =>
      array (
        'id' => 'promote',
        'table' => 'node',
        'field' => 'promote',
        'operator' => '=',
        'value' => '1',
        'group' => 0,
        'exposed' => false,
        'expose' =>
        array (
          'operator' => false,
          'label' => '',
        ),
      ),
      'status' =>
      array (
        'id' => 'status',
        'table' => 'node',
        'field' => 'status',
        'operator' => '=',
        'value' => '1',
        'group' => 0,
        'exposed' => false,
        'expose' =>
        array (
          'operator' => false,
          'label' => '',
        ),
      ),
    ),
    'items_per_page' => 10,
    'use_pager' => '1',
    'pager_element' => 0,
    'title' => '',
    'header' => '',
    'header_format' => '1',
    'footer' => '',
    'footer_format' => '1',
    'empty' => '',
    'empty_format' => '1',
  );
  $view->display['default'] = $display;
    $display = new views_display;
    $display->id = 'page';
    $display->display_title = t('Page');
    $display->display_plugin = 'page';
    $display->position = '2';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => true,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
      'style_plugin' => true,
      'style_options' => true,
      'row_plugin' => true,
      'row_options' => true,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => true,
      'filters' => true,
      'use_ajax' => true,
      'distinct' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
    ),
    'path' => 'frontpage',
  );
  $view->display['page'] = $display;
    $display = new views_display;
    $display->id = 'feed';
    $display->display_title = t('Feed');
    $display->display_plugin = 'feed';
    $display->position = '3';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => false,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'use_ajax' => true,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'use_more' => true,
      'distinct' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
      'style_plugin' => false,
      'style_options' => false,
      'row_plugin' => false,
      'row_options' => false,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => true,
      'filters' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
    ),
    'displays' =>
    array (
      'default' => 'default',
      'page' => 'page',
    ),
    'style_plugin' => 'rss',
    'style_options' =>
    array (
      'mission_description' => 1,
      'description' => '',
    ),
    'row_plugin' => 'node_rss',
    'row_options' =>
    array (
      'item_length' => 'default',
    ),
    'path' => 'rss.xml',
    'title' => t('Front page feed'),
  );
  $view->display['feed'] = $display;
  $views[$view->name] = $view;

  // Archive view
  $view = new view;
  $view->name = 'archive';
  $view->description = t('Display a list of months that link to content for that month.');
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
    $display->display_options = array (
    'style_plugin' => 'default',
    'style_options' =>
    array (
    ),
    'row_plugin' => 'node',
    'row_options' =>
    array (
      'teaser' => true,
      'links' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
      'created' =>
      array (
        'id' => 'created',
        'table' => 'node',
        'field' => 'created',
        'order' => 'DESC',
        'granularity' => 'second',
        'relationship' => 'none',
      ),
    ),
    'arguments' =>
    array (
      'created_year_month' =>
      array (
        'id' => 'created_year_month',
        'table' => 'node',
        'field' => 'created_year_month',
        'default_action' => 'summary asc',
        'style_plugin' => 'default_summary',
        'style_options' =>
        array (
          'count' => 1,
          'override' => 1,
          'items_per_page' => '30',
        ),
        'wildcard' => 'all',
        'wildcard_substitution' => 'All',
        'title' => '%1',
        'relationship' => 'none',
        'validate_type' => 'none',
        'validate_fail' => 'not found',
        'default_argument_type' => 'fixed',
      ),
    ),
    'filters' =>
    array (
      'status' =>
      array (
        'id' => 'status',
        'table' => 'node',
        'field' => 'status',
        'operator' => '=',
        'value' => 1,
        'group' => 0,
        'exposed' => false,
        'expose' =>
        array (
          'operator' => false,
          'label' => '',
        ),
        'relationship' => 'none',
      ),
    ),
    'items_per_page' => 10,
    'use_pager' => '1',
    'pager_element' => 0,
    'distinct' => 0,
  );
  $view->display['default'] = $display;
    $display = new views_display;
    $display->id = 'page';
    $display->display_title = t('Page');
    $display->display_plugin = 'page';
    $display->position = '2';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => true,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'use_ajax' => true,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'use_more' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
      'style_plugin' => true,
      'style_options' => true,
      'row_plugin' => true,
      'row_options' => true,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => true,
      'filters' => true,
      'distinct' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
    ),
    'path' => 'archive',
  );
  $view->display['page'] = $display;
    $display = new views_display;
    $display->id = 'block';
    $display->display_title = t('Block');
    $display->display_plugin = 'block';
    $display->position = '3';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => true,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'use_ajax' => false,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'use_more' => true,
      'distinct' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
      'style_plugin' => true,
      'style_options' => true,
      'row_plugin' => true,
      'row_options' => true,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => false,
      'filters' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
      'created_year_month' =>
      array (
        'id' => 'created_year_month',
        'table' => 'node',
        'field' => 'created_year_month',
        'default_action' => 'summary asc',
        'style_plugin' => 'default_summary',
        'style_options' =>
        array (
          'count' => 1,
          'override' => 0,
          'items_per_page' => '30',
        ),
        'wildcard' => 'all',
        'wildcard_substitution' => 'All',
        'title' => '%1',
        'relationship' => 'none',
        'validate_type' => 'none',
        'validate_fail' => 'not found',
        'default_argument_type' => 'fixed',
      ),
    ),
    'filters' =>
    array (
    ),
    'block_description' => 'Archive list',
    'use_ajax' => '1',
  );
  $view->display['block'] = $display;
  $views[$view->name] = $view;

  return $views;
}
