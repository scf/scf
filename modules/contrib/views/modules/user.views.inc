<?php
//$Id: user.views.inc,v 1.43 2008/05/27 20:53:59 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers for user.module
 */

/**
 * @defgroup views_user_module user.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function user_views_data() {
  // ----------------------------------------------------------------
  // users table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['users']['table']['group']  = t('User');

  $data['users']['table']['base'] = array(
    'field' => 'uid',
    'title' => t('User'),
    'help' => t('Users who have created accounts on your site.'),
  );

  $data['users']['table']['join'] = array(
    'node' => array(
      'left_field' => 'uid',
      'field' => 'uid',
      'type' => 'INNER', // all nodes have an author.
    ),
    // This goes to the node so that we have consistent authorship.
    'node_revisions' => array(
      'left_table' => 'node',
      'left_field' => 'uid',
      'field' => 'uid',
      'type' => 'INNER', // all nodes have an author.
    ),
  );

  // uid
  $data['users']['uid'] = array(
    'title' => t('Uid'),
    'help' => t('The user ID'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
      'name field' => 'name', // display this field in the summary
    ),
    'filter' => array(
      'title' => t('Name'),
      'handler' => 'views_handler_filter_user_name',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // uid
  $data['users']['uid_current'] = array(
    'real field' => 'uid',
    'title' => t('Current'),
    'help' => t('Filter the view to the currently logged in user.'),
    'filter' => array(
      'handler' => 'views_handler_filter_user_current',
    ),
  );

  // name
  $data['users']['name'] = array(
    'title' => t('Name'), // The item it appears as on the UI,
    'help' => t('The user or author name.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_user_name',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // mail
  // Note that this field implements field level access control.
  $data['users']['mail'] = array(
    'title' => t('E-mail'), // The item it appears as on the UI,
    'help' => t('Email address for a given user. Only accessible to users with <em>administer users</em> permission'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_user_mail',
      'click sortable' => TRUE,
      'access callback' => 'user_access',
      'access arguments' => array('administer users'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // picture
  $data['users']['picture'] = array(
    'title' => t('Picture'),
    'help' => t("The user's picture, if allowed."), // The help that appears on the UI,
    // Information for displaying the uid
    'field' => array(
      'handler' => 'views_handler_field_user_picture',
      'click sortable' => TRUE,
    ),
  );

    // created field
  $data['users']['created'] = array(
    'title' => t('Created date'), // The item it appears as on the UI,
    'help' => t('The date the user was created.'), // The help that appears on the UI,
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

  // access field
  $data['users']['access'] = array(
    'title' => t('Last access'), // The item it appears as on the UI,
    'help' => t("The user's last access date."), // The help that appears on the UI,
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

  // login field
  $data['users']['login'] = array(
    'title' => t('Last login'), // The item it appears as on the UI,
    'help' => t("The user's last login date."), // The help that appears on the UI,
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

  // published status
  $data['users']['status'] = array(
    'title' => t('Active'), // The item it appears as on the UI,
    'help' => t('Whether a user is active or blocked.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Active'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // log field
  $data['users']['signature'] = array(
    'title' => t('Signature'), // The item it appears as on the UI,
    'help' => t("The user's signature."), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_xss',
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  $data['users']['edit_node'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the user.'),
      'handler' => 'views_handler_field_user_link_edit',
    ),
  );

  $data['users']['delete_node'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the user.'),
      'handler' => 'views_handler_field_user_link_delete',
    ),
  );

  // ----------------------------------------------------------------------
  // users_roles table

  $data['users_roles']['table']['group']  = t('User');

  // Explain how this table joins to others.
  $data['users_roles']['table']['join'] = array(
     // Directly links to users table.
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node_revisions' => array(
      'left_table' => 'node',
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );

  $data['users_roles']['rid'] = array(
    'title' => t('Roles'),
    'help' => t('Roles that a user belongs to.'),
    'field' => array(
      'handler' => 'views_handler_field_user_roles',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_user_roles',
      'numeric' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_users_roles_rid',
      'name table' => 'role',
      'name field' => 'name',
      'empty field name' => t('No role'),
      'numeric' => TRUE,
    ),
  );

  // ----------------------------------------------------------------------
  // role table

  $data['role']['table']['join'] = array(
     // Directly links to users table.
    'users' => array(
      'left_table' => 'users_roles',
      'left_field' => 'rid',
      'field' => 'rid',
    ),
    // needed for many to one helper sometimes
    'users_roles' => array(
      'left_field' => 'rid',
      'field' => 'rid',
    ),
    'node' => array(
      'left_table' => 'users_roles',
      'left_field' => 'rid',
      'field' => 'rid',
    ),
    'node_revisions' => array(
      'left_table' => 'users_roles',
      'left_field' => 'rid',
      'field' => 'rid',
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_plugins
 */
