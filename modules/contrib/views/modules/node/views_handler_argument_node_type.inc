<?php
// $Id: views_handler_argument_node_type.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * Argument handler to accept a node type.
 */
class views_handler_argument_node_type extends views_handler_argument {
  function construct() {
    parent::construct('type');
  }

  /**
   * Override the behavior of summary_name(). Get the user friendly version
   * of the node type.
   */
  function summary_name($data) {
    return $this->node_type($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title(). Get the user friendly version of the
   * node type.
   */
  function title() {
    return $this->node_type($this->argument);
  }

  function node_type($type) {
    $output = node_get_types('name', $type);
    if (empty($output)) {
      $output = t('Unknown node type');
    }
    return check_plain($output);
  }
}
