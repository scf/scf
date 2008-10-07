<?php
// $Id: node.views.inc,v 1.68 2008/05/20 00:46:02 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers for node.module
 */

/**
 * @defgroup views_node_module node.module handlers
 *
 * Includes the tables 'node', 'node_revisions' and 'history'.
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function node_views_data() {
  // ----------------------------------------------------------------
  // node table -- basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node']['table']['group']  = t('Node');

  // Advertise this table as a possible base table
  $data['node']['table']['base'] = array(
    'field' => 'nid',
    'title' => t('Node'),
    'help' => t("Nodes are a Drupal site's primary content."),
    'weight' => -10,
  );

  // For other base tables, explain how we join
  $data['node']['table']['join'] = array(
    // this explains how the 'node' table (named in the line above)
    // links toward the node_revisions table.
    'node_revisions' => array(
      'handler' => 'views_join', // this is actually optional
      'left_table' => 'node_revisions', // Because this is a direct link it could be left out.
      'left_field' => 'nid',
      'field' => 'nid',
      // also supported:
      // 'type' => 'INNER',
      // 'extra' => array(array('field' => 'fieldname', 'value' => 'value', 'operator' => '='))
      //   Unfortunately, you can't specify other tables here, but you can construct
      //   alternative joins in the handlers that can do that.
      // 'table' => 'the actual name of this table in the database',
     ),
  );

  // ----------------------------------------------------------------
  // node table -- fields

  // nid
  $data['node']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node.'), // The help that appears on the UI,
    // Information for displaying the nid
    'field' => array(
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
    ),
    // Information for accepting a nid as an argument
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'nid',
    ),
    // Information for accepting a nid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    // Information for sorting on a nid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // title
  // This definition has more items in it than it needs to as an example.
  $data['node']['title'] = array(
    'title' => t('Title'), // The item it appears as on the UI,
    'help' => t('The title of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'field' => 'title', // the real field. This could be left out since it is the same.
      'group' => t('Node'), // The group it appears in on the UI. Could be left out.
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    // Information for accepting a title as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // created field
  $data['node']['created'] = array(
    'title' => t('Post date'), // The item it appears as on the UI,
    'help' => t('The date the node was posted.'), // The help that appears on the UI,
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

  // changed field
  $data['node']['changed'] = array(
    'title' => t('Updated date'), // The item it appears as on the UI,
    'help' => t('The date the node was last updated.'), // The help that appears on the UI,
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

  // Node type
  $data['node']['type'] = array(
    'title' => t('Type'), // The item it appears as on the UI,
    'help' => t('The type of a node (for example, "blog entry", "forum post", "story", etc).'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_node_type',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_node_type',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_type',
    ),
  );

  // published status
  $data['node']['status'] = array(
    'title' => t('Published'),
    'help' => t('The published status of the node.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // published status + extra
  $data['node']['status_extra'] = array(
    'title' => t('Published or admin'),
    'help' => t('Filters out unpublished nodes if the current user cannot view them.'),
    'filter' => array(
      'field' => 'status',
      'handler' => 'views_handler_filter_node_status',
      'label' => t('Published'),
    ),
  );

  // promote status
  $data['node']['promote'] = array(
    'title' => t('Promoted to front page'),
    'help' => t('The front page of the node.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Promoted to front page'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // sticky
  $data['node']['sticky'] = array(
    'title' => t('Sticky'), // The item it appears as on the UI,
    'help' => t('Whether or not the node is sticky.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Sticky'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // links to operate on the node

  $data['node']['view_node'] = array(
    'field' => array(
      'title' => t('Link'),
      'help' => t('Provide a simple link to the node.'),
      'handler' => 'views_handler_field_node_link',
    ),
  );

  $data['node']['edit_node'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the node.'),
      'handler' => 'views_handler_field_node_link_edit',
    ),
  );

  $data['node']['delete_node'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the node.'),
      'handler' => 'views_handler_field_node_link_delete',
    ),
  );

  // Language field if translation module enabled
  if (module_exists('translation')) {
    $data['node']['language'] = array(
      'title' => t('Language'),
      'help' => t('The language the content is in.'),
      'field' => array(
        'handler' => 'views_handler_field_node_language',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_node_language',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_node_language',
      ),
    );
  }

  // Bogus fields for aliasing purposes.

  $data['node']['created_fulldate'] = array(
    'title' => t('Created date'),
    'help' => t('In the form of CCYYMMDD.'),
    'argument' => array(
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_fulldate',
    ),
  );

  $data['node']['created_year_month'] = array(
    'title' => t('Created year + month'),
    'help' => t('In the form of YYYYMM.'),
    'argument' => array(
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_year_month',
    ),
  );

  $data['node']['created_year'] = array(
    'title' => t('Created year'),
    'help' => t('In the form of YYYY.'),
    'argument' => array(
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_year',
    ),
  );

  $data['node']['created_month'] = array(
    'title' => t('Created month'),
    'help' => t('In the form of MM (01 - 12).'),
    'argument' => array(
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_month',
    ),
  );

  $data['node']['created_week'] = array(
    'title' => t('Created week'),
    'help' => t('In the form of WW (01 - 53).'),
    'argument' => array(
      'field' => 'created',
      'handler' => 'views_handler_argument_node_created_week',
    ),
  );

  // ----------------------------------------------------------------------
  // Node revisions table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node_revisions']['table']['group']  = t('Node revision');

  // Advertise this table as a possible base table
  $data['node_revisions']['table']['base'] = array(
    'field' => 'vid',
    'title' => t('Node revision'),
    'help' => t('Node revisions are a history of changes to nodes.'),
  );

  // For other base tables, explain how we join
  $data['node_revisions']['table']['join'] = array(
    // Directly links to node table.
    'node' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );

  // uid field
  $data['node_revisions']['uid'] = array(
    'title' => t('User'),
    'help' => t('Relate a node revision to the user who created the revision.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
      'label' => t('user'),
    ),
  );

  // Body field
  $data['node_revisions']['body'] = array(
    'group' => t('Node'),
    'title' => t('Body'), // The item it appears as on the UI,
    'help' => t('The actual, full data in the body field; this may not be valid data on all node types.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format', // The name of the format field
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // Teaser field
  $data['node_revisions']['teaser'] = array(
    'group' => t('Node'),
    'title' => t('Teaser'), // The item it appears as on the UI,
    'help' => t('The stored teaser field. This may not be valid or useful data on all node types.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format', // The name of the format field
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // nid
  $data['node_revisions']['vid'] = array(
    'title' => t('Vid'),
    'help' => t('The revision ID of the node revision.'), // The help that appears on the UI,
    // Information for displaying the nid
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    // Information for accepting a nid as an argument
    'argument' => array(
      'handler' => 'views_handler_argument_node_vid',
      'click sortable' => TRUE,
      'numeric' => TRUE,
    ),
    // Information for accepting a nid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    // Information for sorting on a nid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // title
  $data['node_revisions']['title'] = array(
    'title' => t('Title'), // The item it appears as on the UI,
    'help' => t('The title of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'field' => 'title', // the real field
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // log field
  $data['node_revisions']['log'] = array(
    'title' => t('Log message'), // The item it appears as on the UI,
    'help' => t('The log message entered when the revision was created.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_xss',
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // revision timestamp
  // changed field
  $data['node_revisions']['timestamp'] = array(
    'title' => t('Created date'), // The item it appears as on the UI,
    'help' => t('The date the node revision was created.'), // The help that appears on the UI,
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

  $data['node_revisions']['revert_revision'] = array(
    'field' => array(
      'title' => t('Revert link'),
      'help' => t('Provide a simple link to revert to the revision.'),
      'handler' => 'views_handler_field_node_revision_link_revert',
    ),
  );

  $data['node_revisions']['delete_revision'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the node revision.'),
      'handler' => 'views_handler_field_node_revision_link_delete',
    ),
  );

  // ----------------------------------------------------------------------
  // History table

  // We're actually defining a specific instance of the table, so let's
  // alias it so that we can later add the real table for other purposes if we
  // need it.
  $data['history_user']['table']['group']  = t('Node');

  // Explain how this table joins to others.
  $data['history_user']['table']['join'] = array(
     // Directly links to node table.
    'node' => array(
      'table' => 'history',
      'left_field' => 'nid',
      'field' => 'nid',
      'extra' => array(
        array('field' => 'uid', 'value' => '***CURRENT_USER***', 'numeric' => TRUE),
      ),
    ),
  );

  $data['history_user']['timestamp'] = array(
    'title' => t('Has new content'),
    'field' => array(
      'handler' => 'views_handler_field_history_user_timestamp',
      'help' => t('Show a marker if the node has new or updated content.'),
    ),
    'filter' => array(
      'help' => t('Show only nodes that have new content.'),
      'handler' => 'views_handler_filter_history_user_timestamp',
    ),
  );
  return $data;
}

/**
 * Field handler to provide simple renderer that allows linking to a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   */
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = 'nid';
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_node'] = array(
      '#title' => t('Link this field to its node'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_node']),
    );
  }

  /**
   * Render whatever the data is as a link to the node.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_node'])) {
      return l($data, "node/" . $values->{$this->aliases['nid']}, array('html' => TRUE));
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
 * Field handler to translate a language into its readable form.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_language extends views_handler_field_node {
  function render($values) {
    $languages = locale_language_list();
    $value = $languages[$values->{$this->field_alias}];
    return $this->render_link($value, $values);
  }
}

/**
 * Field handler to translate a node type into its readable form.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_type extends views_handler_field_node {
  function render($values) {
    $value = node_get_types('name', $values->{$this->field_alias});
    return $this->render_link(check_plain($value), $values);
  }
}

/**
 * Field handler to present a link to the node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_link extends views_handler_field {
  function construct() {
    parent::construct();
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
    $nid = $values->{$this->aliases['nid']};
    return l($text, "node/$nid");
  }
}

/**
 * Field handler to present a link node edit.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_link_edit extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['type'] = 'type';
    $this->additional_fields['format'] = array('table' => 'node_revisions', 'field' => 'format');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->type = $values->{$this->aliases['type']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    return l($text, "node/$node->nid/edit", array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to present a link to delete a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_link_delete extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['type'] = 'type';
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['format'] = array('table' => 'node_revisions', 'field' => 'format');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->type = $values->{$this->aliases['type']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('delete', $node)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return l($text, "node/$node->nid/delete", array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to present a link to revert a node to a revision
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_revision_link_revert extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['node_vid'] = array('table' => 'node', 'field' => 'vid');
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['format'] = 'format';
  }

  function access() {
    return user_access('revert revisions') || user_access('administer nodes');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->vid = $values->{$this->aliases['vid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node)) {
      return;
    }

    // Current revision cannot be reverted.
    if ($node->vid == $values->{$this->aliases['node_vid']}) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('revert');
    return l($text, "node/$node->nid/revisions/$node->vid/revert", array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to present delete a node revision.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_revision_link_delete extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['node_vid'] = array('table' => 'node', 'field' => 'vid');
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['format'] = 'format';
  }

  function access() {
    return user_access('delete revisions') || user_access('administer nodes');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->vid = $values->{$this->aliases['vid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('delete', $node)) {
      return;
    }

    // Current revision cannot be deleted.
    if ($node->vid == $values->{$this->aliases['node_vid']}) {
      return;
    }
    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return l($text, "node/$node->nid/revisions/$node->vid/delete", array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to display the marker for new content.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_field_history_user_timestamp extends views_handler_field_node {
  function init(&$view, $options) {
    parent::init($view, $options);
    global $user;
    if ($user->uid) {
      $this->additional_fields['created'] = array('table' => 'node', 'field' => 'created');
      $this->additional_fields['changed'] = array('table' => 'node', 'field' => 'changed');
      if (module_exists('comment') && !empty($this->options['comments'])) {
        $this->additional_fields['last_comment'] = array('table' => 'node_comment_statistics', 'field' => 'last_comment_timestamp');
      }
    }
  }

  function options(&$options) {
    parent::options($options);
    $options['comments'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    if (module_exists('comment')) {
      $form['comments'] = array(
        '#type' => 'checkbox',
        '#title' => t('Check for new comments as well'),
        '#default_value' => !empty($this->options['comments']),
      );
    }
  }

  function query() {
    // Only add ourselves to the query if logged in.
    global $user;
    if (!$user->uid) {
      return;
    }
    parent::query();
  }

  function render($values) {
    // Let's default to 'read' state.
    // This code shadows node_mark, but it reads from the db directly and
    // we already have that info.
    $mark = MARK_READ;
    global $user;
    if ($user->uid) {
      $last_read = $values->{$this->field_alias};
      $created = $values->{$this->aliases['created']};
      $changed = $values->{$this->aliases['changed']};

      $last_comment = module_exists('comment') && !empty($this->options['comments']) ? $values->{$this->aliases['last_comment']} : 0;

      if (!$last_read && $created > NODE_NEW_LIMIT) {
        $mark = MARK_NEW;
      }
      elseif ($changed > $last_read && $changed > NODE_NEW_LIMIT) {
        $mark = MARK_UPDATED;
      }
      elseif ($last_comment > $last_read && $last_comment > NODE_NEW_LIMIT) {
        $mark = MARK_UPDATED;
      }
    }

    return $this->render_link(theme('mark', $mark), $values);
  }
}

/**
 * Argument handler to accept a node type.
 *
 * @ingroup views_argument_handlers
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

/**
 * Argument handler to accept a node id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_nid extends views_handler_argument_numeric {
  /**
   * Override the behavior of title(). Get the title of the node.
   */
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT n.title FROM {node} n WHERE n.nid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}

