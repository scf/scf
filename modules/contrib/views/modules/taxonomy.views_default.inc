<?php
// $Id: taxonomy.views_default.inc,v 1.2 2008/05/27 23:40:09 merlinofchaos Exp $
/**
 * @file
 * Contains default views on behalf of the statistics module.
 */

/**
 * Implementation of hook views_default_views().
 */
function taxonomy_views_default_views() {
  $view = new view;
  $view->name = 'taxonomy_term';
  $view->description = t('A view to emulate Drupal core\'s handling of taxonomy/term; it also emulates Views 1\'s handling by having two possible feeds.');
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
    'style_plugin' => 'default',
    'style_options' => array(),
    'row_plugin' => 'node',
    'row_options' => array(
      'teaser' => TRUE,
      'links' => TRUE,
    ),
    'relationships' => array(),
    'fields' => array(),
    'sorts' => array(
      'sticky' => array(
        'id' => 'sticky',
        'table' => 'node',
        'field' => 'sticky',
        'order' => 'DESC',
        'relationship' => 'none',
      ),
      'created' => array(
        'id' => 'created',
        'table' => 'node',
        'field' => 'created',
        'order' => 'DESC',
        'granularity' => 'second',
        'relationship' => 'none',
      ),
    ),
    'arguments' => array(
      'term_node_tid_depth' => array(
        'id' => 'term_node_tid_depth',
        'table' => 'node',
        'field' => 'term_node_tid_depth',
        'default_action' => 'not found',
        'style_plugin' => 'default_summary',
        'style_options' => array(
    'count' => TRUE,
    'override' => FALSE,
    'items_per_page' => 25,
        ),
        'wildcard' => 'all',
        'wildcard_substitution' => 'All',
        'title' => '%1',
        'default_argument_type' => 'fixed',
        'default_argument' => '',
        'validate_type' => 'taxonomy_term',
        'validate_fail' => 'not found',
        'depth' => '0',
        'break_phrase' => 1,
        'relationship' => 'none',
        'default_argument_fixed' => '',
        'default_argument_php' => '',
        'validate_argument_node_type' => array(
          'album' => 0,
          'artist' => 0,
          'book' => 0,
          'page' => 0,
          'story' => 0,
          'track' => 0,
        ),
        'validate_argument_vocabulary' => array(
          '3' => 0,
          '4' => 0,
          '1' => 0,
          '5' => 0,
          '2' => 0,
        ),
        'validate_argument_type' => 'tids',
        'validate_argument_php' => '',
      ),
      'term_node_tid_depth_modifier' => array(
        'id' => 'term_node_tid_depth_modifier',
        'table' => 'node',
        'field' => 'term_node_tid_depth_modifier',
        'default_action' => 'ignore',
        'style_plugin' => 'default_summary',
        'style_options' => array(
          'count' => TRUE,
          'override' => FALSE,
          'items_per_page' => 25,
        ),
        'wildcard' => 'all',
        'wildcard_substitution' => 'All',
        'title' => '',
        'default_argument_type' => 'fixed',
        'default_argument' => '',
        'validate_type' => 'none',
        'validate_fail' => 'not found',
      ),
    ),
    'filters' => array(
      'status_extra' => array(
        'id' => 'status_extra',
        'table' => 'node',
        'field' => 'status_extra',
        'operator' => '=',
        'value' => '',
        'group' => 0,
        'exposed' => FALSE,
        'expose' => array(
          'operator' => FALSE,
          'label' => '',
        ),
        'relationship' => 'none',
      ),
    ),
    'items_per_page' => 10,
    'use_pager' => '1',
    'pager_element' => 0,
  );
  $view->display['default'] = $display;
    $display = new views_display;
    $display->id = 'page';
    $display->display_title = t('Page');
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
      'php_arg_code' => TRUE,
      'exposed_options' => TRUE,
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
    'path' => 'taxonomy/term/%',
  );
  $view->display['page'] = $display;
    $display = new views_display;
    $display->id = 'feed';
    $display->display_title = t('Core feed');
    $display->display_plugin = 'feed';
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
    'displays' => array(
      'page' => 'page',
      'default' => 0,
    ),
    'style_plugin' => 'rss',
    'style_options' => array(
      'mission_description' => FALSE,
      'description' => '',
    ),
    'row_plugin' => 'node_rss',
    'path' => 'taxonomy/term/%/%/feed',
    'row_options' => array(
      'item_length' => 'default',
    ),
    'items_per_page' => 15,
    'offset' => 0,
    'use_pager' => '1',
    'pager_element' => 0,
  );
  $view->display['feed'] = $display;
    $display = new views_display;
    $display->id = 'feed_1';
    $display->display_title = t('Views 1 feed');
    $display->display_plugin = 'feed';
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
    'displays' => array(),
    'style_plugin' => 'rss',
    'style_options' => array(
      'mission_description' => FALSE,
      'description' => '',
    ),
    'row_plugin' => 'node_rss',
    'path' => 'taxonomy/term/%/feed',
    'row_options' => array(
      'item_length' => 'default',
    ),
    'items_per_page' => 15,
    'offset' => 0,
    'use_pager' => '1',
    'pager_element' => 0,
  );
  $view->display['feed_1'] = $display;
  $views[$view->name] = $view;
  return $views;
}
