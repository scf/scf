<?php
// $Id: plugins.inc,v 1.100 2008/05/27 21:57:06 merlinofchaos Exp $
/**
 * @file plugins.inc
 * Built in plugins for Views output handling.
 *
 */

/**
 * Implementation of hook_views_plugins
 */
function views_views_plugins() {
  $path = drupal_get_path('module', 'views') . '/js';
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'display' => array(
      'default' => array(
        'title' => t('Defaults'),
        'help' => t('Default settings for this view.'),
        'handler' => 'views_plugin_display_default',
        'theme' => 'views_view',
        'no ui' => TRUE,
        'no remove' => TRUE,
        'js' => array('misc/collapse.js', 'misc/textarea.js', 'misc/tabledrag.js', 'misc/autocomplete.js', "$path/dependent.js"),
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'help topic' => 'display-default',
      ),
      'page' => array(
        'title' => t('Page'),
        'help' => t('Display the view as a page, with a URL and menu links.'),
        'handler' => 'views_plugin_display_page',
        'theme' => 'views_view',
        'uses hook menu' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Page'),
        'help topic' => 'display-page',
      ),
      'block' => array(
        'title' => t('Block'),
        'help' => t('Display the view as a block.'),
        'handler' => 'views_plugin_display_block',
        'theme' => 'views_view',
        'uses hook block' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Block'),
        'help topic' => 'display-block',
      ),
      'attachment' => array(
        'title' => t('Attachment'),
        'help' => t('Attachments added to other displays to achieve multiple views in the same view.'),
        'handler' => 'views_plugin_display_attachment',
        'theme' => 'views_view',
        'use ajax' => TRUE,
        'help topic' => 'display-attachment',
      ),
      'feed' => array(
        'title' => t('Feed'),
        'help' => t('Display the view as a feed, such as an RSS feed.'),
        'handler' => 'views_plugin_display_feed',
        'uses hook menu' => TRUE,
        'use ajax' => FALSE,
        'use pager' => FALSE,
        'accept attachments' => FALSE,
        'admin' => t('Feed'),
        'help topic' => 'display-feed',
      ),
    ),
    'style' => array(
      'default' => array(
        'title' => t('Unformatted'),
        'help' => t('Displays rows one after another.'),
        'handler' => 'views_plugin_style_default',
        'theme' => 'views_view_unformatted',
        'uses row plugin' => TRUE,
        'type' => 'normal',
      ),
      'list' => array(
        'title' => t('List'),
        'help' => t('Displays rows as an HTML list.'),
        'handler' => 'views_plugin_style_list',
        'theme' => 'views_view_list',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'grid' => array(
        'title' => t('Grid'),
        'help' => t('Displays rows in a grid.'),
        'handler' => 'views_plugin_style_grid',
        'theme' => 'views_view_grid',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'table' => array(
        'title' => t('Table'),
        'help' => t('Displays rows in a table.'),
        'handler' => 'views_plugin_style_table',
        'theme' => 'views_view_table',
        'uses row plugin' => FALSE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'default_summary' => array(
        'title' => t('Default'),
        'help' => t('Displays the default summary view'),
        'handler' => 'views_plugin_style_summary',
        'theme' => 'views_view_summary',
        'type' => 'summary', // only shows up as a summary style
        'uses options' => TRUE,
      ),
      'rss' => array(
        'title' => t('RSS Feed'),
        'help' => t('Generates an RSS feed from a view.'),
        'handler' => 'views_plugin_style_rss',
        'theme' => 'views_view_rss',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'feed',
      ),
    ),
    'row' => array(
      'fields' => array(
        'title' => t('Fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'views_plugin_row_fields',
        'theme' => 'views_view_fields',
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
    ),
    'argument default' => array(
      'fixed' => array(
        'title' => t('Fixed entry'),
        'handler' => 'views_plugin_argument_default',
      ),
      'php' => array(
        'title' => t('PHP Code'),
        'handler' => 'views_plugin_argument_default_php',
      ),
    ),
    'argument validator' => array(
      'php' => array(
        'title' => t('PHP Code'),
        'handler' => 'views_plugin_argument_validate_php',
      ),
      'numeric' => array(
        'title' => t('Numeric'),
        'handler' => 'views_plugin_argument_validate_numeric',
      ),
    ),
  );
}

/**
 * Builds and return a list of all plugins available in the system.
 *
 * @return Nested array of plugins, grouped by type and
 */
function views_discover_plugins() {
  $cache = array('display' => array(), 'style' => array(), 'row' => array());
  // Get plugins from all mdoules.
  foreach (module_implements('views_plugins') as $module) {
    $function = $module . '_views_plugins';
    $result = $function();
    if (!is_array($result)) {
      continue;
    }

    $module_dir = isset($result['module']) ? $result['module'] : $module;
    // Setup automatic path/file finding for theme registration
    if ($module_dir == 'views') {
      $path = drupal_get_path('module', $module_dir) . '/theme';
      $file = 'theme.inc';
    }
    else {
      $path = drupal_get_path('module', $module_dir);
      $file = "$module.views.inc";
    }
    foreach ($result as $type => $info) {
      if ($type == 'module') {
        continue;
      }
      foreach ($info as $plugin => $def) {
        $def['module'] = $module_dir;
        if (isset($def['theme']) && !isset($def['path'])) {
          $def['path'] = $path;
          $def['file'] = $file;
        }
        // merge the new data in
        $cache[$type][$plugin] = $def;
      }
    }
  }
  return $cache;
}

/**
 * Abstract base class to provide interface common to all plugins.
 */
class views_plugin extends views_object {
  /**
   * Init will be called after construct, when the plugin is attached to a
   * view and a display.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;
  }

  /**
   * Provide a form to edit options for this plugin.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) { }

  /**
   * Handle any special handling on the validate form.
   */
  function options_submit(&$form, &$form_state) { }

  /**
   * Add anything to the query that we might need to.
   */
  function query() { }

  /**
   * Provide a full list of possible theme templates used by this style.
   */
  function theme_functions() {
    return views_theme_functions($this->definition['theme'], $this->view, $this->display);
  }

  /**
   * Validate that the plugin is correct and can be saved.
   *
   * @return
   *   An array of error strings to tell the user what is wrong with this
   *   plugin.
   */
  function validate() { return array(); }
}

/**
 * @defgroup views_display_plugins Views' display plugins
 * @{
 * Display plugins control how Views interact with the rest of Drupal.
 *
 * They can handle creating Views from a Drupal page hook; they can
 * handle creating Views from a Drupal block hook. They can also
 * handle creating Views from an external module source, such as
 * a Panels pane, or an insert view, or a CCK field type.
 *
 * @see hook_views_plugins
 */

/**
 * The default display plugin handler. Display plugins handle options and
 * basic mechanisms for different output methods.
 */
class views_plugin_display extends views_plugin {
  /**
   * Determine if this display is the 'default' display which contains
   * fallback settings
   */
  function is_default_display() { return FALSE; }

  /**
   * Determine if this display uses exposed filters, so the view
   * will know whether or not to build them.
   */
  function uses_exposed() {
    if (!isset($this->has_exposed)) {
      foreach (array('field', 'filter') as $type) {
        foreach ($this->view->$type as $key => $info) {
          if (!empty($info['handler']) && $info['handler']->is_exposed()) {
            // one is all we need; if we find it, return true.
            $this->has_exposed = TRUE;
            return TRUE;
          }
        }
      }
      $this->has_exposed = FALSE;
    }

    return $this->has_exposed;
  }

  /**
   * Does the display use AJAX?
   */
  function use_ajax() {
    if (!empty($this->definition['use ajax'])) {
      return $this->get_option('use_ajax');
    }
    return FALSE;
  }

  /**
   * Does the display have a pager enabled?
   */
  function use_pager() {
    if (!empty($this->definition['use pager'])) {
      return $this->get_option('use_pager');
    }
    return FALSE;
  }

  /**
   * Does the display have a more link enabled?
   */
  function use_more() {
    if (!empty($this->definition['use more'])) {
      return $this->get_option('use_more');
    }
    return FALSE;
  }

  /**
   * Can this display accept attachments?
   */
  function accept_attachments() {
    return !empty($this->definition['accept attachments']);
  }

  /**
   * Allow displays to attach to other views.
   */
  function attach_to($display_id) { }

  /**
   * Static member function to list which sections are defaultable
   * and what items each section contains.
   */
  function defaultable_sections($section = NULL) {
    $sections = array(
      'access' => array('access'),
      'title' => array('title'),
      'header' => array('header', 'header_format', 'header_empty'),
      'footer' => array('footer', 'footer_format', 'footer_empty'),
      'empty' => array('empty', 'empty_format'),
      'use_ajax' => array('use_ajax'),
      'items_per_page' => array('items_per_page', 'offset', 'use_pager', 'pager_element'),
      'use_pager' => array('items_per_page', 'offset', 'use_pager', 'pager_element'),
      'use_more' => array('use_more'),
      'link_display' => array('link_display'),
      'distinct' => array('distinct'),

      // Force these to cascade properly.
      'style_plugin' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'style_options' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'row_plugin' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'row_options' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),

      // These guys are special
      'relationships' => array('relationships'),
      'fields' => array('fields'),
      'sorts' => array('sorts'),
      'arguments' => array('arguments'),
      'filters' => array('filters'),
    );
    if ($section) {
      if (!empty($sections[$section])) {
        return $sections[$section];
      }
    }
    else {
      return $sections;
    }
  }

  /**
   * This is called when a new display is created that has never been
   * saved to the database. It provides appropriate defaults for the
   * display, if necessary.
   */
  function options(&$display) {
    // Set the following options to use the default display, forcing
    // them to pass through unless we override.
    $display->display_options['defaults'] = array(
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
      'pager_element'  => TRUE,
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
    );

    $display->display_options['relationships'] = array();
    $display->display_options['fields'] = array();
    $display->display_options['sorts'] = array();
    $display->display_options['arguments'] = array();
    $display->display_options['filters'] = array();
  }

  /**
   * Check to see if the display has a 'path' field.
   *
   * This is a pure function and not just a setting on the definition
   * because some displays (such as a panel pane) may have a path based
   * upon configuration.
   *
   * By default, displays do not have a path.
   */
  function has_path() { return FALSE; }

  /**
   * Check to see if the display has some need to link to another display.
   *
   * For the most part, displays without a path will use a link display. However,
   * sometimes displays that have a path might also need to link to another display.
   * This is true for feeds.
   */
  function uses_link_display() { return !$this->has_path(); }

  /**
   * Check to see which display to use when creating links within
   * a view using this display.
   */
  function get_link_display() {
    $display_id = $this->get_option('link_display');
    // If unknown, pick the first one.
    if (empty($display_id) || empty($this->view->display[$display_id])) {
      foreach ($this->view->display as $display_id => $display) {
        if ($display->handler->has_path()) {
          return $display_id;
        }
      }
    }
    else {
      return $display_id;
    }
    // fall-through returns NULL
  }

  /**
   * Return the base path to use for this display.
   *
   * This can be overridden for displays that do strange things
   * with the path.
   */
  function get_path() {
    if ($this->has_path()) {
      return $this->get_option('path');
    }

    $display_id = $this->get_link_display();
    if ($display_id && !empty($this->view->display[$display_id])) {
      return $this->view->display[$display_id]->handler->get_path();
    }
  }

  /**
   * Check to see if the display needs a breadcrumb
   *
   * By default, displays do not need breadcrumbs
   */
  function uses_breadcrumb() { return FALSE; }

  /**
   * Determine if a given option is set to use the default display or the
   * current display
   *
   * @return
   *   TRUE for the default display
   */
  function is_defaulted($option) {
    return !$this->is_default_display() && !empty($this->default_display) && !empty($this->display->display_options['defaults'][$option]);
  }

  /**
   * Intelligently get an option either from this display or from the
   * default display, if directed to do so.
   */
  function get_option($option) {
    if ($this->is_defaulted($option)) {
      return $this->default_display->get_option($option);
    }

    if (array_key_exists($option, $this->display->display_options)) {
      return $this->display->display_options[$option];
    }
  }

  /**
   * Determine if the display's style uses fields.
   */
  function uses_fields() {
    $plugin = $this->get_plugin();
    if ($plugin) {
      return $plugin->uses_fields();
    }
  }

  /**
   * Get the display or row plugin, if it exists.
   */
  function get_plugin($type = 'style', $name = NULL) {
    if (!$name) {
      $name = $this->get_option($type . '_plugin');
    }

    $plugin = views_get_plugin($type, $name);
    if ($plugin) {
      $options = $this->get_option($type . '_options');
      $plugin->init($this->view, $this->display, $options);
      return $plugin;
    }
  }

  /**
   * Intelligently set an option either from this display or from the
   * default display, if directed to do so.
   */
  function set_option($option, $value) {
    if (!$this->is_default_display() && !empty($this->default_display) && !empty($this->display->display_options['defaults'][$option])) {
      return $this->default_display->set_option($option, $value);
    }

    return $this->display->display_options[$option] = $value;
  }

  /**
   * Because forms may be split up into sections, this provides
   * an easy URL to exactly the right section. Don't override this.
   */
  function option_link($text, $section, $class = '', $title = '') {
    if (!empty($class)) {
      $text = '<span>' . $text . '</span>';
    }

    if (empty($text)) {
      $text = t('Broken field');
    }

    if (empty($title)) {
      $title = $text;
    }

    return l($text, 'admin/build/views/nojs/display/' . $this->view->name . '/' . $this->display->id . '/' . $section, array('attributes' => array('class' => 'views-ajax-link ' . $class, 'title' => $title), 'html' => TRUE));
  }

  /**
   * Provide the default summary for options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    $categories['basic'] = array(
      'title' => t('Basic settings'),
    );

    $options['display_title'] = array(
      'category' => 'basic',
      'title' => t('Name'),
      'value' => $this->display->display_title,
      'desc' => t('Change the name of this display.'),
    );

    $title = $this->get_option('title');
    if (!$title) {
      $title = t('None');
    }

    $options['title'] = array(
      'category' => 'basic',
      'title' => t('Title'),
      'value' => $title,
      'desc' => t('Change the title that this display will use.'),
    );

    $style_plugin = views_fetch_plugin_data('style', $this->get_option('style_plugin'));
    $style_title = empty($style_plugin['title']) ? t('Missing style plugin') : $style_plugin['title'];

    $style = '';

    $options['style_plugin'] = array(
      'category' => 'basic',
      'title' => t('Style'),
      'value' => $style_title,
      'desc' => t('Change the style plugin.'),
    );

    // This adds a 'Settings' link to the style_options setting if the style has options.
    if (!empty($style_plugin['uses options'])) {
      $options['style_plugin']['links']['style_options'] = t('Change settings for this style');
    }

    if (!empty($style_plugin['uses row plugin'])) {
      $row_plugin = views_fetch_plugin_data('row', $this->get_option('row_plugin'));
      $row_title = empty($row_plugin['title']) ? t('Missing style plugin') : $row_plugin['title'];

      $options['row_plugin'] = array(
        'category' => 'basic',
        'title' => t('Row style'),
        'value' => $row_title,
        'desc' => t('Change the row plugin.'),
      );
      // This adds a 'Settings' link to the row_options setting if the row style has options.
      if (!empty($row_plugin['uses options'])) {
        $options['row_plugin']['links']['row_options'] = t('Change settings for this style');
      }
    }
    if (!empty($this->definition['use ajax'])) {
      $options['use_ajax'] = array(
        'category' => 'basic',
        'title' => t('Use AJAX'),
        'value' => $this->get_option('use_ajax') ? t('Yes') : t('No'),
        'desc' => t('Change whether or not this display will use AJAX.'),
      );
    }

    if (!empty($this->definition['use pager'])) {
      $options['use_pager'] = array(
        'category' => 'basic',
        'title' => t('Use pager'),
        'value' => $this->get_option('use_pager') ? ($this->get_option('use_pager') === 'mini' ? t('Mini') : t('Yes')) : t('No'),
        'desc' => t("Change this display's pager setting."),
      );
    }

    $items = intval($this->get_option('items_per_page'));
    $options['items_per_page'] = array(
      'category' => 'basic',
      'title' => $this->use_pager() ? t('Items per page') : t('Items to display'),
      'value' => $items ? $items : t('Unlimited'),
      'desc' => t('Change how many items to display.'),
    );

    if (!empty($this->definition['use more'])) {
      $options['use_more'] = array(
        'category' => 'basic',
        'title' => t('More link'),
        'value' => $this->get_option('use_more') ? t('Yes') : t('No'),
        'desc' => t('Specify whether this display will provide a "more" link.'),
      );
    }

    $options['distinct'] = array(
      'category' => 'basic',
      'title' => t('Distinct'),
      'value' => $this->get_option('distinct') ? t('Yes') : t('No'),
      'desc' => t('Display only distinct items, without duplicates.'),
    );

    $access = $this->get_option('access');
    if (!is_array($access)) {
      $access = array('type' => 'none');
    }
    switch($access['type']) {
      case 'none':
      default:
        $access_str = t('Unrestricted');
        break;
      case 'perm':
        $access_str = $access['perm'];
        break;
      case 'role':
        $roles = array_keys(array_filter($access['role']));
        if (count($roles) > 1) {
          $access_str = t('Multiple roles');
        }
        else {
          $rids = views_ui_get_roles();
          $rid = array_shift($roles);
          $access_str = $rids[$rid];
        }
        break;
    }

    $options['access'] = array(
      'category' => 'basic',
      'title' => t('Access'),
      'value' => $access_str,
      'desc' => t('Specify access control settings for this display.'),
    );

    if ($this->uses_link_display()) {
      // Only show the 'link display' if there is more than one option.
      $count = 0;
      foreach ($this->view->display as $display_id => $display) {
        if ($display->handler->has_path()) {
          $count++;
        }
        if ($count > 1) {
          break;
        }
      }

      if ($count > 1) {
        $display_id = $this->get_link_display();
        $link_display = empty($this->view->display[$display_id]) ? t('None') : $this->view->display[$display_id]->display_title;
        $options['link_display'] = array(
          'category' => 'basic',
          'title' => t('Link display'),
          'value' => $link_display,
          'desc' => t('Specify which display this display will link to.'),
        );
      }
    }

    foreach (array('header' => t('Header'), 'footer' => t('Footer'), 'empty' => t('Empty text')) as $type => $name) {
      if (!$this->get_option($type)) {
        $field = t('None');
      }
      else {
        // A lot of code to get the name of the filter format.
        $fmt_string = $this->get_option($type . '_format');
        if (empty($fmt_string)) {
          $fmt_string = FILTER_FORMAT_DEFAULT;
        }
        $format_val = filter_resolve_format($fmt_string);
        $format = filter_formats($format_val);
        if ($format) {
          $field = $format->name;
        }
        else {
          $field = t('Unknown/missing filter');
        }
      }

//      $output[] = t('!name: !field', array('!name' => $name, '!field' => $this->option_link($field, $type)));
      $options[$type] = array(
        'category' => 'basic',
        'title' => $name,
        'value' => $field,
        'desc' => t("Change this display's !name.", array('!name' => strtolower($name))),
      );
    }

    $options['analyze-theme'] = array(
      'category' => 'basic',
      'title' => t('Theme'),
      'value' => t('Information'),
      'desc' => t('Get information on how to theme this display'),
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    if ($this->defaultable_sections($form_state['section'])) {
      $this->add_override_button($form, $form_state['section']);
    }
    $form['#title'] = check_plain($this->display->display_title) . ': ';

    // Set the 'section' to hilite on the form.
    // If it's the item we're looking at is pulling from the default display,
    // reflect that.
    if (!empty($this->display->display_options['defaults'][$form_state['section']])) {
      $form['#section'] = 'default-' . $form_state['section'];
    }
    else {
      $form['#section'] = $this->display->id . '-' . $form_state['section'];
    }

    switch ($form_state['section']) {
      case 'display_title':
        $form['#title'] .= t('The name of this display');
        $form['display_title'] = array(
          '#type' => 'textfield',
          '#description' => t('This title will appear only in the administrative interface for the View.'),
          '#default_value' => $this->display->display_title,
        );
        break;
      case 'title':
        $form['#title'] .= t('The title of this view');
        $form['title'] = array(
          '#type' => 'textfield',
          '#description' => t('This title will be displayed with the view, wherever titles are normally displayed; i.e, as the page title, block title, etc.'),
          '#default_value' => $this->get_option('title'),
        );
        break;
      case 'use_ajax':
        $form['#title'] .= t('Use AJAX when available to load this view');
        $form['description'] = array(
          '#prefix' => '<div class="description form-item">',
          '#suffix' => '</div>',
          '#value' => t('If set, this view will use an AJAX mechanism for paging, table sorting and exposed filters. This means the entire page will not refresh. It is not recommended that you use this if this view is the main content of the page as it will prevent deep linking to specific pages, but it is very useful for side content.'),
        );
        $form['use_ajax'] = array(
          '#type' => 'radios',
          '#options' => array(1 => t('Yes'), 0 => t('No')),
          '#default_value' => $this->get_option('use_ajax'),
        );
        break;
      case 'use_pager':
        $form['#title'] .= t('Use a pager for this view');
        $form['use_pager'] = array(
          '#type' => 'radios',
          '#options' => array(TRUE => t('Full pager'), 'mini' => t('Mini pager'), 0 => t('No')),
          '#default_value' => $this->get_option('use_pager'),
        );
        $form['pager_element'] = array(
          '#type' => 'textfield',
          '#title' => t('Pager element'),
          '#description' => t("Unless you're experiencing problems with pagers related to this view, you should leave this at 0. If using multiple pagers on one page you may need to set this number to a higher value so as not to conflict within the ?page= array. Large values will add a lot of commas to your URLs, so avoid if possible."),
          '#default_value' => intval($this->get_option('pager_element')),
        );
        break;
      case 'items_per_page':
        $form['#title'] .= $this->use_pager() ? t('Items per page') : t('Items to display');

        $form['items_per_page'] = array(
          '#type' => 'textfield',
          '#description' => t('The number of items to display per page. Enter 0 for no limit.'),
          '#default_value' => intval($this->get_option('items_per_page')),
        );
        $form['offset'] = array(
          '#type' => 'textfield',
          '#title' => t('Offset'),
          '#description' => t('The number of items to skip. For example, if this field is 3, the first 3 items will be skipped and not displayed.'),
          '#default_value' => intval($this->get_option('offset')),
        );
        break;
      case 'use_more':
        $form['#title'] .= t('Add a more link to the bottom of the display.');
        $form['use_more'] = array(
          '#type' => 'checkbox',
          '#title' => t('Create more link'),
          '#description' => t("This will add a more link to the bottom of this view, which will link to the page view. If you have more than one page view, the link will point to the display specified in 'Link display' above."),
          '#default_value' => $this->get_option('use_more'),
        );
        break;
      case 'distinct':
        $form['#title'] .= t('Display only distinct items, without duplicates.');
        $form['distinct'] = array(
          '#type' => 'checkbox',
          '#title' => t('Distinct'),
          '#description' => t('This will make the view display only distinct items. If there are multiple identical items, each will be displayed only once. You can use this to try and remove duplicates from a view, though it does not always work. Note that this can slow queries down, so use it with caution.'),
          '#default_value' => $this->get_option('distinct'),
        );
        break;
      case 'access':
        $form['#title'] .= t('Access restrictions');
        $form['access'] = array(
          '#prefix' => '<div class="clear-block">',
          '#suffix' => '</div>',
          '#tree' => TRUE,
        );

        $access = $this->get_option('access');
        if (empty($access)) {
          $access = array('type' => 'none', 'role' => array(), 'perm' => '');
        }
        $form['access']['type'] = array(
          '#prefix' => '<div class="views-left-25">',
          '#suffix' => '</div>',
          '#title' => t('Type'),
          '#type' => 'radios',
          '#options' => array('none' => t('Unrestricted'), 'role' => t('By role'), 'perm' => t('By permission')),
          '#default_value' => $access['type'],
        );
        $form['access']['role'] = array(
          // Add an id to the surrounding div because checkboxes don't get ids
          // as a whole group. =(
          '#prefix' => '<div class="views-left-30"><div id="edit-access-role">',
          '#suffix' => '</div></div>',
          '#type' => 'checkboxes',
          '#title' => t('If by role'),
          '#default_value' => $access['role'],
          '#options' => views_ui_get_roles(),
          '#description' => t('Only the checked roles will be able to access this display.'),
          '#process' => array('expand_checkboxes', 'views_process_dependency'),
          '#dependency' => array('radio:access[type]' => array('role')),
        );

        $perms = array();
        // Get list of permissions
        foreach (module_list(FALSE, FALSE, TRUE) as $module) {
          if ($permissions = module_invoke($module, 'perm')) {
            $perms[$module] = drupal_map_assoc($permissions);
          }
        }
        $form['access']['perm'] = array(
          '#prefix' => '<div class="views-left-30">',
          '#suffix' => '</div>',
          '#type' => 'select',
          '#options' => $perms,
          '#title' => t('If by permission'),
          '#default_value' => $access['perm'],
          '#description' => t('Only users with the selected permission flag will be able to access this display.'),
          '#process' => array('views_process_dependency'),
          // Radios don't *get* ids in this system, so we use a special format
          // for the id.
          '#dependency' => array('radio:access[type]' => array('perm')),
        );
        break;
      case 'header':
        $form['#title'] .= t('Header');
        $form['header_empty'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display even if view has no result'),
          '#default_value' => $this->get_option('header_empty'),
        );
        $form['header'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('header'),
          '#rows' => 6,
          '#description' => t('Text to display at the top of the view. May contain an explanation or links or whatever you like. Optional.'),
        );

        $form['header_format'] = filter_form($this->get_option('header_format'), NULL, array('header_format'));
        break;
      case 'footer':
        $form['#title'] .= t('Footer');
        $form['footer_empty'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display even if view has no result'),
          '#default_value' => $this->get_option('footer_empty'),
        );
        $form['footer'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('footer'),
          '#rows' => 6,
          '#description' => t('Text to display beneath the view. May contain an explanation or links or whatever you like. Optional.'),
        );

        $form['footer_format'] = filter_form($this->get_option('footer_format'), NULL, array('footer_format'));
        break;
      case 'empty':
        $form['#title'] .= t('Empty text');
        $form['empty'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('empty'),
          '#rows' => 6,
          '#description' => t('Text to display if the view has no results. Optional.'),
        );

        $form['empty_format'] = filter_form($this->get_option('empty_format'), NULL, array('empty_format'));
        break;
      case 'style_plugin':
        $form['#title'] .= t('How should this view be styled');
        $form['style_plugin'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('style', $this->get_style_type(), array($this->view->base_table)),
          '#default_value' => $this->get_option('style_plugin'),
          '#description' => t('If the style you choose has settings, be sure to click the settings button that will appear next to it in the View summary.'),
        );
        break;
      case 'style_options':
        $form['#title'] .= t('Style options');
        $style = TRUE;
        $type = 'style_plugin';

      case 'row_options':
        // if row, $style will be empty.
        if (empty($style)) {
          $form['#title'] .= t('Row style options');
          $type = 'row_plugin';
        }
        $plugin = $this->get_plugin(empty($style) ? 'row' : 'style');
        if ($plugin) {
          $form[$form_state['section']] = array(
            '#tree' => TRUE,
          );
          $plugin->options_form($form[$form_state['section']], $form_state);
        }
        break;
      case 'row_plugin':
        $form['#title'] .= t('How should each row in this view be styled');
        $form['row_plugin'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('row', $this->get_style_type(), array($this->view->base_table)),
          '#default_value' => $this->get_option('row_plugin'),
        );
        break;
      case 'link_display':
        $form['#title'] .= t('Which display to use for path');
        foreach ($this->view->display as $display_id => $display) {
          if ($display->handler->has_path()) {
            $options[$display_id] = $display->display_title;
          }
        }
        $form['link_display'] = array(
          '#type' => 'radios',
          '#options' => $options,
          '#description' => t("Which display to use to get this display's path for things like summary links, rss feed links, more links, etc."),
          '#default_value' => $this->get_link_display(),
        );
        break;
      case 'analyze-theme':
        $form['#title'] .= t('Theming information');
        $form['#help_topic'] = 'analyze-theme';
        $funcs = array();

        // Get theme functions for the display. Note that some displays may
        // not have themes. The 'feed' display, for example, completely
        // delegates to the style.
        if (!empty($this->definition['theme'])) {
          $funcs[] = $this->option_link(t('Display output'), 'analyze-theme-display') . ': '  . $this->format_themes($this->theme_functions());
        }

        $plugin = $this->get_plugin();
        if ($plugin) {
          $funcs[] = $this->option_link(t('Style output'), 'analyze-theme-style') . ': ' . $this->format_themes($plugin->theme_functions());

          if ($plugin->uses_row_plugin()) {
            $row_plugin = $this->get_plugin('row');
            if ($row_plugin) {
              $funcs[] = $this->option_link(t('Row style output'), 'analyze-theme-row') . ': ' . $this->format_themes($row_plugin->theme_functions());
            }
          }

          if ($plugin->uses_fields()) {
            $fields = $this->get_option('fields');
            foreach ($fields as $field => $info) {
              $handler = views_get_handler($info['table'], $info['field'], 'field');
              if ($handler) {
                $handler->init($this->view, $info);
                $funcs[] = $this->option_link(t('Field @field', array('@field' => $handler->ui_name())), 'analyze-theme-field') . ': ' . $this->format_themes($handler->theme_functions());
              }
            }
          }
        }

        $form['important'] = array(
          '#prefix' => '<div class="form-item description">',
          '#suffix' => '</div>',
          '#value' => '<p>' . t('<strong>Important!</strong> When you add a new template to your theme, be sure to clear the theme registry cache. You can do this by visiting administer >> site building >> themes -- just loading the page should clear the cache.') . '</p>',
        );

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => theme('item_list', $funcs),
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-display':
        $form['#title'] .= t('Theming information (display)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        if (empty($this->definition['theme'])) {
          $output .= t('This display has no theming information');
        }
        else {
          $output .= '<p>' . t('This is the default theme template used for this display.') . '</p>';
          $output .= '<pre>' . check_plain(file_get_contents('./' . $this->definition['path'] . '/' . strtr($this->definition['theme'], '_', '-') . '.tpl.php')) . '</pre>';
        }
        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-style':
        $form['#title'] .= t('Theming information (style)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        $plugin = $this->get_plugin();

        if (empty($plugin->definition['theme'])) {
          $output .= t('This display has no style theming information');
        }
        else {
          $output .= '<p>' . t('This is the default theme template used for this style.') . '</p>';
          $output .= '<pre>' . check_plain(file_get_contents('./' . $plugin->definition['path'] . '/' . strtr($plugin->definition['theme'], '_', '-') . '.tpl.php')) . '</pre>';
        }
        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-row':
        $form['#title'] .= t('Theming information (row style)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        $plugin = $this->get_plugin('row');

        if (empty($plugin->definition['theme'])) {
          $output .= t('This display has no row style theming information');
        }
        else {
          $output .= '<p>' . t('This is the default theme template used for this row style.') . '</p>';
          $output .= '<pre>' . check_plain(file_get_contents('./' . $plugin->definition['path'] . '/' . strtr($plugin->definition['theme'], '_', '-') . '.tpl.php')) . '</pre>';
        }
        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-field':
        $form['#title'] .= t('Theming information (row style)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        $output .= '<p>' . t('This is the default theme template used for this row style.') . '</p>';

        // Field templates aren't registered the normal way...and they're always
        // this one, anyhow.
        $output .= '<pre>' . check_plain(file_get_contents(drupal_get_path('module', 'views') . '/theme/views-view-field.tpl.php')) . '</pre>';

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );
        $form_state['ok_button'] = TRUE;
        break;
    }
  }

  /**
   * Format a list of theme templates for output by the theme info helper.
   */
  function format_themes($themes) {
    init_theme();
    $registry = theme_get_registry();

    // Run through the theme engine variables, if necessary
    global $theme_engine;
    $extension = '.tpl.php';
    if (isset($theme_engine)) {
      $extension_function = $theme_engine .'_extension';
      if (function_exists($extension_function)) {
        $extension = $extension_function();
      }
    }

    $output = '';
    $found = FALSE;
    foreach ($themes as $theme) {
      $template = strtr($theme, '_', '-') . $extension;
      if (!$found && !empty($registry[$theme])) {
        $template = '<strong>' . $template . '</strong>';
        $found = TRUE;
      }
      $fixed[] = $template;
    }

    return implode(', ', array_reverse($fixed));
  }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) {
    switch ($form_state['section']) {
      case 'style_options':
        $style = TRUE;
      case 'row_options':
        // if row, $style will be empty.
        $plugin = $this->get_plugin(empty($style) ? 'row' : 'style');
        if ($plugin) {
          $plugin->options_validate($form[$form_state['section']], $form_state);
        }
        break;
      case 'access':
        $access = $form_state['values']['access'];
        if ($access['type'] == 'role' && !array_filter($access['role'])) {
          form_error($form, t('You must select at least one role if type is "by role"'));
        }
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    $section = $form_state['section'];
    switch ($section) {
      case 'display_title':
        $this->display->display_title = $form_state['values']['display_title'];
        break;
      case 'title':
      case 'access':
      case 'link_display':
      case 'use_ajax':
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'use_pager':
        $this->set_option($section, $form_state['values'][$section]);
        $this->set_option('pager_element', intval($form_state['values']['pager_element']));
        break;
      case 'items_per_page':
        $this->set_option($section, intval($form_state['values'][$section]));
        $this->set_option('offset', intval($form_state['values']['offset']));
        break;
      case 'use_more':
      case 'distinct':
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'row_plugin':
        // This if prevents resetting options to default if they don't change
        // the plugin.
        if ($this->get_option($section) != $form_state['values'][$section]) {
          $this->set_option($section, $form_state['values'][$section]);
          $plugin = views_get_plugin('row', $form_state['values'][$section]);
          if ($plugin) {
            $row_options = array();
            $plugin->init($this->view, $this->display);
            $plugin->options($row_options);
            $this->set_option('row_options', $row_options);

            // send ajax form to options page if we use it.
            if (!empty($plugin->definition['uses options'])) {
              views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('row_options'));
            }
          }
        }
        break;
      case 'style_plugin':
        // This if prevents resetting options to default if they don't change
        // the plugin.
        if ($this->get_option($section) != $form_state['values'][$section]) {
          $this->set_option($section, $form_state['values'][$section]);
          $plugin = views_get_plugin('style', $form_state['values'][$section]);
          if ($plugin) {
            $style_options = array();
            $plugin->init($this->view, $this->display);
            $plugin->options($style_options);
            $this->set_option('style_options', $style_options);

            // send ajax form to options page if we use it.
            if (!empty($plugin->definition['uses options'])) {
              views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('style_options'));
            }
          }
        }
        break;
      case 'style_options':
        $style = TRUE;
      case 'row_options':
        // if row, $style will be empty.
        $plugin = $this->get_plugin(empty($style) ? 'row' : 'style');
        if ($plugin) {
          $plugin->options_submit($form[$section], $form_state);
        }
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'header':
      case 'footer':
      case 'empty':
        $this->set_option($section, $form_state['values'][$section]);
        $this->set_option($section . '_format', $form_state['values'][$section . '_format']);
        if ($section != 'empty') {
          $this->set_option($section . '_empty', $form_state['values'][$section . '_empty']);
        }
        break;
    }
  }

  /**
   * Add an override button for a given section, allowing the user to
   * change whether this info is stored on the default display or on
   * the current display.
   */
  function add_override_button(&$form, $section) {
    if ($this->is_default_display()) {
      return;
    }

    $form['override'] = array(
      '#prefix' => '<div class="views-override clear-block">',
      '#suffix' => '</div>',
    );
    if (!empty($this->display->display_options['defaults'][$section])) {
      $form['override']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Override'),
        '#submit' => array('views_ui_edit_display_form_override'),
      );
      $form['override']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => theme('advanced_help_topic', 'views', 'overrides') . t('Status: using default values.'),
        '#suffix' => '</div>',
      );
    }
    else {
      $form['override']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Use default'),
        '#submit' => array('views_ui_edit_display_form_override'),
      );
      $form['override']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => theme('advanced_help_topic', 'views', 'overrides') . t('Status: using overridden values.'),
        '#suffix' => '</div>',
      );
    }
  }

  /**
   * If override/revert was clicked, perform the proper toggle.
   */
  function options_override($form, &$form_state) {
    $this->set_override($form_state['section']);
  }

  /**
   * Flip the override setting for the given section.
   */
  function set_override($section) {
    $options = $this->defaultable_sections($section);
    if (!$options) {
      return;
    }

    $new_state = empty($this->display->display_options['defaults'][$section]);

    // For each option that is part of this group, fix our settings.
    foreach ($options as $option) {
      if ($new_state) {
        // Revert to defaults.
        unset($this->display->display_options[$option]);
      }
      else {
        // copy existing values into our display.
        $this->display->display_options[$option] = $this->get_option($option);
      }
      $this->display->display_options['defaults'][$option] = $new_state;
    }
  }

  /**
   * Inject anything into the query that the display handler needs.
   */
  function query() {
    // Make the query distinct if the option was set.
    if ($this->get_option('distinct')) {
      $this->view->query->set_distinct();
    }
  }

  /**
   * Not all display plugins will support filtering
   */
  function render_filters() { }

  /**
   * Render the 'more' link
   */
  function render_more_link() {
    if ($this->use_more()) {
      $path = $this->get_path();
      if ($path) {
        $path = $this->view->get_url(NULL, $path);
        $theme = views_theme_functions('views_more', $this->view, $this->display);
        $path = check_url(url($path));
        return theme($theme, $path);
      }
    }
  }

  /**
   * Render a text area, using hte proper format.
   */
  function render_textarea($area) {
    $value = $this->get_option($area);
    if ($value) {
      return check_markup($value, $this->get_option($area . '_format'), FALSE);
    }
  }

  /**
   * Render the header of the view.
   */
  function render_header() {
    if (!empty($this->view->result) || $this->get_option('header_empty')) {
      return $this->render_textarea('header');
    }
  }

  /**
   * Render the footer of the view.
   */
  function render_footer() {
    if (!empty($this->view->result) || $this->get_option('footer_empty')) {
      return $this->render_textarea('footer');
    }
  }

  /**
   * Render the empty text of the view.
   */
  function render_empty() { return $this->render_textarea('empty'); }
  /**
   * If this display creates a block, implement one of these.
   */
  function hook_block($op = 'list', $delta = 0, $edit = array()) { return array(); }

  /**
   * If this display creates a page with a menu item, implement it here.
   */
  function hook_menu() { return array(); }

  /**
   * Render this display.
   */
  function render() {
    return theme($this->theme_functions(), $this->view);
  }

  /**
   * Determine if the user has access to this display of the view.
   */
  function access($account) {
    $access = $this->get_option('access');
    switch ($access['type']) {
      case 'role':
        $roles = array_keys($account->roles);
        $roles[] = $account->uid ? DRUPAL_AUTHENTICATED_RID : DRUPAL_ANONYMOUS_RID;
        return array_intersect(array_filter($access['role']), $roles);
      case 'perm':
        return user_access($access['perm'], $account);
      case 'none':
      default:
        return TRUE;
    }
  }

  /**
   * Set up any variables on the view prior to execution. These are separated
   * from execute because they are extremely common and unlikely to be
   * overridden on an individual display.
   */
  function pre_execute() {
    $this->view->set_use_ajax($this->use_ajax());
    // Copy pager information from the display.
    $this->view->set_use_pager($this->use_pager());
    $this->view->set_pager_element($this->get_option('pager_element'));
    $this->view->set_items_per_page($this->get_option('items_per_page'));
    $this->view->set_offset($this->get_option('offset'));
  }

  /**
   * When used externally, this is how a view gets run and returns
   * data in the format required.
   *
   * The base class cannot be executed.
   */
  function execute() { }

  /**
   * Fully render the display for the purposes of a live preview or
   * some other AJAXy reason.
   */
  function preview() { return $this->view->render(); }

  /**
   * Displays can require a certain type of style plugin. By default, they will
   * be 'normal'.
   */
  function get_style_type() { return 'normal'; }

  function validate() {
    $errors = array();
    // Make sure displays that use fields HAVE fields.
    if ($this->uses_fields() && !$this->get_option('fields')) {
      $errors[] = t('Display @display uses fields but there are none defined for it.', array('@display' => $this->display->display_title));
    }

    if ($this->has_path() && !$this->get_option('path')) {
      $errors[] = t('Display @display uses path but path is undefined.', array('@display' => $this->display->display_title));
    }

    $style = $this->get_plugin();
    if (empty($style)) {
      $errors[] = t('Display @display has an invalid style plugin.', array('@display' => $this->display->display_title));
    }
    else {
      $result = $style->validate();
      if (!empty($result) && is_array($result)) {
        $errors = array_merge($errors, $result);
      }
    }
    return $errors;
  }
}

