<?php
// $Id: views_handler_field_user_name.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * Field handler to provide simple renderer that allows using a themed user link
 */
class views_handler_field_user_name extends views_handler_field_user {
  function render_link($data, $values) {
    if (!empty($this->options['link_to_user'])) {
      $account = new stdClass();
      $account->name = $values->{$this->field_alias};
      $account->uid = $values->{$this->aliases['uid']};
      return theme('username', $account);
    }
    else {
      return $data;
    }
  }
}

