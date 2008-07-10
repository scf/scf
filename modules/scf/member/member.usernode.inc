<?php

// $Id: $

// ==================================
// ==================================
// ====== THIS FILE IS OBSOLETE =====
// ==================================
// ==================================


/**
 * @file Allows users to maintain a catalog of members with rich
 * information about each member.
 * 
 * Much code and ideas have been copied or adapted from the usernode
 * module by Wolfgang Ziegler, nuppla@zites.net, and also from the
 * core profile module.
 */


/**
 * Implementation of hook_node_info().
 */
function member_node_info() {
  return array(
    'member' => array(
      'name' => t('Member'),
      'module' => 'member',
      'description' => t("Member catalog."),
      'title_label' => t('Full name'),
      'has_body' => TRUE,
      'body_label' => t('Biography')
    )
  );
}

/**
 * Implementation of hook_menu().
 */
function member_menu() {
  drupal_set_message("In member_menu()");
  $items['member/autocomplete'] = array(
    'title' => 'Member autocomplete',
    'page callback' => '_member_autocomplete',
    'access callback' => TRUE,
    'type' => MENU_CALLBACK
  );
  return $items;
}


/**
 * Implementation of hook_form().
 */
function member_form(&$node, $form_state) {
  $admin = user_access('administer members');
  
  $form = _member_user_form_fields($node);

  // temporarily store a newly registering user's user object,
  // so that member_submit() can set the user's actual uid on node creation.
  // TODO: I don't think this is necessary...
  if (isset($node->user)) {
    $form['user'] = array(
      '#type' => 'value',
      '#value' => $node->user,
    );
  }

  return $form;

}

/**
 * Implementation of member_submit():
 * Use the temporarily stored user object of a newly registering user,
 * so that the member points to the actual user instead of Anonymous.
 */
function member_submit(&$node) {
  global $user;
  $node->uid = $user->uid;
  $muser = $user;
  if (isset($node->user)) {
    $muser = $node->user;
  }
  $node->muid = $muser->uid;
  //unpublish the member if the user is blocked
  $node_options = variable_get('node_options_member', array('status', 'promote'));
  if (in_array('status', $node_options)) {
    $node->status = $muser->status;
  }
}


/**
 * Implementation of hook_insert():
 */
function member_insert($node) {
  $sql = "INSERT INTO {member} (" . _member_table_field_string() . ") VALUES "
    . "(%d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')";
  db_query($sql, $node->nid, $node->vid, $node->muid, $node->firstname, $node->lastname,
    $node->middleinitial, $node->sortname, $node->jobtitle, $node->affiliation, 
    $node->phone, $node->fax);
}

/**
 * Implementation of hook_update():
 */
function member_update($node) {
  $sql = "UPDATE {member} SET nid = %d, vid = %d, muid = %d, firstname = '%s', lastname = '%s',"
    . " middleinitial = '%s', sortname = '%s', jobtitle = '%s', affiliation = '%s', phone = '%s', fax = '%s'"
    . " WHERE nid = %d AND vid = %d";
  db_query($sql, $node->nid, $node->vid, $node->muid, $node->firstname, $node->lastname,
    $node->middleinitial, $node->sortname, $node->jobtitle, $node->affiliation, 
    $node->phone, $node->fax, $node->nid, $node->vid);
}

/**
 * Implementation of hook_load():
 */
function member_load($node) {
  drupal_set_message("member_load()");
  $sql = "SELECT " . _member_table_field_string() . " FROM {member} WHERE vid = %d";
  $mem = db_fetch_object(db_query($sql, $node->vid));
  if ($mem) {
    $mem->user = user_load(array('uid' => $mem->muid));
  }
  return $mem;
}


/**
 * Implementation of hook_delete().
 * Insane code mostly stolen from usernode.
 */
function member_delete(&$node) {
  // prevent deletion of members, if a module directly calls node_delete()
  drupal_set_message('You may not manually delete a member! Delete the '.
    'according user and the member is going to be deleted too.', 'error');
  
  $user = _member_get_user($node);
  
  // the node has already been deleted, so save it again
  $nid = $node->nid;
  unset($node->nid);
  
  node_save($node);
  
  // change to old nid
  db_query("UPDATE {node} SET nid = %d WHERE nid = %d", $nid, $node->nid);
  db_query("UPDATE {node_revisions} SET nid = %d WHERE nid = %d", $nid, $node->nid);
  
  // simulate deleting of new nid
  node_invoke_nodeapi($node, 'delete');
  
  // prevent other modules from reacting on old node deletion through nodeapi
  // TODO: redirect to the referrer, not to the homepage.
  drupal_goto('');
}


/**
 * Implementation of hook_user().
 */