/**
 * A plugin to handle defaults on a view.
 */
class views_plugin_display_default extends views_plugin_display {
  /**
   * Determine if this display is the 'default' display which contains
   * fallback settings
   */
  function is_default_display() { return TRUE; }

  function options(&$display) {
    // Make sure the default display has a style plugin to start with.
    $display->display_options['style_plugin'] = 'default';
    $display->display_options['style_options'] = array();
    $display->display_options['row_plugin'] = 'fields';
    $display->display_options['row_options'] = array();
    $display->display_options['relationships'] = array();
    $display->display_options['fields'] = array();
    $display->display_options['sorts'] = array();
    $display->display_options['arguments'] = array();
    $display->display_options['filters'] = array();

    $display->display_options['items_per_page'] = 10;
  }

  /**
   * The default execute handler fully renders the view.
   *
   * For the simplest use:
   * @code
   *   $output = $view->execute_display('default', $args);
   * @endcode
   *
   * For more complex usages, a view can be partially built:
   * @code
   *   $view->set_arguments($args);
   *   $view->build('default'); // Build the query
   *   $view->execute(); // Run the query
   *   $output = $view->render(); // Render the view
   * @endcode
   *
   * If short circuited at any point, look in $view->build_info for
   * information about the query. After execute, look in $view->result
   * for the array of objects returned from db_query.
   *
   * You can also do:
   * @code
   *   $view->set_arguments($args);
   *   $output = $view->render('default'); // Render the view
   * @endcode
   *
   * This illustrates that render is smart enough to call build and execute
   * if these items have not already been accomplished.
   *
   * Note that execute also must accomplish other tasks, such
   * as setting page titles, breadcrumbs, and generating exposed filter
   * data if necessary.
   */
  function execute() {
    return $this->view->render($this->display->id);
  }
}

