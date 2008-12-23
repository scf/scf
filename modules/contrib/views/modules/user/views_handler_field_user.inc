<?php
// $Id: views_handler_field_user.inc,v 1.2 2008/11/19 22:44:30 merlinofchaos Exp $

/**
 * Field handler to provide simple renderer that allows linking to a user.
 */
class views_handler_field_user extends views_handler_field {
  /**
   * Override init function to provide generic option to link to user.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (!empty($this->options['link_to_user'])) {
      $this->additional_fields['uid'] = 'uid';
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_user'] = array('default' => TRUE);
    return $options;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_user'] = array(
      '#title' => t('Link this field to its user'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_user'],
    );
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_user']) && user_access('access user profiles') && $values->{$this->aliases['uid']} && $data !== NULL && $data !== '') {
      return l($data, "user/" . $values->{$this->aliases['uid']}, array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}