/**
 * Argument handler to accept a node revision id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_vid extends views_handler_argument_numeric {
  // No constructor is necessary.

  /**
   * Override the behavior of title(). Get the title of the revision.
   */
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT n.title FROM {node_revisions} n WHERE n.nid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->title);
    }
    return $titles;
  }
}

/**
 * Argument handler for a full date (CCYYMMDD)
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_created_fulldate extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $this->formula = views_date_sql_format('Ymd', "***table***.$this->real_field");
    $this->format = 'F j, Y';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return format_date(strtotime($created), 'custom', $this->format, 0);
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    return format_date(strtotime($this->argument), 'custom', $this->format, 0);
  }
}

/**
 * Argument handler for a year (CCYY)
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_created_year extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $this->formula = views_date_sql_extract('YEAR', "***table***.$this->real_field");
  }
}

/**
 * Argument handler for a year plus month (CCYYMM)
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_created_year_month extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $this->formula = views_date_sql_format('Ym', "***table***.$this->real_field");
    $this->format = 'F, Y';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return format_date(strtotime($created . "15"), 'custom', $this->format, 0);
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    return format_date(strtotime($this->argument . "15"), 'custom', $this->format, 0);
  }
}

/**
 * Argument handler for a month (MM)
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_created_month extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $this->formula = views_date_sql_extract('MONTH', "***table***.$this->real_field");
    $this->format = 'F';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $month = str_pad($data->{$this->name_alias}, 2, '0', STR_PAD_LEFT);
    return format_date(strtotime("2005" . $month . "15"), 'custom', $this->format);
  }

  /**
   * Provide a link to the next level of the view
   */
  function title() {
    $month = str_pad($this->argument, 2, '0', STR_PAD_LEFT);
    return format_date(strtotime("2005" . $month . "15"), 'custom', $this->format, 0);
  }

  function summary_argument($data) {
    // Make sure the argument contains leading zeroes.
    return str_pad($data->{$this->base_alias}, 2, '0', STR_PAD_LEFT);
  }
}