/**
 * The plugin that handles a full page.
 */
class views_plugin_display_page extends views_plugin_display {
  /**
   * The page display has a path.
   */
  function has_path() { return TRUE; }
  function uses_breadcrumb() { return TRUE; }

  /**
   * Add this display's path information to Drupal's menu system.
   */
  function execute_hook_menu() {
    $items = array();
    // Replace % with the link to our standard views argument loader
    // views_arg_load -- which lives in views.module

    $bits = explode('/', $this->get_option('path'));
    $page_arguments = array($this->view->name, $this->display->id);

    // Replace % with %views_arg for menu autoloading and add to the
    // page arguments so the argument actually comes through.
    foreach($bits as $pos => $bit) {
      if ($bit == '%') {
        $bits[$pos] = '%views_arg';
        $page_arguments[] = $pos;
      }
    }

    $path = implode('/', $bits);

    if ($path) {
      // NOTE: This is the very simple 'menu normal item' version. The
      // tab version needs to come later. Maybe it should be its own plugin.
      $items[$path] = array(
        // default views page entry
        'page callback' => 'views_page',
        'page arguments' => $page_arguments,
        // Default access check (per display)
        'access callback' => 'views_access',
        'access arguments' => array(array($this->view->name, $this->display->id)),
        // Identify URL embedded arguments and correlate them to a handler
        'load arguments'  => array($this->view->name, $this->display->id, '%index'),
      );
      $menu = $this->get_option('menu');
      if (empty($menu)) {
        $menu = array('type' => 'none');
      }
      // Set the title if we have one.
      if ($menu['type'] != 'none') {
        $items[$path]['title'] = $menu['title'];
      }

      if (isset($menu['weight'])) {
        $items[$path]['weight'] = intval($menu['weight']);
      }

      switch ($menu['type']) {
        case 'none':
        default:
          $items[$path]['type'] = MENU_CALLBACK;
          break;
        case 'normal':
          $items[$path]['type'] = MENU_NORMAL_ITEM;
          break;
        case 'tab':
          $items[$path]['type'] = MENU_LOCAL_TASK;
          break;
        case 'default tab':
          $items[$path]['type'] = MENU_DEFAULT_LOCAL_TASK;
          break;
      }

      // If this is a 'default' tab, check to see if we have to create teh
      // parent menu item.
      if ($menu['type'] == 'default tab') {
        $tab_options = $this->get_option('tab_options');
        if (!empty($tab_options['type']) && $tab_options['type'] != 'none') {
          $bits = explode('/', $path);
          // Remove the last piece.
          $bit = array_pop($bits);

          // we can't do this if they tried to make the last path bit variable.
          // @todo: We can validate this.
          if ($bit != '%views_arg' && !empty($bits)) {
            $default_path = implode('/', $bits);
            $items[$default_path] = array(
              // default views page entry
              'page callback' => 'views_page',
              'page arguments' => $page_arguments,
              // Default access check (per display)
              'access callback' => 'views_access',
              'access arguments' => array(array($this->view->name, $this->display->id)),
              // Identify URL embedded arguments and correlate them to a handler
              'load arguments'  => array($this->view->name, $this->display->id, '%index'),
              'title' => $tab_options['title'],
            );
            switch ($tab_options['type']) {
              default:
              case 'normal':
                $items[$default_path]['type'] = MENU_NORMAL_ITEM;
                break;
              case 'tab':
                $items[$default_path]['type'] = MENU_LOCAL_TASK;
                break;
            }
            if (isset($tab_options['weight'])) {
              $items[$default_path]['weight'] = intval($tab_options['weight']);
            }
          }

        }
      }
    }

    return $items;
  }

