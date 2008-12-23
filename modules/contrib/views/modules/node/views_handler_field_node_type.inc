<?php
// $Id: views_handler_field_node_type.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $

/**
 * Field handler to translate a node type into its readable form.
 */
class views_handler_field_node_type extends views_handler_field_node {
  function render($values) {
    $value = node_get_types('name', $values->{$this->field_alias});
    return $this->render_link(check_plain($value), $values);
  }
}