function user_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
/*
    'argument validator' => array(
      'user' => array(
        'title' => t('User'),
        'handler' => 'views_plugin_argument_validate_user',
      ),
    ),
*/
    'argument default' => array(
      'user' => array(
        'title' => t('User ID from URL'),
        'handler' => 'views_plugin_argument_default_user',
      ),
    ),
  );
}

/**
 * Default argument plugin to extract a user via menu_get_object
 */
class views_plugin_argument_default_user extends views_plugin_argument_default {
  var $option_name = 'default_argument_user';

  function argument_form(&$form, &$form_state) {
    $form[$this->option_name] = array(
      '#type' => 'checkbox',
      '#title' => t('Also look for a node and use the node author'),
      '#default_value' => !empty($this->argument->options[$this->option_name]),
      '#process' => array('views_process_dependency'),
      '#dependency' => array(
        'radio:options[default_action]' => array('default'),
        'radio:options[default_argument_type]' => array($this->id)
      ),
      '#dependency_count' => 2,
    );
  }

  function get_argument() {
    foreach (range(1, 3) as $i) {
      $user = menu_get_object('user', $i);
      if (!empty($user)) {
        return $user->uid;
      }
    }

    foreach (range(1, 3) as $i) {
      $user = menu_get_object('user_uid_optional', $i);
      if (!empty($user)) {
        return $user->uid;
      }
    }

    if (!empty($this->argument->options[$this->option_name])) {
      foreach (range(1, 3) as $i) {
        $node = menu_get_object('node', $i);
        if (!empty($node)) {
          return $node->uid;
        }
      }
    }
  }
}

/**
 * Field handler to provide simple renderer that allows linking to a user.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user extends views_handler_field {
  /**
   * Override init function to provide generic option to link to user.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (!empty($this->options['link_to_user'])) {
      $this->additional_fields['uid'] = 'uid';
    }
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = TRUE;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_user'] = array(
      '#title' => t('Link this field to its user'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_user'],
    );
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_user']) && user_access('access user profiles') && $values->{$this->aliases['uid']}) {
      return l($data, "user/" . $values->{$this->aliases['uid']}, array('html' => TRUE));
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
 * Field handler to provide simple renderer that allows using a themed user link
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_name extends views_handler_field_user {
  function render_link($data, $values) {
    if (!empty($this->options['link_to_user'])) {
      $account = new stdClass();
      $account->name = $values->{$this->field_alias};
      $account->uid = $values->{$this->aliases['uid']};
      return theme('username', $account);
    }
    else {
      return $data;
    }
  }
}

/**
 * Field handler to present a link to the user.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_link extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
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

  // An example of field level access control.
  function access() {
    return user_access('access user profiles');
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    $uid = $values->{$this->aliases['uid']};
    return l($text, "user/$uid");
  }
}

/**
 * Field handler to present a link to user edit.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_link_edit extends views_handler_field_user_link {
  // An example of field level access control.
  function access() {
    return user_access('administer users');
  }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    $uid = $values->{$this->aliases['uid']};
    return l($text, "user/$uid/edit", array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to present a link to user delete.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_link_delete extends views_handler_field_user_link {
  // An example of field level access control.
  function access() {
    return user_access('administer users');
  }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    $uid = $values->{$this->aliases['uid']};
    return l($text, "user/$uid/delete", array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to provide acess control for the email field
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_mail extends views_handler_field_user {
  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = 'mailto';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_user'] = array(
      '#title' => t('Link this field'),
      '#type' => 'radios',
      '#options' => array(
        0 => t('No link'),
        'user' => t('To the user'),
        'mailto' => t("With a mailto:"),
      ),
      '#default_value' => $this->options['link_to_user'],
    );
  }

  function render($values) {
    if ($this->options['link_to_user'] == 'mailto') {
      return l($values->{$this->field_alias}, "mailto:" . $values->{$this->field_alias});
    }
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

/**
 * Field handler to provide simple renderer that allows using a themed user link
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_picture extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['name'] = 'name';
  }

  function render($values) {
    // Fake an account object.
    $account = new stdClass();
    $account->uid = $values->{$this->aliases['uid']};
    $account->name = $values->{$this->aliases['name']};
    $account->picture = $values->{$this->field_alias};

    return theme('user_picture', $account);
  }
}

/**
 * Field handler to provide a list of roles.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_roles extends views_handler_field_prerender_list {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'users', 'field' => 'uid');
  }

  function query() {
    $this->add_additional_fields();
    $this->field_alias = $this->aliases['uid'];
  }

  function pre_render($values) {
    $uids = array();
    $this->items = array();

    foreach ($values as $result) {
      $uids[] = $result->{$this->aliases['uid']};
    }

    if ($uids) {
      $result = db_query("SELECT u.uid, u.rid, r.name FROM {role} r INNER JOIN {users_roles} u ON u.rid = r.rid WHERE u.uid IN (" . implode(', ', $uids) . ") ORDER BY r.name");
      while ($role = db_fetch_object($result)) {
        $this->items[$role->uid][$role->rid] = check_plain($role->name);
      }
    }
  }
}

/**
 * Argument handler to accept a user id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_user_uid extends views_handler_argument_numeric {
  /**
   * Override the behavior of title(). Get the name of the user.
   */
  function title_query() {
    if (!$this->argument) {
      return array(variable_get('anonymous', t('Anonymous')));
    }

    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT u.name FROM {users} u WHERE u.uid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->name);
    }
    return $titles;
  }
}