  /**
   * The display page handler returns a normal view, but it also does
   * a drupal_set_title for the page, and does a views_set_page_view
   * on the view.
   */
  function execute() {
    // Let the world know that this is the page view we're using.
    views_set_page_view($this);

    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    $this->view->build();
    $this->view->get_breadcrumb(TRUE);

    // And the title, which is much easier.
    drupal_set_title(filter_xss_admin($this->view->get_title()));

    // And now render the view.
    return $this->view->render();
  }

  /**
   * Provide the summary for page options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    $categories['page'] = array(
      'title' => t('Page settings'),
    );

    $path = $this->get_option('path');
    if (empty($path)) {
      $path = t('None');
    }

    if (strlen($path) > 16) {
      $path = substr($path, 0, 16) . '...';
    }

    $options['path'] = array(
      'category' => 'page',
      'title' => t('Path'),
      'value' => $path,
    );

    $menu = $this->get_option('menu');
    if (!is_array($menu)) {
      $menu = array('type' => 'none');
    }
    switch($menu['type']) {
      case 'none':
      default:
        $menu_str = t('No menu');
        break;
      case 'normal':
        $menu_str = t('Normal: @title', array('@title' => $menu['title']));
        break;
      case 'tab':
      case 'default tab':
        $menu_str = t('Tab: @title', array('@title' => $menu['title']));
        break;
    }

    if (strlen($menu_str) > 16) {
      $menu_str = substr($menu_str, 0, 16) . '...';
    }

    $options['menu'] = array(
      'category' => 'page',
      'title' => t('Menu'),
      'value' => $menu_str,
    );

    // This adds a 'Settings' link to the style_options setting if the style has options.
    if ($menu['type'] == 'default tab') {
      $options['menu']['links']['tab_options'] = t('Change settings for the parent menu');
    }
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'path':
        $form['#title'] .= t('The menu path or URL of this view');
        $form['#help_topic'] = 'path';
        $form['path'] = array(
          '#type' => 'textfield',
          '#description' => t('This view will be displayed by visiting this path on your site. You may use "%" in your URL to represent values that will be used for arguments: For example, "node/%/feed".'),
          '#default_value' => $this->get_option('path'),
          '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
        );
        break;
      case 'menu':
        $form['#title'] .= t('Menu item entry');
        $form['#help_topic'] = 'menu';
        $form['menu'] = array(
          '#prefix' => '<div class="clear-block">',
          '#suffix' => '</div>',
          '#tree' => TRUE,
        );
        $menu = $this->get_option('menu');
        if (empty($menu)) {
          $menu = array('type' => 'none', 'title' => '', 'weight' => 0);
        }
        $form['menu']['type'] = array(
          '#prefix' => '<div class="views-left-30">',
          '#suffix' => '</div>',
          '#title' => t('Type'),
          '#type' => 'radios',
          '#options' => array(
            'none' => t('No menu entry'),
            'normal' => t('Normal menu entry'),
            'tab' => t('Menu tab'),
            'default tab' => t('Default menu tab')
          ),
          '#default_value' => $menu['type'],
        );
        $form['menu']['title'] = array(
          '#prefix' => '<div class="views-left-50">',
          '#title' => t('Title'),
          '#type' => 'textfield',
          '#default_value' => $menu['title'],
          '#description' => t('If set to normal or tab, enter the text to use for the menu item.'),
          '#process' => array('views_process_dependency'),
          '#dependency' => array('radio:menu[type]' => array('normal', 'tab', 'default tab')),
        );
        $form['menu']['weight'] = array(
          '#suffix' => '</div>',
          '#title' => t('Weight'),
          '#type' => 'textfield',
          '#default_value' => isset($menu['weight']) ? $menu['weight'] : 0,
          '#description' => t('If set to tab, enter the weight of the item. The lower th weight the higher/further left it will appear.'),
          '#process' => array('views_process_dependency'),
          '#dependency' => array('radio:menu[type]' => array('tab', 'default tab')),
        );
        break;
      case 'tab_options':
        $form['#title'] .= t('Default tab options');
        $tab_options = $this->get_option('tab_options');
        if (empty($tab_options)) {
          $tab_options = array('type' => 'none', 'title' => '', 'weight' => 0);
        }

        $form['tab_markup'] = array(
          '#prefix' => '<div class="form-item description">',
          '#suffix' => '</div>',
          '#value' => t('When providing a menu item as a tab, Drupal needs to know what the parent menu item of that tab will be. Sometimes the parent will already exist, but other times you will need to have one created. The path of a parent item will always be the same path with the last part left off. i.e, if the path to this view is <em>foo/bar/baz</em>, the parent path would be <em>foo/bar</em>.'),
        );

        $form['tab_options'] = array(
          '#prefix' => '<div class="clear-block">',
          '#suffix' => '</div>',
          '#tree' => TRUE,
        );
        $form['tab_options']['type'] = array(
          '#prefix' => '<div class="views-left-25">',
          '#suffix' => '</div>',
          '#title' => t('Parent menu item'),
          '#type' => 'radios',
          '#options' => array('none' => t('Already exists'), 'normal' => t('Normal menu item'), 'tab' => t('Menu tab')),
          '#default_value' => $tab_options['type'],
        );
        $form['tab_options']['title'] = array(
          '#prefix' => '<div class="views-left-75">',
          '#title' => t('Title'),
          '#type' => 'textfield',
          '#default_value' => $tab_options['title'],
          '#description' => t('If creating a parent menu item, enter the title of the item.'),
          '#process' => array('views_process_dependency'),
          '#dependency' => array('radio:tab_options[type]' => array('normal', 'tab')),
        );
        $form['tab_options']['weight'] = array(
          '#suffix' => '</div>',
          '#title' => t('Tab weight'),
          '#type' => 'textfield',
          '#default_value' => $tab_options['weight'],
          '#size' => 5,
          '#description' => t('If the parent menu item is a tab, enter the weight of the tab. The lower the number, the more to the left it will be.'),
          '#process' => array('views_process_dependency'),
          '#dependency' => array('radio:tab_options[type]' => array('tab')),
        );
        break;
    }
  }

  function options_validate($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_validate($form, $form_state);
    switch ($form_state['section']) {
      case 'path':
        if (strpos($form_state['values']['path'], '$arg') !== FALSE) {
          form_error($form['path'], t('"$arg" is no longer supported. Use % instead.'));
        }
        break;
      case 'menu':
        $path = $this->get_option('path');
        if ($form_state['values']['menu']['type'] == 'normal' && strpos($path, '%') !== FALSE) {
          form_error($form['menu']['type'], t('Views cannot create normal menu items for paths with a % in them.'));
        }

        if ($form_state['values']['menu']['type'] == 'default tab' || $form_state['values']['menu']['type'] == 'tab') {
          $bits = explode('/', $path);
          $last = array_pop($bits);
          if ($last == '%') {
            form_error($form['menu']['type'], t('A display whose path ends with a % cannot be a tab.'));
          }
        }

        if ($form_state['values']['menu']['type'] != 'none' && empty($form_state['values']['menu']['title'])) {
          form_error($form['menu']['title'], t('Title is required for this menu type.'));
        }
        break;
    }
  }

  function options_submit($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'path':
        $this->set_option('path', $form_state['values']['path']);
        break;
      case 'menu':
        $this->set_option('menu', $form_state['values']['menu']);
        // send ajax form to options page if we use it.
        if ($form_state['values']['menu']['type'] == 'default tab') {
          views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('tab_options'));
        }
        break;
      case 'tab_options':
        $this->set_option('tab_options', $form_state['values']['tab_options']);
        break;
    }
  }

  function validate() {
    $errors = parent::validate();

    $menu = $this->get_option('menu');
    if (!empty($menu['type']) && $menu['type'] != 'none' && empty($menu['title'])) {
      $errors[] = t('Display @display is set to use a menu but the menu title is not set.', array('@display' => $this->display->display_title));
    }

    if ($menu['type'] == 'default tab') {
      $tab_options = $this->get_option('tab_options');
      if (!empty($tab_options['type']) && $tab_options['type'] != 'none' && empty($tab_options['title'])) {
        $errors[] = t('Display @display is set to use a parent menu but the parent menu title is not set.', array('@display' => $this->display->display_title));
      }
    }

    return $errors;
  }
}

/**
 * The plugin that handles a block.
 */
class views_plugin_display_block extends views_plugin_display {
  /**
   * The default block handler doesn't support configurable items,
   * but extended block handlers might be able to do interesting
   * stuff with it.
   */
  function execute_hook_block($op = 'list', $delta = 0, $edit = array()) {
    if ($op == 'list') {
      $delta = $this->view->name . '-' . $this->display->id;
      $desc = $this->get_option('block_description');

      if (empty($desc)) {
        $desc = $this->view->name;
      }
      return array(
        $delta => array(
          'info' => $desc,
          'cache' => $this->get_cache_type()
        )
      );
    }
  }