/**
 * Argument handler for a full date (CCYYMMDD)
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_created_week extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $this->formula = views_date_sql_extract('WEEK', "***table***.$this->real_field");
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return t('Week @week', array('@week' => $created));
  }
}

/**
 * Filter by node type
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_node_type extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Node type');
      $types = node_get_types();
      foreach ($types as $type => $info) {
        $options[$type] = $info->name;
      }
      $this->value_options = $options;
    }
  }
}

/**
 * Filter for new content
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_history_user_timestamp extends views_handler_filter {
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



/**
 * Filter by language
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_node_language extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Language');
      $languages = array('***CURRENT_LANGUAGE***' => t("Current user's language"), '***NO_LANGUAGE***' => t('No language'));
      $languages = array_merge($languages, locale_language_list());
      $this->value_options = $languages;
    }
  }
}

/**
 * Filter by published status
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_node_status extends views_handler_filter {
  function admin_summary() { }
  function operator_form() { }

  function query() {
    $table = $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$table.status <> 0 OR $table.uid = ***CURRENT_USER*** or ***ADMINISTER_NODES*** = 1");
  }
}

/**
 * Argument handler to accept a language.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_language extends views_handler_argument {
  function construct() {
    parent::construct('language');
  }

  /**
   * Override the behavior of summary_name(). Get the user friendly version
   * of the language.
   */
  function summary_name($data) {
    return $this->node_language($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title(). Get the user friendly version of the
   * node type.
   */
  function title() {
    return $this->node_language($this->argument);
  }

  function node_language($langcode) {
    $languages = locale_language_list();
    return isset($languages[$langcode]) ? $languages[$langcode] : t('Unknown language');
  }
}

/**
 * Implementation of hook_views_plugins
 */
function node_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'row' => array(
      'node' => array(
        'title' => t('Node'),
        'help' => t('Display the node with standard node view.'),
        'handler' => 'views_plugin_row_node_view',
        'theme' => 'views_view_row_node',
        'base' => array('node'), // only works with 'node' as base.
        'uses options' => TRUE,
        'type' => 'normal',
      ),
      'node_rss' => array(
        'title' => t('Node'),
        'help' => t('Display the node with standard node view.'),
        'handler' => 'views_plugin_row_node_rss',
        'base' => array('node'), // only works with 'node' as base.
        'uses options' => TRUE,
        'type' => 'feed',
      ),
    ),
    'argument validator' => array(
      'node' => array(
        'title' => t('Node'),
        'handler' => 'views_plugin_argument_validate_node',
      ),
    ),
    'argument default' => array(
      'node' => array(
        'title' => t('Node ID from URL'),
        'handler' => 'views_plugin_argument_default_node',
      ),
    ),
  );
}

