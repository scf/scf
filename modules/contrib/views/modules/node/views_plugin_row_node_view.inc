<?php
// $Id: views_plugin_row_node_view.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * @file
 * Contains the node view row style plugin.
 */

/**
 * Plugin which performs a node_view on the resulting object.
 *
 * Most of the code on this object is in the theme function.
 */
class views_plugin_row_node_view extends views_plugin_row {
  function option_definition() {
    $options = parent::option_definition();

    $options['teaser'] = array('default' => TRUE);
    $options['links'] = array('default' => TRUE);
    $options['comments'] = array('default' => FALSE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['teaser'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display only teaser'),
      '#default_value' => $this->options['teaser'],
    );
    $form['links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    );
    $form['comments'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display node comments'),
      '#default_value' => $this->options['comments'],
    );
  }
}
