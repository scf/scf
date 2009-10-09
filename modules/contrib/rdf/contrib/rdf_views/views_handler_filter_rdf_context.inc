<?php
// $Id: views_handler_filter_rdf_context.inc,v 1.3 2009/02/19 17:19:11 jmiccolis Exp $

class views_handler_filter_rdf_context extends views_handler_filter_in_operator {
  function get_value_options() {
    $options = array();
    foreach (rdf_db_rdf_contexts() as $g) {
      if ($label = rdf_query($g, 'rdfs:label', NULL)->to_array()) {
        $label = array_pop($label);
        $options[$g] = $label[2];
      }
      else {
        $options[$g] = $g;
      }
    }
    $this->value_options = $options;
  }
}
