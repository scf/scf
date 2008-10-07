<?php
// $Id: search.views_default.inc,v 1.5 2008/05/27 23:49:38 merlinofchaos Exp $

/**
 * Implementation of hook_views_default_views().
 */
function search_views_default_views() {
  $views = array();

  $view = new view;
  $view->name = 'backlinks';
  $view->description = t('Displays a list of nodes that link to the node, using the search backlinks table.');
  $view->tag = 'default';
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
    'style_plugin' => 'list',
    'style_options' => array(
      'type' => 'ol',
    ),
    'row_plugin' => 'fields',
    'row_options' => array(),
    'relationships' => array(),
    'fields' => array(
      'title' => array(
        'id' => 'title',
        'table' => 'node',
        'field' => 'title',
        'label' => '',
        'relationship' => 'none',
        'link_to_node' => 1,
      ),
    ),
    'sorts' => array(),
    'arguments' => array(
      'nid' => array(
        'id' => 'nid',
        'table' => 'search_node_links_to',
        'field' => 'nid',
        'default_action' => 'not found',
        'style_plugin' => 'default_summary',
        'style_options' => array(
          'count' => TRUE,
          'override' => FALSE,
          'items_per_page' => 25,
        ),
        'wildcard' => '',
        'wildcard_substitution' => '',
        'title' => t('Pages that link to %1'),
        'default_argument_type' => 'fixed',
        'default_argument' => '',
        'validate_type' => 'node',
        'validate_fail' => 'not found',
        'relationship' => 'none',
        'default_argument_fixed' => '',
        'default_argument_php' => '',
        'validate_argument_node_type' => array(),
        'validate_argument_php' => '',
      ),
    ),
    'filters' => array(
      'status' => array(
        'id' => 'status',
        'table' => 'node',
        'field' => 'status',
        'operator' => '=',
        'value' => 1,
        'group' => 0,
        'exposed' => FALSE,
        'expose' => array(
          'operator' => FALSE,
          'label' => '',
        ),
        'relationship' => 'none',
      ),
    ),
    'items_per_page' => 30,
    'use_pager' => '1',
    'pager_element' => 0,
    'offset' => 0,
    'empty' => 'No backlinks found.',
    'empty_format' => '1',
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
    'path' => 'node/%/backlinks',
    'menu' => array(
      'type' => 'tab',
      'title' => t('What links here'),
      'weight' => '0',
    ),
  );
  $view->display['page'] = $display;
    $display = new views_display;
    $display->id = 'block';
    $display->display_title = t('Block');
    $display->display_plugin = 'block';
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
      'use_more' => FALSE,
      'distinct' => TRUE,
      'link_display' => TRUE,
      'style_plugin' => FALSE,
      'style_options' => FALSE,
      'row_plugin' => FALSE,
      'row_options' => FALSE,
      'relationships' => TRUE,
      'fields' => TRUE,
      'sorts' => TRUE,
      'arguments' => FALSE,
      'filters' => TRUE,
    ),
    'relationships' => array(),
    'fields' => array(),
    'sorts' => array(),
    'arguments' => array(
      'nid' => array(
        'id' => 'nid',
        'table' => 'search_node_links_to',
        'field' => 'nid',
        'default_action' => 'default',
        'style_plugin' => 'default_summary',
        'style_options' => array(
          'count' => TRUE,
          'override' => FALSE,
          'items_per_page' => 25,
        ),
        'wildcard' => '',
        'wildcard_substitution' => '',
        'title' => t('What links here'),
        'default_argument_type' => 'node',
        'default_argument' => '',
        'validate_type' => 'node',
        'validate_fail' => 'not found',
        'relationship' => 'none',
        'default_argument_fixed' => '',
        'default_argument_php' => 'return ($node = menu_get_object()) ? $node->nid : FALSE;',
        'validate_argument_node_type' => array(
          'album' => 0,
          'artist' => 0,
          'book' => 0,
          'page' => 0,
          'story' => 0,
          'track' => 0,
        ),
        'validate_argument_php' => '',
        'default_argument_user' => 0,
        'validate_argument_vocabulary' => array(
          '3' => 0,
          '4' => 0,
          '1' => 0,
          '5' => 0,
          '2' => 0,
        ),
        'validate_argument_type' => 'tid',
      ),
    ),
    'filters' => array(),
    'items_per_page' => 6,
    'offset' => 0,
    'use_pager' => '0',
    'pager_element' => 0,
    'use_more' => 1,
    'style_plugin' => 'list',
    'style_options' => array(
      'type' => 'ul',
    ),
    'row_plugin' => 'fields',
    'row_options' => array(),
    'block_description' => 'What links here',
  );
  $view->display['block'] = $display;

  $views[$view->name] = $view;
  return $views;
}
