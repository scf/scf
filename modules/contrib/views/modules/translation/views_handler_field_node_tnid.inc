<?php
// $Id: views_handler_field_node_tnid.inc,v 1.1 2008/09/30 22:07:15 merlinofchaos Exp $

/**
 * Field handler to translate a language into its readable form.
 */
class views_handler_field_node_tnid extends views_handler_field_node {
  function render($values) {
    global $language;
    $languages = locale_language_list();
    if ($values->nid != $this->field_alias) {
    }
    $value = $languages[$values->{$this->field_alias}];
    return $this->render_link($value, $values);
  }
}
