<?php
// $Id: views_handler_filter_node_status.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * Filter by published status
 */
class views_handler_filter_node_status extends views_handler_filter {
  function admin_summary() { }
  function operator_form() { }

  function query() {
    $table = $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$table.status <> 0 OR $table.uid = ***CURRENT_USER*** or ***ADMINISTER_NODES*** = 1");
  }
}
