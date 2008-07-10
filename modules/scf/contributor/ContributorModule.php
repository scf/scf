<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class ContributorModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("contributor", "Contributor");
  }

  // ------------------------------------------------------- template methods

  protected function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Name'),
      'has_body' => TRUE,
      'body_label' => t('Bio')
    );
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['contributor/createfor/%user'] = array(
      'title' => 'Create contributor for user',
      'page callback' => 'contributor_create_and_view',
      'page arguments' => array(2),
      'access arguments' => array('create contributors'),
      'type' => MENU_CALLBACK
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT cuid, capacity, affiliation, email FROM {contributor} WHERE vid = %d";
    $obj = db_fetch_object(db_query($sql, $node->vid));
    return $obj;
  }

  /****************************************************************************
   * @see hook_form()
   ****************************************************************************/
  public function form (&$node, &$form_state) {
    $type = $this->getNodeTypeInfo();
    $cuser = FALSE;
    $member = FALSE;
    // previous state either from existing node or from rebuilding of form from previous post
    $state = isset($form_state['values']) ? (object) $form_state['values'] : $node;
    $defaults = (object) array(
      'name' => '',
      'capacity' => '',
      'affiliation' => '',
      'email' => '',
      'bio' => '',
      'format' => FILTER_FORMAT_DEFAULT
    );
    $cuid = isset($state->cuid) ? $state->cuid : 0;
    if ($cuid > 1) {
      $cuser = user_load($cuid);
      if ($cuser) {
        $member = _member_get_node($cuser);
        $defaults->name = $member ? $member->title : $cuser->name;
        $defaults->email = $cuser->mail;
        if ($member) {
          $defaults->capacity = $member->jobtitle;
          $defaults->affiliation = $member->affiliation;
          $defaults->bio = $member->body;
          $defaults->format = $member->format;
        }
      }
    }
    $wt = -20;
    if (!$cuser) {
      $form['cuid'] = array(
        '#type' => 'select',
        '#options' => $this->cuidOptions(TRUE),
        '#default_value' => 0,
        '#description' => t('Select an existing user.  You may need to !create first.', array('!create' => l('create a new user', 'admin/user/user/create', array('attributes' => array('target' => 'new_user'))))),
        '#weight' => $wt++
      );
      $form['populate_user'] = array(
        '#type' => 'submit',
        '#value' => t('Populate'),
        '#submit' => array('contributor_populate_from_user'),
        '#weight' => $wt++
      );
    } else {
      $form['cuid_show'] = array(
        '#value' => t('<p>Contributor information for user \'!name\'.</p>', array('!name' => $defaults->name)),
        '#weight' => -20
      );
      $form['cuid'] = array(
        '#type' => 'value',
        '#value' => $cuid
      );
    }
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => check_plain($type->title_label),
      // not required here but will be by validate method
      '#required' => FALSE, 
      '#default_value' => !empty($state->title) ? $state->title : $defaults->name,
      '#weight' => $wt++
    );
    $form['capacity'] = array(
      '#type' => 'textfield',
      '#title' => t('Capacity, position or job title'),
      '#required' => FALSE,
      '#default_value' => !empty($state->capacity) ? $state->capacity : $defaults->capacity,
      '#weight' => $wt++
    );
    $form['affiliation'] = array(
      '#type' => 'textfield',
      '#title' => t('Affiliation'),
      '#required' => FALSE,
      '#default_value' => !empty($state->affiliation) ? $state->affiliation : $defaults->affiliation,
      '#weight' => $wt++
    );
    $form['email'] = array(
      '#type' => 'textfield',
      '#title' => t('Email'),
      '#required' => FALSE,
      '#default_value' => !empty($state->email) ? $state->email : $defaults->email,
      '#weight' => $wt++
    );
    
    if (empty($state->body)) {
      $state->body = $defaults->bio;
      $state->format = $defaults->format;
    }
    $this->addNodeBodyField($form, $state, $wt++);
    
    return $form;
  }

  /****************************************************************************
   * Submit callback for the "Populate" button
   ****************************************************************************/
  public function populateFromUser (&$form, &$form_state) {
    $form_state['rebuild'] = TRUE;
  }

  /****************************************************************************
   * @see hook_validate()
   ****************************************************************************/
  public function validate ($node) {
    //dvm($node);
    if (isset($node->op) && ($node->op == t('Populate'))) {
      // just populating the form based on a given user: do nothing
    } else if (empty($node->title)) {
      form_set_error('title', t("Name field is required"));
    }
  }

  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    $node = node_prepare($node, $teaser);
    if (module_exists('member')) {
      $mid = member_get_node_id($node->cuid);
      if ($mid) {
        $node->content['contributor_member'] = array(
          '#value' => l(t('View member profile'), 'node/' . $mid),
          '#weight' => 10
        );
      }
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
    return array(
      'contributor_embed' => array(
        'template' => 'contributor-embed',
        'arguments' => array('node' => NULL)
      )
    );
  }

  /**
   * @param $overrides includes values for any fields that should not
   * come from the user or user's member profile
   */
  public function createFromUser ($account, $overrides = array()) {
    global $user;
    $values = $overrides;
    $values['type'] = $this->name;
    $values['name'] = isset($user->name) ? $user->name : '';
    $values['language'] = '';
    if (is_object($account)) {
      $values['cuid'] = $account->uid;
    } else {
      $values['cuid'] = $account;
    }
    $form_state = array(
      'values' => $values
    );
    $form_state['values']['op'] = t('Save');
    module_load_include("inc", "node", "node.pages");
    // call drupal_execute with the form values filled in by the
    // proxy modules for this node type
    drupal_execute('contributor_node_form', $form_state, (object) $values);
    if (isset($form_state['nid']))
      return $form_state['nid'];
    else
      return FALSE;
  }


  // --------------------------------------- private

  private function cuidOptions ($membersOnly = FALSE) {
    $sql = "SELECT u.uid, IFNULL(m.sortname, u.name) as name";
    $sql .= " FROM {users} u";
    $sql .= " LEFT OUTER JOIN {member} m ON u.uid = m.muid";
    $sql .= " WHERE u.uid > 1";
    if ($membersOnly) {
      $sql .= " AND m.nid IS NOT NULL";
    }
    $sql .= " ORDER BY name";
    $result = db_query($sql);
    $options = array(0 => '<select>');
    while ($u = db_fetch_object($result)) {
      $options[$u->uid] = $u->name;
    }
    return $options;
  }


  // --------------------------------------- BOILERPLATE SINGLETON CODE

  private static $INSTANCE = NULL;

  // boilerplate: could move to superclass but would then 
  // need a map of instances
  public static function getInstance () {
    if (self::$INSTANCE === NULL) {
      self::$INSTANCE = new self;
    }
    return self::$INSTANCE;
  }


}

/**
 * Handy method to return the singleton instance.
 */
function contributor () {
  return ContributorModule::getInstance();
}