/**
 * Plugin which performs a node_view on the resulting object.
 *
 * Most of the code on this object is in the theme function.
 *
 * @ingroup views_plugin_rows
 */
class views_plugin_row_node_view extends views_plugin_row {
  function options(&$options) {
    $options['teaser'] = TRUE;
    $options['links'] = TRUE;
  }

  function options_form(&$form, &$form_state) {
    $form['teaser'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display only teaser'),
      '#default_value' => $this->options['teaser'],
    );
    $form['links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    );
  }
}

/**
 * Template helper for theme_views_view_row_node
 */
function template_preprocess_views_view_row_node(&$vars) {
  $options = $vars['options'];
  $vars['node'] = ''; // make sure var is defined.
  $nid = $vars['row']->nid;
  if (!is_numeric($nid)) {
    return;
  }

  $node = node_load($nid);
  $node->view = $vars['view'];

  if (empty($node)) {
    return;
  }

  $vars['node'] = node_view($node, $options['teaser'], FALSE, $options['links']);
}

/**
 * Automatically allow view-based node templates if called from a view.
 */
function views_preprocess_node($vars) {
  if (!empty($vars['node']->view) && !empty($vars['node']->view->name)) {
    $vars['template_files'][] = 'node-view-' . $vars['node']->view->name;
  }
}

/**
 * Plugin which performs a node_view on the resulting object
 * and formats it as an RSS item.
 *
 * @ingroup views_plugin_rows
 */
class views_plugin_row_node_rss extends views_plugin_row {
  function options(&$options) {
    $options['item_length'] = 'default';
  }

