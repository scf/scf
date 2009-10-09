<?php
// $Id: views_handler_field_rdf_literal.inc,v 1.2 2009/02/16 13:27:10 arto Exp $

class views_handler_field_rdf_literal extends views_handler_field {
  function render($values) {
    $value = $values->{$this->field_alias};
    return filter_xss($value);
  }
}