  /**
   * The display block handler returns the structure necessary for a block.
   */
  function execute() {
    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    $info['content'] = $this->view->render();
    $info['subject'] = filter_xss_admin($this->view->get_title());
    return $info;
  }

  /**
   * Provide the summary for page options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    $categories['block'] = array(
      'title' => t('Block settings'),
    );

    $block_description = $this->get_option('block_description');
    if (empty($block_description)) {
      $block_description = t('None');
    }

    if (strlen($block_description) > 16) {
      $block_description = substr($block_description, 0, 16) . '...';
    }

    $options['block_description'] = array(
      'category' => 'block',
      'title' => t('Admin'),
      'value' => $block_description,
    );

    $cache_type = $this->get_option('block_caching');
    if (empty($cache_type)) {
      $cache_type = BLOCK_NO_CACHE;
    }

    $types = $this->block_caching_modes();
    $options['block_caching'] = array(
      'category' => 'block',
      'title' => t('Caching'),
      'value' => $types[$this->get_cache_type()],
    );
  }

  /**
   * Provide a list of core's block caching modes.
   */
  function block_caching_modes() {
    return array(
      BLOCK_NO_CACHE => t('Do not cache'),
      BLOCK_CACHE_GLOBAL => t('Cache once for everything (global)'),
      BLOCK_CACHE_PER_PAGE => t('Per page'),
      BLOCK_CACHE_PER_ROLE => t('Per role'),
      BLOCK_CACHE_PER_ROLE | BLOCK_CACHE_PER_PAGE => t('Per role per page'),
      BLOCK_CACHE_PER_USER => t('Per user'),
      BLOCK_CACHE_PER_USER | BLOCK_CACHE_PER_PAGE => t('Per user per page'),
    );
  }