  function options_form(&$form, &$form_state) {
    $form['item_length'] = array(
      '#type' => 'select',
      '#title' => t('Display type'),
      '#options' => array(
        'fulltext' => t('Full text'),
        'teaser' => t('Title plus teaser'),
        'title' => t('Title only'),
        'default' => t('Use default RSS settings'),
      ),
      '#default_value' => $this->options['item_length'],
    );
  }

  function render($row) {
    // For the most part, this code is taken from node_feed() in node.module
    global $base_url;

    $item_length = $this->options['item_length'];
    if ($item_length == 'default') {
      $item_length = variable_get('feed_item_length', 'teaser');
    }

    if (empty($this->view->style_plugin->namespaces)) {
      $this->view->style_plugin->namespaces['xmlns:dc'] = 'http://purl.org/dc/elements/1.1/';
    }

    // Load the specified node:
    $item = node_load($row->nid);
    $item->build_mode = NODE_BUILD_RSS;
    $item->link = url("node/$row->nid", array('absolute' => TRUE));

    if ($item_length != 'title') {
      $teaser = ($item_length == 'teaser') ? TRUE : FALSE;

      // Filter and prepare node teaser
      if (node_hook($item, 'view')) {
        $item = node_invoke($item, 'view', $teaser, FALSE);
      }
      else {
        $item = node_prepare($item, $teaser);
      }

      // Allow modules to change $node->teaser before viewing.
      node_invoke_nodeapi($item, 'view', $teaser, FALSE);
    }

    // Allow modules to add additional item fields and/or modify $item
    $extra = node_invoke_nodeapi($item, 'rss item');
    $extra = array_merge($extra,
      array(
        array('key' => 'pubDate', 'value' => gmdate('r', $item->created)),
        array('key' => 'dc:creator', 'value' => $item->name),
        array(
          'key' => 'guid',
          'value' => $item->nid . ' at ' . $base_url,
          'attributes' => array('isPermaLink' => 'false')
        ),
      )
    );
    foreach ($extra as $element) {
      if (isset($element['namespace'])) {
        $this->view->style_plugin->namespaces = array_merge($this->view->style_plugin->namespaces, $element['namespace']);
      }
    }

    // Prepare the item description
    switch ($item_length) {
      case 'fulltext':
        $item_text = $item->body;
        break;
      case 'teaser':
        $item_text = $item->teaser;
        if (!empty($item->readmore)) {
          $item_text .= '<p>'. l(t('read more'), 'node/'. $item->nid, array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))) .'</p>';
        }
        break;
      case 'title':
        $item_text = '';
        break;
    }

