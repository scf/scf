<?php
// $Id: views_handler_filter_boolean_operator.inc,v 1.2 2008/09/10 01:08:06 merlinofchaos Exp $
/**
 * Simple filter to handle matching of boolean values
 *
 * Definition items:
 * - label: (REQUIRED) The label for the checkbox.
 */
class views_handler_filter_boolean_operator extends views_handler_filter {
  // exposed filter options
  var $no_single = TRUE;
  // Don't display empty space where the operator would be.
  var $no_operator = TRUE;

  function construct() {
    $this->value_value = t('True');
    if (isset($this->definition['label'])) {
      $this->value_value = $this->definition['label'];
    }
    parent::construct();
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['value']['default'] = FALSE;

    return $options;
  }

  function operator_form(&$form, &$form_state) {
    $form['operator'] = array();
  }

  function value_form(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      $form['value'] = array(
        '#type' => 'checkbox',
        '#title' => $this->value_value,
        '#default_value' => $this->value,
      );
    }
    else {
      $form['value'] = array(
        '#type' => 'select',
        '#title' => $this->value_value,
        '#options' => array(1 => t('Yes'), 0 => t('No')),
        '#default_value' => $this->value,
      );
      $identifier = $this->options['expose']['identifier'];
      if (!isset($form_state['input'][$identifier])) {
        $form_state['input'][$identifier] = $this->value;
      }
    }
  }
  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    return (empty($this->value) ? t("False") : t('True'));
  }

  function expose_options() {
    parent::expose_options();
    $this->options['expose']['operator'] = '';
    $this->options['expose']['label'] = $this->value_value;
    $this->options['expose']['optional'] = FALSE;
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . (empty($this->value) ? '=' : '<>') . " 0");
  }
}