function member_user($op, &$edit, &$account, $category = NULL) {
  drupal_set_message("member_user('$op')");
  // drupal_set_message("edit = ");
  // dvm($edit);
  // drupal_set_message("account = ");
  // dvm($account);
  switch ($op) {
    case 'form':
      if (isset($account->member)) {
        $node = $account->member;
        return _member_user_form_fields($node, TRUE);
      }
      // else fall through to 'register'...
    case 'register':
      $fake_node = (object)$edit;
      $fake_node->type = 'member';
      return _member_user_form_fields($fake_node, TRUE);
    case 'load':
      _member_get_node_id($account);
      break;
    case 'insert':
      _member_create_node($edit, $account);
      break;
    case 'update':
      $node = _member_get_node($account);
      _member_save_node($edit, $node);
      break;
      /*
    case 'after_update':
       return $category == 'account' ? member_update_node($account) : '';
    case 'delete':
      return member_delete_node($account);

    case 'update':
      return profile_save_profile($edit, $account, $category);
    case 'view':
      return profile_view_profile($account);
    case 'categories':
      return array();
      // return profile_categories();
    case 'delete':
      db_query('DELETE FROM {profile_values} WHERE uid = %d', $account->uid);
      */
  }
}

/**
 * Implementation of hook_enable().
 */
function member_enable() {
  //when the module is activated, we check the members
  _member_check_all();
}

/**
 * Implementation of hook_cron().
 */
function member_cron() {
  _member_check_all();
}


// -------------------------------- internal (non-hook) functions

/**
 * Callback to allow autocomplete of profile text fields.
 */
function _member_autocomplete($field, $string) {
  drupal_set_message("In member_autocomplete()");
  $matches = array();
  switch ($field) {
    case 'affiliation':
    case 'jobtitle':
      $sql = "SELECT DISTINCT $field as value FROM {member} WHERE LOWER($field) LIKE LOWER('%s%%') ORDER BY $field ASC";
      $result = db_query_range($sql, $string, 0, 20);
      while ($data = db_fetch_object($result)) {
        $matches[$data->value] = check_plain($data->value);
      }
  }
  drupal_json($matches);
}

/**
 * Return the id of the member node of the user.
 * @param $user The user object or the user's uid
 */
function _member_get_node_id(&$user) {
  // cache result so only do this once per req.
  static $history = array();
  if (!is_object($user)) {
    if (!isset($history[$user])) {
      $history[$user] = db_result(db_query("SELECT nid FROM {member} WHERE muid = %d", $user));
    }
    return $history[$user];
  }

  // else ($user is an obj)
  if (!isset($user->member_id)) {
    $user->member_id = _member_get_node_id($user->uid);
  }
  return $user->member_id;
}

function _member_get_node(&$user) {
  if (!is_object($user)) {
    $nid = _member_get_node_id($user);
    return $nid ? node_load($nid) : FALSE;
  }
  if (!isset($user->member)) {
    $nid = _member_get_node_id($user);
    $user->member = $nid ? node_load($nid) : FALSE;
  }
  return $user->member;
}

function _member_table_fields() {
  static $fields = array(
    'nid',
    'vid',
    'muid',
    'firstname',
    'lastname',
    'middleinitial',
    'sortname',
    'jobtitle',
    'affiliation',
    'phone',
    'fax'
  );
  return $fields;
}

function _member_table_field_string() {
  static $str;
  $str = implode(', ', _member_table_fields());
  return $str;
}

/**
 * Return the user object of the member.
 */
function _member_get_user($node) {
  return $node->user;
}


/**
 * Create an associated node. Called by hook_user().
 */
function _member_create_node(&$edit, &$user) {
  $nodearr = array(
    'user' => $user,
    'type' => 'member',
    'name' => $user->name,
    'muid' => $user->uid
  );
  $node = (object)$nodearr;
  $user->member = $node;
  _member_save_node($edit, $node);
}

/* 
 * Stolen from usernode module:
 * Saves a member by using drupal_execute.
 * This allows modules like auto_nodetitle to work and makes sure all hooks are invoked properly
 */
function _member_save_node(&$edit, $node) {
  // workaround to disable drupal message "Your member has been created"
  // $messages = drupal_get_messages();

  // HACK HACK HACK: the 'node_form' function is not defined in node.module, but in an include...
  // create the member
  module_load_include("inc", "node", "node.pages");

  $form_state['values'] = $edit;
  $form_state['values']['op'] = t('Save');
  drupal_execute('member_node_form', $form_state, $node);

  // write back the old messages
  // PROBLEM: this also hides any helpful error msgs during dev
  //$_SESSION['messages'] = $messages;
}