    return format_rss_item($item->title, $item->link, $item_text, $extra);
  }
}

/**
 * Validate whether an argument is an acceptable node.
 */
class views_plugin_argument_validate_node extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_node_type';

  function validate_form(&$form, &$form_state) {
    $types = node_get_types();
    foreach ($types as $type => $info) {
      $options[$type] = $info->name;
    }

    $arg = $this->get_argument();
    if (empty($arg)) {
      $arg = array();
    }

    $form[$this->option_name] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div id="edit-options-validate-argument-node-type-wrapper">',
      '#suffix' => '</div>',
      '#title' => t('Types'),
      '#options' => $options,
      '#default_value' => $arg,
      '#description' => t('If you wish to validate for specific node types, check them; if none are checked, all nodes will pass.'),
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_node_access'] = array(
      '#type' => 'checkbox',
      '#title' => t('Validate user has access to the node'),
      '#default_value' => !empty($this->argument->options['validate_argument_node_access']),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_nid_type'] = array(
      '#type' => 'select',
      '#title' => t('Argument type'),
      '#options' => array(
        'nid' => t('Node ID'),
        'nids' => t('Node IDs separated by , or +'),
      ),
      '#default_value' => isset($this->argument->options['validate_argument_nid_type']) ? $this->argument->options['validate_argument_nid_type'] : 'nid',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );
  }

  function validate_argument($argument) {
    $types = array_filter($this->argument->options[$this->option_name]);

    $type = isset($this->argument->options['validate_argument_nid_type']) ? $this->argument->options['validate_argument_nid_type'] : 'nid';

    switch ($type) {
      case 'nid':
        if (!is_numeric($argument)) {
          return FALSE;
        }
        $node = node_load($argument);
        if (!$node) {
          return FALSE;
        }

        if (!empty($this->argument->options['validate_argument_node_access'])) {
          if (!node_access('view', $node)) {
            return FALSE;
          }
        }

        // Save the title() handlers some work.
        $this->argument->validated_title = check_plain($node->title);

        if (empty($types)) {
          return TRUE;
        }

        return isset($types[$node->type]);
      break;
      case 'nids':
        $nids = new stdClass();
        $nids->value = array($argument);
        $nids = views_break_phrase($argument, $nids);
        if ($nids->value == -1) {
          return FALSE;
        }

        $placeholders = implode(', ', array_fill(0, sizeof($nids->value), '%d'));

        $test = drupal_map_assoc($nids->value);
        $titles = array();

        $result = db_query("SELECT * FROM {node} WHERE nid IN ($placeholders)", $nids->value);
        while ($node = db_fetch_object($result)) {
          if ($types && empty($types[$node->type])) {
            return FALSE;
          }

          if (!empty($this->argument->options['validate_argument_node_access'])) {
            if (!node_access('view', $node)) {
              return FALSE;
            }
          }

          $titles[] = check_plain($node->title);
          unset($test[$node->nid]);
        }

        $this->argument->validated_title = implode($nids->operator == 'or' ? ' + ' : ', ', $titles);
        // If this is not empty, we did not find a nid.
        return empty($test);
    }
  }
}

