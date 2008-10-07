<?php
// $Id: comment.views.inc,v 1.15 2008/05/27 21:57:06 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers for comment.module
 */

/**
 * @defgroup views_comment_module comment.module handlers
 *
 * Includes the tables 'comments' and 'node_comment_statistics'
 * @{
 */

/**
 * Implementation of hook_views_data()
 */

function comment_views_data() {
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['comments']['table']['group']  = t('Comment');

  $data['comments']['table']['base'] = array(
    'field' => 'cid',
    'title' => t('Comment'),
    'help' => t("Comments are responses to node content."),
  );

  //joins
  $data['comments']['table']['join'] = array(
    //...to the node table
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
  );

  // ----------------------------------------------------------------
  // Fields

  // subject
  $data['comments']['subject'] = array(
    'title' => t('Title'),
    'help' => t('The title of the comment.'),
    'field' => array(
      'handler' => 'views_handler_field_comment',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // comment (the comment body)
  $data['comments']['comment'] = array(
    'title' => t('Body'),
    'help' => t('The text of the comment.'),
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // cid
  $data['comments']['cid'] = array(
    'title' => t('ID'),
    'help' => t('The commment ID of the field'),
    'field' => array(
      'handler' => 'views_handler_field_comment',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
    ),
  );

  // name (of comment author)
  $data['comments']['name'] = array(
    'title' => t('Author'),
    'help' => t('The name of the poster.'),
    'field' => array(
      'handler' => 'views_handler_field_username_comment',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // homepage
  $data['comments']['homepage'] = array(
    'title' => t("Author's website"),
    'help' => t("The website address of the comment's author. Can be a link. The homepage can also be linked with the Name field. Will be empty if posted by a registered user."),
    'field' => array(
      'handler' => 'views_handler_field_url',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // timestamp (when comment was posted)
  $data['comments']['timestamp'] = array(
    'title' => t('Post date'),
    'help' => t('Date and time of when the comment was posted.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  // status (approved or not)
  $data['comments']['status'] = array(
    'title' => t('In moderation'),
    'help' => t('Whether or not the comment is currently in moderation.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Moderated'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // link to view comment
  $data['comments']['view_comment'] = array(
    'field' => array(
      'title' => t('View link'),
      'help' => t('Provide a simple link to view the comment.'),
      'handler' => 'views_handler_field_comment_link',
    ),
  );

  // link to edit comment
  $data['comments']['edit_comment'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the comment.'),
      'handler' => 'views_handler_field_comment_link_edit',
    ),
  );

  // link to delete comment
  $data['comments']['delete_comment'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the comment.'),
      'handler' => 'views_handler_field_comment_link_delete',
    ),
  );

  // link to reply to comment
  $data['comments']['replyto_comment'] = array(
    'field' => array(
      'title' => t('Reply-to link'),
      'help' => t('Provide a simple link to reply to the comment.'),
      'handler' => 'views_handler_field_comment_link_reply',
    ),
  );

  $data['comments']['thread'] = array(
    'field' => array(
      'title' => t('Depth'),
      'help' => t('Display the depth of the comment if it is threaded.'),
      'handler' => 'views_handler_field_comment_depth',
    ),
    'sort' => array(
      'title' => t('Thread'),
      'help' => t('Sort by the threaded order. This will keep child comments together with their parents.'),
      'handler' => 'views_handler_sort_comment_thread',
    ),
  );

  $data['comments']['nid'] = array(
    'title' => t('Node ID'),
    'help' => t('The node the comment is a reply to.'),
    'relationship' => array(
      'base' => 'node',
      'field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Node'),
    ),
  );

  $data['comments']['uid'] = array(
    'title' => t('User ID'),
    'help' => t('The user who wrote the comment.'),
    'relationship' => array(
      'base' => 'users',
      'field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('User'),
    ),
  );

  $data['comments']['pid'] = array(
    'title' => t('Parent CID'),
    'help' => t('The Comment ID of the parent comment.'),
    'field' => array(
      'handler' => 'views_handler_field',
    ),
    'relationship' => array(
      'base' => 'comment',
      'field' => 'cid',
      'handler' => 'views_handler_relationship',
      'label' => t('Parent comment'),
    ),
  );

  // ----------------------------------------------------------------------
  // node_comment_statistics table

  // define the group
  $data['node_comment_statistics']['table']['group']  = t('Node');

  // joins
  $data['node_comment_statistics']['table']['join'] = array(
    //...to the node table
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
     ),
  );

  // last_comment_timestamp
  $data['node_comment_statistics']['last_comment_timestamp'] = array(
    'title' => t('Last comment time'),
    'help' => t('Date and time of when the last comment was posted.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  // last_comment_name (author's name)
  $data['node_comment_statistics']['last_comment_name'] = array(
    'title' => t("Last comment author"),
    'help' => t('The name of the author of the last posted comment.'),
    'field' => array(
      'handler' => 'views_handler_field_ncs_last_comment_name',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_ncs_last_comment_name',
    ),
  );

  // comment_count
  $data['node_comment_statistics']['comment_count'] = array(
    'title' => t('Comment count'),
    'help' => t('The number of comments a node has.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
    ),
  );

  // last_comment_timestamp
  $data['node_comment_statistics']['last_updated'] = array(
    'title' => t('Updated/commented date'),
    'help' => t('The most recent of last comment posted or node updated time.'),
    'field' => array(
      'handler' => 'views_handler_field_ncs_last_updated',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_ncs_last_updated',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_ncs_last_updated',
    ),
  );

  return $data;
}

/**
 * Use views_data_alter to add items to the node table that are
 * relevant to comments.
 */
function comment_views_data_alter(&$data) {
  // new comments
  $data['node']['new_comments'] = array(
    'title' => t('New comments'),
    'help' => t('The number of new comments on the node.'),
    'field' => array(
      'handler' => 'views_handler_field_node_new_comments',
    ),
  );

  // Comment status of the node
  $data['node']['comment'] = array(
    'title' => t('Comment status'),
    'help' => t('Whether comments are enabled or disabled on the node.'),
    'field' => array(
      'handler' => 'views_handler_field_node_comment',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_node_comment',
    ),
  );

  $data['node']['uid_touch'] = array(
    'title' => t('User posted or commented'),
    'help' => t('Display comments only if a user posted the node or commented on the node.'),
    'argument' => array(
      'field' => 'uid',
      'name table' => 'users',
      'name field' => 'name',
      'handler' => 'views_handler_argument_comment_user_uid',
    ),
  );

}

/**
 * Implementation of hook_views_plugins
 */
function comment_views_plugins() {
  return array(
    'module' => 'views', // This just tells views our themes are in Views.
    'row' => array(
      'comment' => array(
        'title' => t('Comment'),
        'help' => t('Display the comment with standard comment view.'),
        'handler' => 'views_plugin_row_comment_view',
        'theme' => 'views_view_row_comment',
        'base' => array('comments'), // only works with 'comment' as base.
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'comment_rss' => array(
        'title' => t('Comment'),
        'help' => t('Display the comment as RSS.'),
        'handler' => 'views_plugin_row_comment_rss',
        'base' => array('comments'), // only works with 'comment' as base.
        'type' => 'feed',
      ),
    ),
  );
}

/**
 * Sort handler for ordering by thread
 */
class views_handler_sort_comment_thread extends views_handler_sort {
  function query() {
    $this->ensure_my_table();

    //Read comment_render() in comment.module for an explanation of the
    //thinking behind this sort.
    if ($this->options['order'] == 'DESC') {
      $this->query->add_orderby($this->table_alias, $this->real_field, $this->options['order']);
    }
    else {
      $alias = $this->table_alias . '_' . $this->real_field . 'asc';
      //@todo is this secure?
      $this->query->add_orderby(NULL, "SUBSTRING({$this->table_alias}.{$this->real_field}, 1, (LENGTH({$this->table_alias}.{$this->real_field}) - 1))", $this->options['order'], $alias);
    }
  }
}

/**
 * Field handler to display the depth of a comment
 */
class views_handler_field_comment_depth extends views_handler_field {
  /**
   * Work out the depth of this comment
   */
  function render($values) {
    return count(explode('.', $values->comments_thread)) - 1;
  }
}

/**
 * Field handler to allow linking to a comment
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment extends views_handler_field {
  /**
   * Override init function to provide generic option to link to comment.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($options['link_to_comment'])) {
      $this->additional_fields['cid'] = 'cid';
      $this->additional_fields['nid'] = 'nid';
    }
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_comment'] = TRUE;
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
    if (!empty($this->options['link_to_comment'])) {
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

/**
 * Field handler to allow linking to a user account or homepage
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_username_comment extends views_handler_field {
  /**
   * Override init function to add uid and homepage fields.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['homepage'] = 'homepage';
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = TRUE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_user'] = array(
      '#title' => t("Link this field to its user or an author's homepage"),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_user'],
    );
  }

  function render_link($data, $values) {
    $account->uid = $values->{$this->aliases['uid']};
    $account->name = $values->{$this->field_alias};
    $account->homepage = $values->{$this->aliases['homepage']};

    if (!empty($this->options['link_to_user'])) {
      return theme('username', $account);
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }

}

/**
 * Base field handler to present a link.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['cid'] = 'cid';
    $this->additional_fields['nid'] = 'nid';
  }

  function options(&$options) {
    parent::options($options);
    $options['text'] = '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
 }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    return l($text, "node/" . $values->{$this->aliases['nid']}, array('html' => TRUE, 'fragment' => "comment-" . $values->{$this->aliases['cid']}));
  }
}

/**
 * Field handler to present a link node edit.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_edit extends views_handler_field_comment_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
  }

  function render($values) {
    // ensure user has access to edit this comment.
    $comment = new stdClass();
    $comment->cid = $values->{$this->aliases['cid']};
    $comment->uid = $values->{$this->aliases['uid']};
    if (!comment_access('edit', $comment)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    return l($text, "comment/edit/" . $values->{$this->aliases['cid']});
  }
}

/**
 * Field handler to present a link to delete a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_delete extends views_handler_field_comment_link {
  function render($values) {
    //needs permission to administer comments in general
    if (!user_access('administer comments')) {
      return;
    }
    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return l($text, "comment/delete/" . $values->{$this->aliases['cid']}, array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to present a link to delete a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_reply extends views_handler_field_comment_link {
  function render($values) {
    //check for permission to reply to comments
    if (!user_access('post comments')) {
      return;
    }
    $text = !empty($this->options['text']) ? $this->options['text'] : t('reply');
    return l($text, "comment/reply/" . $values->{$this->aliases['nid']} . '/' . $values->{$this->aliases['cid']});
  }
}

/**
 * Field handler to present the name of the last comment poster
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_ncs_last_comment_name extends views_handler_field {
  function query() {
    // last_comment_name only contains data if the user is anonymous. So we
    // have to join in a specially related user table.
    $this->ensure_my_table();
    // join 'users' to this table via vid
    $join = new views_join();
    $join->construct('users', $this->table_alias, 'last_comment_uid', 'uid');
    $join->extra = array(array('field' => 'uid', 'operator' => '!=', 'value' => '0'));

    // ncs_user alias so this can work with the sort handler, below.
//    $this->user_table = $this->query->add_relationship(NULL, $join, 'users', $this->relationship);
    $this->user_table = $this->query->ensure_table('ncs_users', $this->relationship, $join);

    $this->field_alias = $this->query->add_field(NULL, "COALESCE($this->user_table.name, $this->table_alias.$this->field)", $this->table_alias . '_' . $this->field);

    $this->user_field = $this->query->add_field($this->user_table, 'name');
    $this->uid = $this->query->add_field($this->table_alias, 'last_comment_uid');
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = TRUE;
  }

  function render($values) {
    if (!empty($this->options['link_to_user'])) {
      $account = new stdClass();
      $account->name = $values->{$this->field_alias};
      $account->uid = $values->{$this->uid};
      return theme('username', $account);
    }
    else {
      return check_plain($values->{$this->field_alias});
    }
  }
}

/**
 * Sort handler to sort by last comment name which might be in 2 different
 * fields
 *
 * @ingroup views_sort_handlers
 */
class views_handler_sort_ncs_last_comment_name extends views_handler_sort {
  function query() {
    $this->ensure_my_table();
    $join = new views_join();
    $join->construct('users', $this->table_alias, 'last_comment_uid', 'uid');

    // @todo this might be safer if we had an ensure_relationship rather than guessing
    // the table alias. Though if we did that we'd be guessing the relationship name
    // so that doesn't matter that much.
//    $this->user_table = $this->query->add_relationship(NULL, $join, 'users', $this->relationship);
    $this->user_table = $this->query->ensure_table('ncs_users', $this->relationship, $join);
    $this->user_field = $this->query->add_field($this->user_table, 'name');

    // Add the field.
    $this->query->add_orderby(NULL, "LOWER(COALESCE($this->user_table.name, $this->table_alias.$this->field))", $this->options['order'], $this->table_alias . '_' . $this->field);
  }
}

/**
 * Field handler to display the newer of last comment / node updated
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_ncs_last_updated extends views_handler_field_date {
  function query() {
    $this->ensure_my_table();
    $this->node_table = $this->query->ensure_table('node', $this->relationship);
    $this->field_alias = $this->query->add_field(NULL, "GREATEST(" . $this->node_table . ".changed, " . $this->table_alias . ".last_comment_timestamp)", $this->table_alias . '_' . $this->field);
  }
}

/**
 * Sort handler for the newer of last comment / node updated
 *
 * @ingroup views_sort_handlers
 */
class views_handler_sort_ncs_last_updated extends views_handler_sort_date {
  function query() {
    $this->ensure_my_table();
    $this->node_table = $this->query->ensure_table('node', $this->relationship);
    $this->field_alias = $this->query->add_orderby(NULL, "GREATEST(" . $this->node_table . ".changed, " . $this->table_alias . ".last_comment_timestamp)", $this->options['order'], $this->table_alias . '_' . $this->field);
  }
}

/**
 * Filter handler for the newer of last comment / node updated
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_ncs_last_updated extends views_handler_filter_date {
  function query() {
    $this->ensure_my_table();
    $this->node_table = $this->query->ensure_table('node', $this->relationship);

    $field = "GREATEST(" . $this->node_table . ".changed, " . $this->table_alias . ".last_comment_timestamp)";

    $info = $this->operators();
    if (!empty($info[$this->operator]['method'])) {
      $this->{$info[$this->operator]['method']}($field);
    }
  }
}

/**
 * Field handler to display the number of new comments
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_new_comments extends views_handler_field_numeric {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = 'nid';
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_comment'] = TRUE;
    $options['no_empty'] = TRUE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_comment'] = array(
      '#title' => t('Link this field to new comments'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_comment'],
    );
    $form['no_empty'] = array(
      '#title' => t('Display nothing if no new comments'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['no_empty'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
    $this->field_alias = $this->table . '_' . $this->field;
  }

  function pre_render(&$values) {
    global $user;
    if (!$user->uid || empty($values)) {
      return;
    }

    $nids = array();
    $ids = array();
    foreach ($values as $id => $result) {
      $nids[] = $result->{$this->aliases['nid']};
      $values[$id]->{$this->field_alias} = 0;
      // Create a reference so we can find this record in the values again.
      if (empty($ids[$result->{$this->aliases['nid']}])) {
        $ids[$result->{$this->aliases['nid']}] = array();
      }
      $ids[$result->{$this->aliases['nid']}][] = $id;
    }

    if ($nids) {
      $result = db_query("SELECT n.nid, COUNT(c.cid) as num_comments FROM {node} n INNER JOIN {comments} c ON n.nid = c.nid LEFT JOIN {history} h ON h.nid = n.nid AND h.uid = %d WHERE n.nid IN (" . implode(', ', $nids) . ") AND c.timestamp > GREATEST(COALESCE(h.timestamp, %d), %d) AND c.status = %d GROUP BY n.nid  ", $user->uid, NODE_NEW_LIMIT, NODE_NEW_LIMIT, COMMENT_PUBLISHED);

      while ($node = db_fetch_object($result)) {
        foreach ($ids[$node->nid] as $id) {
          $values[$id]->{$this->field_alias} = $node->num_comments;
        }
      }

    }
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_comment'])) {
      return l($data, "node/" . $values->{$this->aliases['nid']}, array('html' => TRUE, 'fragment' => "new"));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    if ($values->{$this->field_alias} || empty($this->options['no_empty'])) {
      return $this->render_link(parent::render($values), $values);
    }
  }
}

/**
 * Argument handler to accept a user id to check for nodes that
 * user posted or commented on.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_comment_user_uid extends views_handler_argument {
  function title() {
    if (!$this->argument) {
      $title = variable_get('anonymous', t('Anonymous'));
    }
    else {
      $title = db_result(db_query("SELECT u.name FROM {users} u WHERE u.uid = %d", $this->argument));
    }
    if (empty($title)) {
      return t('No user');
    }

    return check_plain($title);
  }

  function default_actions($which = NULL) {
    // Disallow summary views on this argument.
    if (!$which) {
      $actions = parent::default_actions();
      unset($actions['summary asc']);
      unset($actions['summary desc']);
      return $actions;
    }

    if ($which != 'summary asc' && $which != 'summary desc') {
      return parent::default_actions($which);
    }
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where(0, "$this->table_alias.uid = %d OR (SELECT COUNT(*) FROM {comments} c WHERE c.uid = %d AND c.nid = $this->table_alias.nid)", $this->argument, $this->argument);
  }
}

/**
 * Display node comment status
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_comment extends views_handler_field {
  function render($values) {
    switch ($values->{$this->field_alias}) {
      case COMMENT_NODE_DISABLED:
      default:
        return t('Disabled');
      case COMMENT_NODE_READ_ONLY:
        return t('Read only');
      case COMMENT_NODE_READ_WRITE:
        return t('Read/Write');
    }
  }
}

/**
 * Filter based on comment node status
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_node_comment extends views_handler_filter_in_operator {
  function get_value_options() {
    $this->value_options = array(
      COMMENT_NODE_DISABLED => t('Disabled'),
      COMMENT_NODE_READ_ONLY => t('Read only'),
      COMMENT_NODE_READ_WRITE => t('Read/Write'),
    );
  }
}

/**
 * Plugin which performs a comment_view on the resulting object.
 *
 * @ingroup views_plugin_rows
 */
class views_plugin_row_comment_view extends views_plugin_row {
  function options(&$options) {
    $options['links'] = TRUE;
  }

  function options_form(&$form, &$form_state) {
    $form['links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    );
  }

  function pre_render($result) {
    $cids = array();
    $this->comments = array();

    foreach ($result as $row) {
      $cids[] = $row->cid;
    }

    if (count($cids) > 1) {
      $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($cids), '%d')) . ")";
    }
    else {
      $placeholder = " = %d";
    }

    $cresult = db_query("SELECT c.cid as cid, c.pid, c.nid, c.subject, c.comment, c.format, c.timestamp, c.name, c.mail, c.homepage, u.uid, u.name AS registered_name, u.signature, u.picture, u.data, c.thread, c.status FROM {comments} c INNER JOIN {users} u ON c.uid = u.uid WHERE c.cid $placeholder", $cids);
    while ($comment = db_fetch_object($cresult)) {
      $comment = drupal_unpack($comment);
      $comment->name = $comment->uid ? $comment->registered_name : $comment->name;
      $comment->depth = count(explode('.', $comment->thread)) - 1;
      $this->comments[$comment->cid] = $comment;
    }
  }
}

/**
 * Template helper for theme_views_view_row_comment
 */
function template_preprocess_views_view_row_comment(&$vars) {
  $options = $vars['options'];
  $view = &$vars['view'];
  $plugin = &$view->style_plugin->row_plugin;
  $comment = $plugin->comments[$vars['row']->cid];
  $node = node_load($comment->nid);

  $links = '';
  if (!empty($options['links'])) {
    $links = module_invoke_all('link', 'comment', $comment, 0);
    drupal_alter('link', $links, $node);
  }

  $vars['comment'] = theme('comment_view', $comment, $node, $links);
}

/**
 * Plugin which formats the comments as RSS items.
 *
 * @ingroup views_plugin_rows
 */
class views_plugin_row_comment_rss extends views_plugin_row {

  function render($row) {
    global $base_url;

    if (empty($this->view->style_plugin->namespaces)) {
      $this->view->style_plugin->namespaces['xmlns:dc'] = 'http://purl.org/dc/elements/1.1/';
    }

    // Load the specified comment:
    $item = _comment_load($row->cid);
    $item->link = url('node/' . $item->nid, array('absolute' => TRUE, 'fragment' => 'comment-' . $item->cid));

    $properties = array(
      array('key' => 'pubDate', 'value' => gmdate('r', $item->timestamp)),
      array('key' => 'dc:creator', 'value' => $item->name),
      array(
        'key' => 'guid',
        'value' => 'comment ' .  $item->cid . ' at ' . $base_url,
        'attributes' => array('isPermaLink' => 'false')
      ),
    );
    $item_text = check_markup($item->comment, $item->format, FALSE);
    return format_rss_item($item->subject, $item->link, $item_text, $properties);
  }
}

/**
 * @}
 */