  /**
   * Provide a single method to figure caching type, keeping a sensible default
   * for when it's unset.
   */
  function get_cache_type() {
    $cache_type = $this->get_option('block_caching');
    if (empty($cache_type)) {
      $cache_type = BLOCK_NO_CACHE;
    }
    return $cache_type;
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'block_description':
        $form['#title'] .= t('Block admin description');
        $form['block_description'] = array(
          '#type' => 'textfield',
          '#description' => t('This will appear as the name of this block in administer >> site building >> blocks.'),
          '#default_value' => $this->get_option('block_description'),
        );
        break;
      case 'block_caching':
        $form['#title'] .= t('Block caching type');

        $form['block_caching'] = array(
          '#type' => 'radios',
          '#description' => t("This sets the default status for Drupal's built-in block caching method; this requires that caching be turned on in block administration, and be careful because you have little control over when this cache is flushed."),
          '#options' => $this->block_caching_modes(),
          '#default_value' => $this->get_cache_type(),
        );
        break;
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'block_description':
        $this->set_option('block_description', $form_state['values']['block_description']);
        break;
      case 'block_caching':
        $this->set_option('block_caching', $form_state['values']['block_caching']);
        break;
    }
  }

  /**
   * Block views do not use exposed widgets.
   */
  function uses_exposed() { return FALSE; }
}

/**
 * The plugin that handles an attachment display.
 */
class views_plugin_display_attachment extends views_plugin_display {
  function options(&$display) {
    parent::options($display);
    $display->display_options['attachment_position'] = 'before';
    $display->display_options['inherit_arguments'] = TRUE;
    $display->display_options['displays'] = array();
  }

  function execute() {
    return $this->view->render($this->display->id);
  }

  function attachment_positions($position = NULL) {
    $positions = array(
      'before' => t('Before'),
      'after' => t('After'),
      'both' => t('Both'),
    );

    if ($position) {
      return $positions[$position];
    }

    return $positions;
  }

  /**
   * Provide the summary for attachment options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    $categories['attachment'] = array(
      'title' => t('Attachment settings'),
    );

    $options['inherit_arguments'] = array(
      'category' => 'attachment',
      'title' => t('Inherit arguments'),
      'value' => $this->get_option('inherit_arguments') ? t('Yes') : t('No'),
    );

    $options['attachment_position'] = array(
      'category' => 'attachment',
      'title' => t('Position'),
      'value' => $this->attachment_positions($this->get_option('attachment_position')),
    );

    $displays = array_filter($this->get_option('displays'));
    if (count($displays) > 1) {
      $attach_to = t('Multiple displays');
    }
    else if (count($displays) == 1) {
      $display = array_shift($displays);
      if (!empty($this->view->display[$display])) {
        $attach_to = $this->view->display[$display]->display_title;
      }
    }

    if (!isset($attach_to)) {
      $attach_to = t('None');
    }

    $options['displays'] = array(
      'category' => 'attachment',
      'title' => t('Attach to'),
      'value' => $attach_to,
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'inherit_arguments':
        $form['#title'] .= t('Inherit arguments');
        $form['inherit_arguments'] = array(
          '#type' => 'checkbox',
          '#title' => t('Inherit'),
          '#description' => t('Should this display inherit its arguments from the parent display to which it is attached?'),
          '#default_value' => $this->get_option('inherit_arguments'),
        );
        break;
      case 'attachment_position':
        $form['#title'] .= t('Position');
        $form['attachment_position'] = array(
          '#type' => 'radios',
          '#description' => t('Attach before or after the parent display?'),
          '#options' => $this->attachment_positions(),
          '#default_value' => $this->get_option('attachment_position'),
        );
        break;
      case 'displays':
        $form['#title'] .= t('Attach to');
        $displays = array();
        foreach ($this->view->display as $display_id => $display) {
          if (!empty($display->handler) && $display->handler->accept_attachments()) {
            $displays[$display_id] = $display->display_title;
          }
        }
        $form['displays'] = array(
          '#type' => 'checkboxes',
          '#description' => t('Select which display or displays this should attach to.'),
          '#options' => $displays,
          '#default_value' => $this->get_option('displays'),
        );
        break;
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'inherit_arguments':
      case 'attachment_position':
      case 'displays':
        $this->set_option($form_state['section'], $form_state['values'][$form_state['section']]);
        break;
    }
  }

  /**
   * Attach to another view.
   */
  function attach_to($display_id) {
    $displays = $this->get_option('displays');
    if (empty($displays[$display_id])) {
      return;
    }

    // Get a fresh view because our current one has a lot of stuff on it because it's
    // already been executed.
    $view = $this->view->clone_view();

    $args = $this->get_option('inherit_arguments') ? $this->view->args : array();

    // because of this, it is very very important that displays that can accept
    // attachments not also be attachments, or this could explode messily.
    $attachment = $view->execute_display($this->display->id, $args);

    switch ($this->get_option('attachment_position')) {
      case 'before':
        $this->view->attachment_before .= $attachment;
        break;
      case 'after':
        $this->view->attachment_after .= $attachment;
        break;
      case 'both':
        $this->view->attachment_before .= $attachment;
        $this->view->attachment_after .= $attachment;
        break;
    }
  }

  /**
   * Attachment views do not use exposed widgets.
   */
  function uses_exposed() { return FALSE; }
}

/**
 * The plugin that handles a feed, such as RSS or atom.
 *
 * For the most part, feeds are page displays but with some subtle differences.
 */
class views_plugin_display_feed extends views_plugin_display_page {
  function uses_breadcrumb() { return FALSE; }
  function get_style_type() { return 'feed'; }

  /**
   * Feeds do not go through the normal page theming mechanism. Instead, they
   * go through their own little theme function and then return NULL so that
   * Drupal believes that the page has already rendered itself...which it has.
   */
  function execute() {
    print $this->view->render();
  }

  function preview() {
    return '<pre>' . check_plain($this->view->render()) . '</pre>';
  }

  /**
   * Instead of going through the standard views_view.tpl.php, delegate this
   * to the style handler.
   */
  function render() {
    return $this->view->style_plugin->render($this->view->result);
  }

  function defaultable_sections($section = NULL) {
    if (in_array($section, array('style_options', 'style_plugin', 'row_options', 'row_plugin',))) {
      return FALSE;
    }

    return parent::defaultable_sections($section);
  }

  function options(&$display) {
    parent::options($display);
    $display->display_options['displays'] = array();
    $display->display_options['style_plugin'] = 'rss';
    $display->display_options['style_options'] = array('mission_description' => FALSE, 'description' => '');
    $display->display_options['row_plugin'] = '';
    $display->display_options['defaults']['style_plugin'] = FALSE;
    $display->display_options['defaults']['style_options'] = FALSE;
    $display->display_options['defaults']['row_plugin'] = FALSE;
    $display->display_options['defaults']['row_options'] = FALSE;

  }

  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    // Since we're childing off the 'page' type, we'll still *call* our
    // category 'page' but let's override it so it says feed settings.
    $categories['page'] = array(
      'title' => t('Feed settings'),
    );

    // I don't think we want to give feeds menus directly.
    unset($options['menu']);

    $displays = array_filter($this->get_option('displays'));
    if (count($displays) > 1) {
      $attach_to = t('Multiple displays');
    }
    else if (count($displays) == 1) {
      $display = array_shift($displays);
      if (!empty($this->view->display[$display])) {
        $attach_to = $this->view->display[$display]->display_title;
      }
    }

    if (!isset($attach_to)) {
      $attach_to = t('None');
    }

    $options['displays'] = array(
      'category' => 'page',
      'title' => t('Attach to'),
      'value' => $attach_to,
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here.
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'displays':
        $form['#title'] .= t('Attach to');
        $displays = array();
        foreach ($this->view->display as $display_id => $display) {
          if (!empty($display->handler) && $display->handler->accept_attachments()) {
            $displays[$display_id] = $display->display_title;
          }
        }
        $form['displays'] = array(
          '#type' => 'checkboxes',
          '#description' => t('Attach before or after the parent display?'),
          '#options' => $displays,
          '#default_value' => $this->get_option('displays'),
        );
        break;
      case 'path':
        $form['path']['#description'] = t('This view will be displayed by visiting this path on your site. It is recommended that the path be something like "path/%/%/feed" or "path/%/%/rss.xml", putting one % in the path for each argument you have defined in the view.');
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'displays':
        $this->set_option($form_state['section'], $form_state['values'][$form_state['section']]);
        break;
    }
  }

  /**
   * Attach to another view.
   */
  function attach_to($display_id) {
    $displays = $this->get_option('displays');
    if (empty($displays[$display_id])) {
      return;
    }

    // Defer to the feed style; it may put in meta information, and/or
    // attach a feed icon.
    $plugin = $this->get_plugin();
    if ($plugin) {
      $plugin->attach_to($display_id, $this->get_path(), $this->get_option('title'));
    }
  }
}

/**
 * @}
 */

/**
 * @defgroup views_style_plugins Views' style plugins
 * @{
 * Style plugins control how a view is rendered. For example, they
 * can choose to display a collection of fields, node_view() output,
 * table output, or any kind of crazy output they want.
 *
 * Many style plugins can have an optional 'row' plugin, that displays
 * a single record. Not all style plugins can utilize this, so it is
 * up to the plugin to set this up and call through to the row plugin.
 *
 * @see hook_views_plugins
 */

/**
 * Base class to define a style plugin handler.
 */