/**
 * Default argument plugin to extract a node via menu_get_object
 */
class views_plugin_argument_default_node extends views_plugin_argument_default {
  function argument_form(&$form, &$form_state) {
  }

  function get_argument() {
    foreach (range(1, 3) as $i) {
      $node = menu_get_object('node', $i);
      if (!empty($node)) {
        return $node->nid;
      }
    }
  }
}

/**
 * Implementation of hook_views_query_substitutions().
 */
function node_views_query_substitutions() {
  global $language;
  return array(
    '***ADMINISTER_NODES***' => intval(user_access('administer nodes')),
  );
}

/**
 * Implementation of hook_views_analyze().
 */
function node_views_analyze($view) {
  $ret = array();
  // Check for something other than the default display:
  if ($view->base_table == 'node') {
    foreach ($view->display as $id => $display) {
      if (!$display->handler->is_defaulted('access') || !$display->handler->is_defaulted('filters')) {
        // check for no access control
        $access = $display->handler->get_option('access');
        if (empty($access['type']) || $access['type'] == 'none') {
          $filters = $display->handler->get_option('filters');
          foreach ($filters as $filter) {
            if ($filter['table'] == 'node' && ($filter['field'] == 'status' || $filter['field'] == 'status_extra')) {
              continue 2;
            }
          }
          $ret[] = views_ui_analysis(t('Display %display has no access control but does not contain a filter for published nodes.', array('%display' => $display->display_title)), 'warning');
        }
      }
    }
  }

  return $ret;
}

/**
 * @}
 */
