<?php
// $Id: views_handler_filter_many_to_one.inc,v 1.1 2008/09/03 19:21:28 merlinofchaos Exp $

/**
 * Complex filter to handle filtering for many to one relationships,
 * such as terms (many terms per node) or roles (many roles per user).
 *
 * The construct method needs to be overridden to provide a list of options;
 * alternately, the value_form and admin_summary methods need to be overriden
 * to provide something that isn't just a select list.
 */
class views_handler_filter_many_to_one extends views_handler_filter_in_operator {
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->helper = new views_many_to_one_helper($this);
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'or';
    $options['value']['default'] = array();

    return $options;
  }

  /**
   * Provide inclusive/exclusive matching
   */
  function operator_options() {
    return array(
      'or' => t('Is one of'),
      'and' => t('Is all of'),
      'not' => t('Is none of'),
    );
  }

  var $value_form_type = 'select';
  function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    if (empty($form_state['exposed'])) {
      $this->helper->options_form($form, $form_state);
    }
  }

  /**
   * Override ensure_my_table so we can control how this joins in.
   * The operator actually has influence over joining.
   */
  function ensure_my_table() {
    $this->helper->ensure_my_table();
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->helper->add_filter();
  }
}
