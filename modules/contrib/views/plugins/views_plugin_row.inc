<?php
// $Id: views_plugin_row.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * @file
 * Contains the base row style plugin.
 */

/**
 * Default plugin to view a single row of a table. This is really just a wrapper around
 * a theme function.
 *
 * @ingroup views_row_plugins
 */
class views_plugin_row extends views_plugin {
  /**
   * Initialize the row plugin.
   */
  function init(&$view, &$display, $options = NULL) {
    $this->view = &$view;
    $this->display = &$display;

    // Overlay incoming options on top of defaults
    $this->unpack_options($this->options, isset($options) ? $options : $display->handler->get_option('row_options'));
  }

  function uses_fields() {
    return !empty($this->definition['uses fields']);
  }

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