/**
 * Filter handler for usernames
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_user_name extends views_handler_filter_in_operator {
  function value_form(&$form, &$form_state) {
    $values = array();
    if ($this->value) {
      $result = db_query("SELECT * FROM {users} u WHERE uid IN ("  . implode(', ', $this->value) . ")");
      while ($account = db_fetch_object($result)) {
        if ($account->uid) {
          $values[] = $account->name;
        }
        else {
          $values[] = 'Anonymous'; // Intentionally NOT translated.
        }
      }
    }

    sort($values);
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t('Usernames'),
      '#description' => t('Enter a comma separated list of user names.'),
      '#default_value' => implode(', ', $values),
      '#autocomplete_path' => 'admin/views/ajax/autocomplete/user',
    );
  }

  function value_validate(&$form, &$form_state) {
    $values = drupal_explode_tags($form_state['values']['options']['value']);
    $uids = $this->validate_user_strings($form['value'], $values);

    if ($uids) {
      $form_state['values']['options']['value'] = $uids;
    }
  }

  function exposed_validate(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }

    if (empty($this->options['expose']['identifier'])) {
      return;
    }

    $identifier = $this->options['expose']['identifier'];
    $values = drupal_explode_tags($form_state['values'][$identifier]);

    $uids = $this->validate_user_strings($form[$identifier], $values);

    if ($uids) {
      $form_state['values'][$identifier] = $uids;
    }
  }

  /**
   * Validate the user string. Since this can come from either the form
   * or the exposed filter, this is abstracted out a bit so it can
   * handle the multiple input sources.
   */
  function validate_user_strings(&$form, $values) {
    $uids = array();
    $placeholders = array();
    $args = array();
    $results = array();
    foreach ($values as $value) {
      if (strtolower($value) == 'anonymous') {
        $uids[] = 0;
      }
      else {
        $missing[strtolower($value)] = TRUE;
        $args[] = $value;
        $placeholders[] = "'%s'";
      }
    }

    if (!$args) {
      return $uids;
    }

    $result = db_query("SELECT * FROM {users} WHERE name IN (" . implode(', ', $placeholders) . ")", $args);
    while ($account = db_fetch_object($result)) {
      unset($missing[strtolower($account->name)]);
      $uids[] = $account->uid;
    }

    if ($missing) {
      form_error($form, format_plural(count($missing), 'Unable to find user: @users', 'Unable to find users: @users', array('@users' => implode(', ', array_keys($missing)))));
    }

    return $uids;
  }

  function value_submit() {
    // prevent array filter from removing our anonymous user.
  }

  // Override to do nothing.
  function get_value_options() { }

  function admin_summary() {
    // set up $this->value_options for the parent summary
    $this->value_options = array();

    if ($this->value) {
      $result = db_query("SELECT * FROM {users} u WHERE uid IN ("  . implode(', ', $this->value) . ")");

      while ($account = db_fetch_object($result)) {
        if ($account->uid) {
          $this->value_options[$account->uid] = $account->name;
        }
        else {
          $this->value_options[$account->uid] = 'Anonymous'; // Intentionally NOT translated.
        }
      }
    }

    return parent::admin_summary();
  }
}

/**
 * Filter handler for the current user
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_user_current extends views_handler_filter_boolean_operator {
  function construct() {
    parent::construct();
    $this->value_value = t('Is the logged in user');
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . (empty($this->value) ? '!=' : '=') . " ***CURRENT_USER***");
  }
}

/**
 * Filter handler for user roles
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_user_roles extends views_handler_filter_many_to_one {
  function get_value_options() {
    $this->value_options = user_roles(TRUE);
    unset($this->value_options[DRUPAL_AUTHENTICATED_RID]);
  }
}

/**
 * Allow replacement of current userid so we can cache these queries
 */
function user_views_query_substitutions($view) {
  global $user;
  return array('***CURRENT_USER***' => intval($user->uid));
}

/**
 * Allow role ID(s) as argument
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_users_roles_rid extends views_handler_argument_many_to_one {
  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT name FROM {role} WHERE rid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->name);
    }
    return $titles;
  }
}

/**
 * @}
 */