function _member_user_form_fields($node, $fieldset = FALSE) {
  $wt = 1;
  $form = array();
  $fs = &$form;
  $type = node_get_types('type', $node);
  
  if ($fieldset) { // if in user page
    // put in profile fieldset...
    $form['profile'] = array(
      '#type' => 'fieldset',
      '#weight' => 5,
      '#title' => t('Profile information'),
      );
    $fs = &$form['profile'];
  } 
  
  $fs['firstname'] = array(
    '#type' => 'textfield',
    '#title' => t('First name'),
    '#required' => TRUE,
    '#default_value' => isset($node->firstname) ? $node->firstname : '',
    '#maxlength' => 32,
    '#weight' => $wt++,
    // '#autocomplete_path' => 'member/autocomplete/firstname'
  );
  $fs['middleinitial'] = array(
    '#type' => 'textfield',
    '#title' => t('Middle initial'),
    '#required' => FALSE,
    '#default_value' => isset($node->middleinitial) ? $node->middleinitial : '',
    '#maxlength' => 5,
    '#weight' => $wt++
  );
  $fs['lastname'] = array(
    '#type' => 'textfield',
    '#title' => t('Last name'),
    '#required' => TRUE,
    '#maxlength' => 32,
    '#default_value' => isset($node->lastname) ? $node->lastname : '',
    '#weight' => $wt++,
    // '#autocomplete_path' => 'member/autocomplete/lastname',
    );
  $fs['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => isset($node->title) ? $node->title : '',
    '#weight' => $wt++,
    );
  $fs['sortname'] = array(
    '#type' => 'textfield',
    '#title' => t('Sort name'),
    '#required' => TRUE,
    '#default_value' => isset($node->sortname) ? $node->sortname : '',
    '#weight' => $wt++,
    );
  
  $fs['affiliation'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliation'),
    '#required' => FALSE,
    '#default_value' => isset($node->affiliation) ? $node->affiliation : '',
    '#weight' => $wt++,
    '#autocomplete_path' => 'member/autocomplete/affiliation',
    );
  $fs['jobtitle'] = array(
    '#type' => 'textfield',
    '#title' => t('Position'),
    '#required' => FALSE,
    '#maxlength' => 80,
    '#default_value' => isset($node->jobtitle) ? $node->jobtitle : '',
    '#weight' => $wt++,
    '#autocomplete_path' => 'member/autocomplete/jobtitle'
  );
  $fs['phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#required' => FALSE,
    '#default_value' => isset($node->phone) ? $node->phone : '',
    '#weight' => $wt++
  );
  $fs['fax'] = array(
    '#type' => 'textfield',
    '#title' => t('Fax'),
    '#required' => FALSE,
    '#weight' => $wt++
  );
  if (!isset($node->format)) {
    // avoid "Undefined property: stdClass::$format" error in node_body_field() call below
    $node->format = NULL;
  }
  if (!isset($node->body)) {
    $node->body = NULL;
  }
  // HACK HACK HACK: the 'node_body_field' function is not defined in node.module, but in an include...
  module_load_include("inc", "node", "node.pages");
  $fs['body_field'] = node_body_field($node, $type->body_label, $type->min_word_count);
  /**
   * Basic node information.
   * These elements are just values so they are not even sent to the client.
   */
  foreach (array('nid', 'vid', 'created', 'type', 'language') as $key) {
    $fs[$key] = array('#type' => 'value', '#value' => isset($node->$key) ? $node->$key : NULL);
  }

  /*
  $fs['body'] = array(
    '#type' => 'textarea',
    '#title' => check_plain($type->body_label),
    '#required' => FALSE,
    '#default_value' => isset($node->body) ? $node->body : '',
    '#weight' => $wt++
  );
  */
  return $form;
}

/**
 * Check all users for an existing member, and create one if necessary.
 */
function _member_check_all() {
  // TODO:
  // delete members whose users have been deleted while the module was deactivated
  //  $result = db_query("SELECT m.nid, m.vid FROM {member} m ".
  //    "LEFT JOIN {node} n ON m.nid = n.nid ".
  //    "WHERE n.nid IS NULL");
  //  while ($row = db_fetch_object($result)) {
  //    db_query("DELETE FROM {member} WHERE vid IN ($subquery)");
  //  }

  // create members for all existing users without a member
  $result = db_query("SELECT u.* FROM {users} u ".
    "LEFT JOIN {member} m ON u.uid = m.muid ".
    "WHERE m.nid IS NULL AND u.uid > 1");
  while ($vals = db_fetch_array($result)) {
    $user = (object)$vals;
    $vals['muid'] = $vals['uid'];
    $vals['name'] = $vals['name'];
    $vals['title'] = $vals['name'];
    $vals['sortname'] = $vals['name'];
    _member_create_node($vals, $user);
  }
}