class views_plugin_style extends views_plugin {
  /**
   * Initialize a style plugin.
   *
   * @param $view
   * @param $display
   * @param $options
   *   The style options might come externally as the style can be sourced
   *   from at least two locations. If it's not included, look on the display.
   */
  function init(&$view, &$display, $options = NULL) {
    $this->view = &$view;
    $this->display = &$display;
    if (isset($options)) {
      $this->options = $options;
    }
    else {
      $this->options = $display->handler->get_option('style_options');
    }

    if ($this->uses_row_plugin() && $display->handler->get_option('row_plugin')) {
      $this->row_plugin = $display->handler->get_plugin('row');
    }
  }

  /**
   * Return TRUE if this style also uses a row plugin.
   */
  function uses_row_plugin() {
    return !empty($this->definition['uses row plugin']);
  }

  /**
   * Return TRUE if this style also uses fields.
   */
  function uses_fields() {
    // If we use a row plugin, ask the row plugin. Chances are, we don't
    // care, it does.
    if ($this->uses_row_plugin() && !empty($this->row_plugin)) {
      return $this->row_plugin->uses_fields();
    }
    // Otherwise, maybe we do.
    return !empty($this->definition['uses fields']);
  }

  /**
   * Static member function to set default options.
   */
  function options(&$options) { }

  /**
   * Called by the view builder to see if this style handler wants to
   * interfere with the sorts. If so it should build; if it returns
   * any non-TRUE value, normal sorting will NOT be added to the query.
   */
  function build_sort() { return TRUE; }

  /**
   * Allow the style to do stuff before each row is rendered.
   *
   * @param $result
   *   The full array of results from the query.
   */
  function pre_render($result) {
    if (!empty($this->row_plugin)) {
      $this->row_plugin->pre_render($result);
    }
  }

  /**
   * Render the display in this style.
   *
   * @param $rows
   *   The full results of the query, loaded into an array of objects.
   */
  function render() {
    if (empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }
    $rows = array();
    foreach ($this->view->result as $row) {
      $rows[] = $this->row_plugin->render($row);
    }
    return theme($this->theme_functions(), $this->view, $this->options, $rows);
  }

  function validate() {
    $errors = parent::validate();

    if ($this->uses_row_plugin()) {
      $plugin = $this->display->handler->get_plugin('row');
      if (empty($plugin)) {
        $errors[] = t('Style @style requires a row style but the row plugin is invalid.', array('@style' => $this->definition['title']));
      }
    }
    return $errors;
  }
}

/**
 * Default style plugin to render rows one after another with no
 * decorations.
 */
class views_plugin_style_default extends views_plugin_style {
  function options_form(&$form, &$form_state) {
    // @todo -- separator option
  }
}

/**
 * Style plugin to render each item in an ordered or unordered list
 */
class views_plugin_style_list extends views_plugin_style {
  /**
   * Set default options
   */
  function options(&$options) {
    $options['type'] = 'ul';
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('List type'),
      '#options' => array('ul' => t('Unordered list'), 'ol' => t('Ordered list')),
      '#default_value' => $this->options['type'],
    );
  }
}

/**
 * Style plugin to render each item in a grid cell.
 */
class views_plugin_style_grid extends views_plugin_style {
  /**
   * Set default options
   */
  function options(&$options) {
    $options['columns'] = 4;
    $options['alignment'] = 'horizontal';
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    $form['columns'] = array(
      '#type' => 'textfield',
      '#title' => t('Number of columns'),
      '#default_value' => $this->options['columns'],
    );
    $form['alignment'] = array(
      '#type' => 'radios',
      '#title' => t('Alignment'),
      '#options' => array('horizontal' => t('Horizontal'), 'vertical' => t('Vertical')),
      '#default_value' => $this->options['alignment'],
      '#description' => t('Horizontal alignment will place items starting in the upper left and moving right. Vertical alignment will place items starting in the upper left and moving down.'),
    );
  }
}

/**
 * Style plugin to render each item as a row in a table.
 */
class views_plugin_style_table extends views_plugin_style {
  /**
   * Set default options
   */
  function options(&$options) {
    $options['columns'] = array();
    $options['default'] = '';
    $options['info'] = array();
    $options['override'] = TRUE;
    $options['order'] = 'asc';
  }

  /**
   * Determine if we should provide sorting based upon $_GET inputs.
   */
  function build_sort() {
    if (!isset($_GET['order'])) {
      // check for a 'default' clicksort. If there isn't one, exit gracefully.
      if (empty($this->options['default'])) {
        return TRUE;
      }
      $sort = $this->options['default'];
      $this->order = !empty($this->options['order']) ? $this->options['order'] : 'asc';
    }
    else {
      $sort = $_GET['order'];
      // Store the $order for later use.
      $this->order = !empty($_GET['sort']) ? strtolower($_GET['sort']) : 'asc';
    }

    // If a sort we don't know anything about gets through, exit gracefully.
    if (empty($this->view->field[$sort])) {
      return TRUE;
    }

    // Ensure $this->order is valid.
    if ($this->order != 'asc' && $this->order != 'desc') {
      $this->order = 'asc';
    }

    // Store the $sort for later use.
    $this->active = $sort;

    // Tell the field to click sort.
    $this->view->field[$sort]['handler']->click_sort($this->order);

    // Let the builder know whether or not we're overriding the default sorts.
    return empty($this->options['override']);
  }

  /**
   * Normalize a list of columns based upon the fields that are
   * available.
   *
   * - Each field must be in a column.
   * - Each column must be based upon a field, and that field
   *   is somewhere in the column.
   * - Any fields not currently represented must be added.
   * - Columns must be re-ordered to match the fields.
   *
   * @param $columns
   *   An array of all fields; the key is the id of the field and the
   *   value is the id of the column the field should be in.
   * @param $fields
   *   The fields to use for the columns. If not provided, they will
   *   be requested from the current display. The running render should
   *   send the fields through, as they may be different than what the
   *   display has listed due to access control or other changes.
   */
  function sanitize_columns($columns, $fields = NULL) {
    $sanitized = array();
    if ($fields === NULL) {
      $fields = $this->display->handler->get_option('fields');
    }

    // Preconfigure the sanitized array so that the order is retained.
    foreach ($fields as $field => $info) {
      // Set to itself so that if it isn't touched, it gets column
      // status automatically.
      $sanitized[$field] = $field;
    }

    foreach ($columns as $field => $column) {
      // first, make sure the field still exists.
      if (!isset($sanitized[$field])) {
        continue;
      }

      // If the field is the column, mark it so, or the column
      // it's set to is a column, that's ok
      if ($field == $column || $columns[$column] == $column && !empty($sanitized[$column])) {
        $sanitized[$field] = $column;
      }
      // Since we set the field to itself initially, ignoring
      // the condition is ok; the field will get its column
      // status back.
    }

    return $sanitized;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    $fields = $this->display->handler->get_option('fields');
    if (empty($fields)) {
      $form['error_markup'] = array(
        '#value' => t('You need at least one field before you can configure your table settings'),
        '#prefix' => '<div class="error form-item description">',
        '#suffix' => '</div>',
      );
      return;
    }

    $form['override'] = array(
      '#type' => 'checkbox',
      '#title' => t('Override normal sorting if click sorting is used'),
      '#default_value' => !empty($this->options['override']),
    );

    $form['sticky'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable Drupal style "sticky" table headers (Javascript)'),
      '#default_value' => !empty($this->options['sticky']),
      '#description' => t('(Sticky header effects will not be active for preview below, only on live output.)'),
    );

    $form['order'] = array(
      '#type' => 'select',
      '#title' => t('Default sort order'),
      '#options' => array('asc' => t('Ascending'), 'desc' => t('Descending')),
      '#default_value' => $this->options['order'],
      '#description' => t('If a default sort order is selected, what order should it use by default.'),
    );

    // Note: views UI registers this theme handler on our behalf. Your module
    // will have to register your theme handlers if you do stuff like this.
    $form['#theme'] = 'views_ui_style_plugin_table';

    $columns = $this->sanitize_columns($this->options['columns']);

    // Create an array of allowed columns from the data we know:
    $field_names = array();
    $handlers = array();
    foreach ($fields as $field => $info) {
      $handlers[$field] = views_get_handler($info['table'], $info['field'], 'field');
      $handlers[$field]->init($this->view, $info);

      if ($label = $handlers[$field]->label()) {
        $field_names[$field] = $label;
      }
      else {
        $field_names[$field] = t('!group: !title', array('!group' => $handlers[$field]->definition['group'], '!title' => $handlers[$field]->definition['title']));
      }
    }

    if (isset($this->options['default'])) {
      $default = $this->options['default'];
      if (!isset($fields[$default])) {
        $default = -1;
      }
    }
    else {
      $default = -1;
    }

    foreach ($columns as $field => $column) {
      $safe = str_replace(array('][', '_', ' '), '-', $field);
      // the $id of the column for dependency checking.
      $id = 'edit-style-options-columns-' . $safe;

      $form['columns'][$field] = array(
        '#type' => 'select',
        '#options' => $field_names,
        '#default_value' => $column,
      );
      if ($handlers[$field]->click_sortable()) {
        $form['info'][$field]['sortable'] = array(
          '#type' => 'checkbox',
          '#default_value' => !empty($this->options['info'][$field]['sortable']),
          '#process' => array('views_process_dependency'),
          '#dependency' => array($id => array($field)),
        );
        // Provide an ID so we can have such things.
        $radio_id = form_clean_id('edit-default-' . $field);
        $form['default'][$field] = array(
          '#type' => 'radio',
          '#return_value' => $field,
          '#parents' => array('style_options', 'default'),
          '#id' => $radio_id,
          // because 'radio' doesn't fully support '#id' =(
          '#attributes' => array('id' => $radio_id),
          '#default_value' => $default,
          '#process' => array('views_process_dependency'),
          '#dependency' => array($id => array($field)),
        );
      }
      $form['info'][$field]['separator'] = array(
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => isset($this->options['info'][$field]['separator']) ? $this->options['info'][$field]['separator'] : '',
        '#process' => array('views_process_dependency'),
        '#dependency' => array($id => array($field)),
      );

      // markup for the field name
      $form['info'][$field]['name'] = array(
        '#value' => $field_names[$field],
      );
    }

    // Provide a radio for no default sort
    $form['default'][-1] = array(
      '#type' => 'radio',
      '#return_value' => -1,
      '#parents' => array('style_options', 'default'),
      '#id' => 'edit-default-0',
      '#default_value' => $default,
    );

    $form['description_markup'] = array(
      '#prefix' => '<div class="description form-item">',
      '#suffix' => '</div>',
      '#value' => t('Place fields into columns; you may combine multiple fields into the same column. If you do, the separator in the column specified will be used to separate the fields. Check the sortable box to make that column clicksortable, and check the default sort radio to determine which column will be sorted by default, if any. You may control column order and field labels in the fields section.'),
    );
  }

  /**
   * Render the table style.
   */
  function render() {
    return theme($this->theme_functions(), $this->view, $this->options, array());
  }
}

/**
 * Theme the form for the table style plugin
 */
