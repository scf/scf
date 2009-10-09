<?php
// $Id: biblio_handler_filter_biblio_contributor_auth_type.inc,v 1.1.2.1 2009/06/16 21:20:51 rjerome Exp $
class biblio_handler_filter_biblio_contributor_auth_type extends views_handler_filter_in_operator {

  function construct() {
    parent::construct();
    $this->definition['numeric'] = true;
  }

  function get_value_options() {
    if (!isset($this->value_options)) {
      $result = db_query("SELECT title, auth_type
              FROM {biblio_contributor_type_data}
              ORDER by auth_type");
      $this->value_title = t('Author type');
      $options = array();
      while ($row = db_fetch_object($result)) {
        $options[$row->auth_type] =  $row->title;
      }
      $this->value_options = $options;
    }
  }
}
