<?php
// $Id: views_handler_filter_history_user_timestamp.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * Filter for new content
 */
class views_handler_filter_history_user_timestamp extends views_handler_filter {
  function can_expose() { return FALSE; }
  function query() {
    global $user;
    // This can only work if we're logged in.
    if (!$user || !$user->uid) {
      return;
    }

    // Hey, Drupal kills old history, so nodes that haven't been updated
    // since NODE_NEW_LIMIT are bzzzzzzzt outta here!

    $limit = time() - NODE_NEW_LIMIT;

    $this->ensure_my_table();
    $field = "$this->table_alias.$this->real_field";
    $node = $this->query->ensure_table('node');

    if (module_exists('comment')) {
      $ncs = $this->query->ensure_table('node_comment_statistics');
      $clause = ("OR $ncs.last_comment_timestamp > (***CURRENT_TIME*** - $limit)");
      $clause2 = "OR $field < $ncs.last_comment_timestamp";
    }

    // NULL means a history record doesn't exist. That's clearly new content.
    // Unless it's very very old content. Everything in the query is already
    // type safe cause none of it is coming from outside here.
    $this->query->add_where($this->options['group'], "($field IS NULL AND ($node.changed > (***CURRENT_TIME*** - $limit) $clause)) OR $field < $node.changed $clause2");
  }

  /**
   * Blank this, because there is nothing to say about this one.
   */
  function admin_summary() { }
}