function theme_views_ui_style_plugin_table($form) {
  $output = drupal_render($form['description_markup']);

  $header = array(
    t('Field'),
    t('Column'),
    t('Separator'),
    array(
      'data' => t('Sortable'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default sort'),
      'align' => 'center',
    ),
  );
  $rows = array();
  foreach (element_children($form['columns']) as $id) {
    $row = array();
    $row[] = drupal_render($form['info'][$id]['name']);
    $row[] = drupal_render($form['columns'][$id]);
    $row[] = drupal_render($form['info'][$id]['separator']);
    if (!empty($form['info'][$id]['sortable'])) {
      $row[] = array(
        'data' => drupal_render($form['info'][$id]['sortable']),
        'align' => 'center',
      );
      $row[] = array(
        'data' => drupal_render($form['default'][$id]),
        'align' => 'center',
      );
    }
    else {
      $row[] = '';
      $row[] = '';
    }
    $rows[] = $row;
  }

  // Add the special 'None' row.
  $rows[] = array(t('None'), '', '', '', array('align' => 'center', 'data' => drupal_render($form['default'][-1])));

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

/**
 * The default style plugin for summaries.
 */
class views_plugin_style_summary extends views_plugin_style {
  function options(&$options) {
    $options['count'] = TRUE;
    $options['override'] = FALSE;
    $options['items_per_page'] = 25;
  }

  function query() {
    if (!empty($this->options['override'])) {
      $this->view->set_items_per_page(intval($this->options['items_per_page']));
    }
  }

  function options_form(&$form, &$form_state) {
    $form['count'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['count']),
      '#title' => t('Display record count with link'),
    );
    $form['override'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['override']),
      '#title' => t('Override number of items to display'),
    );
    $form['items_per_page'] = array(
      '#type' => 'textfield',
      '#title' => t('Items to display'),
      '#default_value' => $this->options['items_per_page'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-override' => array(TRUE)),
    );
  }

  function render() {
    $rows = array();
    foreach ($this->view->result as $row) {
      // @todo: Include separator as an option.
      $rows[] = $row;
    }
    return theme($this->theme_functions(), $this->view, $this->options, $rows);
  }
}

/**
 * Default style plugin to render an RSS feed.
 */
class views_plugin_style_rss extends views_plugin_style {
  function attach_to($display_id, $path, $title) {
    $display = $this->view->display[$display_id]->handler;
    $url = url($this->view->get_url(NULL, $path));
    if ($display->has_path()) {
      if (empty($this->preview)) {
        drupal_add_feed($url, $title);
      }
    }
    else {
      if (empty($this->view->feed_icon)) {
        $this->view->feed_icon = '';
      }

      $this->view->feed_icon .= theme('feed_icon', $url, $title);
    }
  }

  function options(&$options) {
    $options['description'] = '';
    $options['mission_description'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    $form['mission_description'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['mission_description']),
      '#title' => t('Use the site mission for the description'),
    );
    $form['description'] = array(
      '#type' => 'textfield',
      '#title' => t('RSS description'),
      '#default_value' => $this->options['description'],
      '#description' => t('This will appear in the RSS feed itself.'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-override' => array(FALSE)),
    );
  }

  function render() {
    if (empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }
    $rows = '';

    // This will be filled in by the row plugin and is used later on in the
    // theming output.
    $this->namespaces = array();

    foreach ($this->view->result as $row) {
      $rows .= $this->row_plugin->render($row);
    }

    return theme($this->theme_functions(), $this->view, $this->options, $rows);
  }
}

/**
 * @}
 */

/**
 * @defgroup views_row_plugins Views' row plugins
 * @{
 *
 * Row plugins control how Views outputs an individual record. They are
 * tightly coupled to style plugins, in that a style plugin is what calls
 * the row plugin.
 *
 * @see hook_views_plugins
 */

/**
 * Default plugin to view a single row of a table. This is really just a wrapper around
 * a theme function.
 */
class views_plugin_row extends views_plugin {
  /**
   * Initialize the row plugin.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;
    $this->options = $display->handler->get_option('row_options');
  }

  function uses_fields() {
    return !empty($this->definition['uses fields']);
  }

  /**
   * Static member function to set default options.
   */
  function options(&$options) { }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) { }

  /**
   * Allow the style to do stuff before each row is rendered.
   *
   * @param $result
   *   The full array of results from the query.
   */
  function pre_render($result) { }

  /**
   * Render a row object. This usually passes through to a theme template
   * of some form, but not always.
   */
  function render($row) {
    return theme($this->theme_functions(), $this->view, $this->options, $row);
  }
}

/**
 * The basic 'fields' row plugin
 *
 * This displays fields one after another, giving options for inline
 * or not.
 */
class views_plugin_row_fields extends views_plugin_row {
  function options(&$options) {
    $options['inline'] = array();
    $options['separator'] = '';
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) {
    $fields = $this->display->handler->get_option('fields');
    $options = array();
    foreach ($fields as $field => $info) {
      $handler = views_get_handler($info['table'], $info['field'], 'field');
      if ($handler) {
        $options[$field] = $handler->ui_name();
      }
    }

    if (empty($this->options['inline'])) {
      $this->options['inline'] = array();
    }

    $form['inline'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Inline fields'),
      '#options' => $options,
      '#default_value' => $this->options['inline'],
      '#description' => t('Inline fields will be displayed next to each other rather than one after another.'),
    );

    $form['separator'] = array(
      '#title' => t('Separator'),
      '#type' => 'textfield',
      '#size' => 10,
      '#default_value' => isset($this->options['separator']) ? $this->options['separator'] : '',
      '#description' => t('The separator may be placed between inline fields to keep them from squishing up next to each other. You can use HTML in this field.'),
    );
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    $form_state['values']['row_options']['inline'] = array_filter($form_state['values']['row_options']['inline']);
  }
}

/**
 * @}
 */

/**
 * @defgroup views_argument_default_plugins Views' argument default plugins
 * @{
 *
 * Allow specialized methods of filling in arguments when they aren't
 * provided.
 *
 * @see hook_views_plugins
 */

/**
 */
class views_plugin_argument_default extends views_plugin {
  var $option_name = 'default_argument_fixed';
  /**
   * Initialize this plugin with the view and the argument
   * it is linked to.
   */
  function init(&$view, &$argument, $id = NULL) {
    $this->view = &$view;
    $this->argument = &$argument;
    $this->id = $id;
  }

  /**
   * Determine if the administrator has the privileges to use this
   * plugin
   */
  function access() { return TRUE; }

  function argument_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'textfield',
      '#title' => t('Default argument'),
      '#default_value' => $this->get_argument(),
      '#process' => array('views_process_dependency'),
      '#dependency' => array(
        'radio:options[default_action]' => array('default'),
        'radio:options[default_argument_type]' => array($this->id)
      ),
      '#dependency_count' => 2,
    );

    // Only do this if using one simple standard form gadget
    $this->check_access($form);
  }

  /**
   * If we don't have access to the form but are showing it anyway, ensure that
   * the form is safe and cannot be changed from user input.
   */
  function check_access(&$form) {
    if (!$this->access()) {
      $form[$this->option_name]['#disabled'] = TRUE;
      $form[$this->option_name]['#value'] = $form[$this->option_name]['#default_value'];
    }
  }

  /**
   * Return the default argument.
   */
  function get_argument() {
    return isset($this->argument->options[$this->option_name]) ? $this->argument->options[$this->option_name] : '';
  }
}

/**
 * Default argument plugin to provide a PHP code block.
 */
class views_plugin_argument_default_php extends views_plugin_argument_default {
  var $option_name = 'default_argument_php';

  function argument_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'textarea',
      '#title' => t('PHP argument code'),
      '#default_value' => $this->get_argument(TRUE), // the true forces it raw.
      '#process' => array('views_process_dependency'),
      '#description' => t('Enter PHP code that returns a value to use for this argument. Do not use &lt;?php ?&gt;.'),
      '#dependency' => array(
        'radio:options[default_action]' => array('default'),
        'radio:options[default_argument_type]' => array($this->id)
      ),
      '#dependency_count' => 2,
    );

    $this->check_access($form);
  }

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * default plugin.
   */
  function access() {
    return user_access('use PHP for block visibility');
  }

  function get_argument($raw = FALSE) {
    if ($raw) {
      return parent::get_argument();
    }

    // set up variables to make it easier to reference during the argument.
    $view = &$this->view;
    $argument = &$this->argument;
    ob_start();
    $result = eval($this->argument->options[$this->option_name]);
    ob_end_clean();
    return $result;
  }
}

/**
 * @}
 */

/**
 * @defgroup views_argument_validate_plugins Views' argument validate plugins
 * @{
 *
 * Allow specialized methods of validating arguments.
 *
 * @see hook_views_plugins
 */

/**
 * Base argument validator plugin to provide basic functionality.
 */
class views_plugin_argument_validate extends views_plugin {
  var $option_name = 'validate_argument';

  /**
   * Initialize this plugin with the view and the argument
   * it is linked to.
   */
  function init(&$view, &$argument, $id = NULL) {
    $this->view = &$view;
    $this->argument = &$argument;
    $this->id = $id;
  }

  /**
   * Determine if the administrator has the privileges to use this
   * plugin
   */
  function access() { return TRUE; }

  function argument_form(&$form, &$form_state) {
  }

  /**
   * If we don't have access to the form but are showing it anyway, ensure that
   * the form is safe and cannot be changed from user input.
   */
  function check_access(&$form) {
    if (!$this->access()) {
      $form[$this->option_name]['#disabled'] = TRUE;
      $form[$this->option_name]['#value'] = $form[$this->option_name]['#validate_value'];
    }
  }

  /**
   * Return the validate argument.
   */
  function get_argument() {
    return isset($this->argument->options[$this->option_name]) ? $this->argument->options[$this->option_name] : '';
  }

  function validate_form(&$form, &$form_state) { }

  function validate_argument($arg) { return TRUE; }
}

/**
 * Provide PHP code to validate whether or not an argument is ok.
 */
class views_plugin_argument_validate_php extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_php';

  function validate_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'textarea',
      '#title' => t('PHP validate code'),
      '#default_value' => $this->get_argument(),
      '#description' => t('Enter PHP code that returns TRUE or FALSE. No return is the same as FALSE, so be SURE to return something if you do not want to declare the argument invalid. Do not use &lt;?php ?&gt;. The argument to validate will be "$argument" and the view will be "$view". You may change the argument by setting "$handler->argument".'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $this->check_access($form);
  }

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * validate plugin.
   */
  function access() {
    return user_access('use PHP for block visibility');
  }

  function validate_argument($argument) {
    // set up variables to make it easier to reference during the argument.
    $view = &$this->view;
    $handler = &$this->argument;

    ob_start();
    $result = eval($this->argument->options[$this->option_name]);
    ob_end_clean();
    return $result;
  }
}

/**
 * Provide PHP code to validate whether or not an argument is ok.
 */
class views_plugin_argument_validate_numeric extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_numeric';

  /**
   * Only let users with PHP block visibility permissions set/modify this
   * validate plugin.
   */
  function access() {
    return !empty($this->argument->definition['numeric']);
  }

  function validate_argument($argument) {
    return is_numeric($argument);
  }
}

/**
 * @}
 */
