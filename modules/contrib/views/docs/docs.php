<?php
// $Id: docs.php,v 1.10 2008/10/02 22:54:56 merlinofchaos Exp $
/**
 * @file
 * This file contains no working PHP code; it exists to provide additional documentation
 * for doxygen as well as to document hooks in the standard Drupal manner.
 */

/**
 * @mainpage Views 2 API Manual
 *
 * Much of this information is actually stored in the advanced help; please
 * check the API topic. This help will primarily be aimed at documenting
 * classes and function calls.
 *
 * Topics:
 * - @ref view_lifetime
 * - @ref views_hooks
 * - @ref views_handlers
 * - @ref views_plugins
 * - @ref views_templates
 */

/**
 * @page view_lifetime The life of a view
 *
 * This page explains the basic cycle of a view and what processes happen.
 */

/**
 * @page views_handlers About Views' handlers
 *
 * This page explains what views handlers are, how they're written, and what
 * the basic conventions are.
 *
 * - @ref views_field_handlers
 * - @ref views_sort_handlers
 * - @ref views_filter_handlers
 * - @ref views_argument_handlers
 * - @ref views_relationship_handlers
 */

/**
 * @page views_plugins About Views' plugins
 *
 * This page explains what views plugins are, how they're written, and what
 * the basic conventions are.
 *
 * - @ref views_display_plugins
 * - @ref views_style_plugins
 * - @ref views_row_plugins
 */

/**
 * @defgroup views_hooks Views' hooks
 * @{
 * Hooks that can be implemented by other modules in order to implement the
 * Views API.
 */

/**
 * The full documentation for this hook is now in the advanced help.
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 */
function hook_views_data() {
  // example code here
}

/**
 * The full documentation for this hook is now in the advanced help.
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 *
 * This is a stub list as a reminder that this needs to be doc'd and is not used
 * in views anywhere so might not be remembered when this is formally documented:
 * - style: 'even empty'
 */
function hook_views_plugins() {
  // example code here
}

/**
 * Register handler, file and parent information so that handlers can be
 * loaded only on request.
 *
 * The full documentation for this hook is in the advanced help.
 */
function hook_views_handlers() {
  // example code here
}

/**
 * Register View API information. This is required for your module to have
 * its include files loaded.
 *
 * The full documentation for this hook is in the advanced help.
 */
function hook_views_api() {
}

/**
 * This hook allows modules to provide their own views which can either be used
 * as-is or as a "starter" for users to build from.
 *
 * This hook should be placed in MODULENAME.views_default.inc and it will be
 * auto-loaded. This must either be in the same directory as the .module file
 * or in a subdirectory named 'includes'.
 *
 * The $view->disabled boolean flag indicates whether the View should be
 * enabled or disabled by default.
 *
 * @return
 *   An associative array containing the structures of views, as generated from
 *   the Export tab, keyed by the view name. A best practice is to go through
 *   and add t() to all title and label strings, with the exception of menu
 *   strings.
 */
function hook_views_default_views() {
  // Begin copy and paste of output from the Export tab of a view.
  $view = new view;
  $view->name = 'frontpage';
  $view->description = t('Emulates the default Drupal front page; you may set the default home page path to this view to make it your front page.');
  $view->tag = t('default');
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = '0';
  $view->api_version = 2;
  $view->disabled = FALSE; // Edit this to true to make a default view disabled initially
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
  // End copy and paste of Export tab output.

  // Add view to list of views to provide.
  $views[$view->name] = $view;

  // ...Repeat all of the above for each view the module should provide.

  // At the end, return array of default views.
  return $views;
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views_convert.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 */
function hook_views_convert() {
  // example code here
}

/**
 * Stub hook documentation
 */
function hook_views_query_substitutions() {
  // example code here
}

/**
 * This hook is called at the very beginning of views processing,
 * before anything is done.
 *
 * Adding output to the view cam be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after
 */
function hook_views_pre_view(&$view, &$display_id, &$args) {
  // example code here
}

/**
 * This hook is called right before the build process, but after displays
 * are attached and the display performs its pre_execute phase.
 *
 * Adding output to the view cam be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after
 */
function hook_views_pre_build(&$view) {
  // example code here
}

/**
 * This hook is called right before the execute process. The query is
 * now fully built, but it has not yet been run through db_rewrite_sql.
 *
 * Adding output to the view cam be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after
 */
function hook_views_pre_execute(&$view) {
  // example code here
}

/**
 * This hook is called right before the render process. The query has
 * been executed, and the pre_render() phase has already happened for
 * handlers, so all data should be available.
 *
 * Adding output to the view cam be accomplished by placing text on
 * $view->attachment_before and $view->attachment_after
 */
function hook_views_pre_render(&$view) {
  // example code here
}

/**
 * Stub hook documentation
 *
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 *
 */
function hook_views_query_alter(&$view, &$query) {
  // example code here
}

/**
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 *
 * Alter the links that appear over a view. They are in a format suitable for
 * theme('links').
 *
 * Warning: $view is not a reference in PHP4 and cannot be modified here. But it IS
 * a reference in PHP5, and can be modified. Please be careful with it.
 *
 * @see theme_links
 */
function hook_views_admin_links_alter(&$links, $view) {
  // example code here
}

/**
 * This hook should be placed in MODULENAME.views.inc and it will be auto-loaded.
 * This must either be in the same directory as the .module file or in a subdirectory
 * named 'includes'.
 *
 * Alter the rows that appear with a view, which includes path and query information.
 * The rows are suitable for theme('table').
 *
 * Warning: $view is not a reference in PHP4 and cannot be modified here. But it IS
 * a reference in PHP5, and can be modified. Please be careful with it.
 *
 * @see theme_table
 */
function hook_views_preview_info_alter(&$rows, $view) {
  // example code here
}

/**
 * @}
 */
