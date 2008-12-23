<?php
// $Id: views_handler_field_comment.inc,v 1.3 2008/11/19 22:44:29 merlinofchaos Exp $
/**
 * Field handler to allow linking to a comment
 */
class views_handler_field_comment extends views_handler_field {
  /**
   * Override init function to provide generic option to link to comment.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($this->options['link_to_comment'])) {
      $this->additional_fields['cid'] = 'cid';
      $this->additional_fields['nid'] = 'nid';
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_comment'] = array('default' => TRUE);
    return $options;
  }

  /**
   * Provide link-to-comment option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_comment'] = array(
      '#title' => t('Link this field to its comment'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_comment'],
    );
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_comment']) && $data !== NULL && $data !== '') {
      return l($data, "node/" . $values->{$this->aliases['nid']}, array('html' => TRUE, 'fragment' => "comment-" . $values->{$this->aliases['cid']}));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}
