<?php

/**
 * Base class for SCF node modules
 */
abstract class ScfNodeModule extends OONodeModule {
  
  protected function __construct ($name, $displayName = NULL, $permName = NULL) {
    parent::__construct($name, $displayName, $permName);
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * scf's admin_settings_subform hook
   * 
   * default impl just returns print-friendly checkbox
   ****************************************************************************/
  public function adminSettingsSubform () {
    $form = array();
    $this->adminSettingsPrintFriendlyForm($form);
    return $form;
  }

  /**
   * Returns the valid themes for SCF nodes
   *
   * Node list template theme added by MAG 6/3/2008
   * @return array An array of the valid themes
   */
  public function theme() {
  	return array(
  		'node_list' => array(
  			'template' => 'node-list',
  			'path' => drupal_get_path('module', 'scf'),
  			'arguments' => array(
  				'items' => array(),
  				'title' => NULL,
  				'nid' => NULL,
  				'haslink' => NULL,
  				'associations' => array(),
  	      'name' => NULL,
  	      'form' => NULL,
  			)	
  		)
  	);
  }
  
  /****************************************************************************
   * @see hook_block()
   ****************************************************************************/
  public function block ($op = 'list', $delta = 'listing', $edit = array()) {
    if ($op == 'list') {
      $blocks = parent::block('list');
      $blocks['featured'] = array(
        'info' => t('Featured !names', array('!names' => $this->displayNamePlural)),
        'weight' => 0,
        'status' => 0,
        'cache' => BLOCK_NO_CACHE
      );
      $blocks['unpublished'] = array(
        'info' => t('Unpublished !names', array('!names' => $this->displayNamePlural)),
        'weight' => 0,
        'status' => 0,
        'cache' => BLOCK_NO_CACHE
      );
      if (module_exists('association')) {
        $blocks['associated'] = array(
          'info' => t('Associated !names', array('!names' => $this->displayNamePlural)),
          'weight' => 0,
          'status' => 0,
          'cache' => BLOCK_NO_CACHE
        );
      }
      return $blocks;
    } else if ($op == 'view') {
      if ($delta == 'featured') {
        return $this->featuredBlock();
      } else if ($delta == 'unpublished') {
        return $this->unpublishedBlock();
      } else if ($delta == 'associated') {
        return $this->associatedBlock();
      } else {
        return parent::block('view', $delta);
      }
    }
  }


  public function featuredBlock () {
    // make this configurable...
    $limit = 2;
    $proto = array(
      'status' => 1,
      'sticky' => 1
    );
    $out = $this->listInternal($limit, $proto);
    $n = count($out);
    return array(
      'subject' => t("Featured %names", array("%names" => t(($n > 1) ? $this->displayNamePlural : $this->displayName))),
      'content' => implode('', $out)
    );
  }    
  

  public function unpublishedBlock () {
    // make this configurable...
    $limit = 2;
    $proto = array(
      'status' => 0
    );
    $out = $this->listInternal($limit, $proto);
    // dvm($out);
    $n = count($out);
    return array(
      'subject' => t("Unpublished %names", array("%names" => t(($n > 1) ? $this->displayNamePlural : $this->displayName))),
      'content' => implode('', $out)
    );
  }    
  

  public function associatedBlock () {
    // abort if we're not doing a node 'view' page
    if (!iic_util_current_page_is_node_view())
      return;
    $node = iic_util_current_page_node();
    if ($node && association_check_type($node, $this->name)) {
      association_populate_node_associations($node);
      association_populate_node_associations($node, $this->name);
      $field = $this->name . '_associations';
      $nodes = $node->$field;
      if (count($nodes)) {
        $links = array();
        foreach ($nodes as $n) {
          $links[] = l($n->title, 'node/'. $n->nid, array());
        }
        return array(
          'subject' => t('Associated !names', array('!names' => $this->displayNamePlural)),
          'content' => theme( // variables added by MAG 6/3/2008
            'node_list',
            $links,
            '',
            $node->nid,
            user_access('edit own ' . $this->name),
            $nodes,
            $this->name,
            drupal_get_form('scfnode_form_add_association', $node->nid, $this->name)
          )
        );
      }
    }
  }    
  

  public function listRecentlyAssociated ($limit = 10) {
    $out = array();
    if (module_exists("association")) {
      $sql = "SELECT n.nid, a.last_association_timestamp FROM {node} n"
        . " JOIN {association_statistics} a ON n.nid = a.nid"
        . " WHERE type = '%s' AND status = 1"
        . " ORDER BY a.last_association_timestamp DESC";
      $sql = db_rewrite_sql($sql);
      $result = db_query_range($sql, $this->name, 0, $limit);
      while ($row = db_fetch_array($result)) {
        $out[$row['nid']] = $row['last_association_timestamp'];
      }
    }
    return $out;
  }

  public function loadRecentNodes ($limit = 10, $element = 0) {
    $assoc = $this->associatesWith();
    $nodes = array();
    $sql = "SELECT";
    $sql .= " n.nid,";
    $sql .= " IF(n.changed < c.last_comment_timestamp,";
    if ($assoc) {
      $sql .= " IF(IFNULL(a.last_association_timestamp, 0) > c.last_comment_timestamp, a.last_association_timestamp, c.last_comment_timestamp),";
      $sql .= " IF(IFNULL(a.last_association_timestamp, 0) > n.changed, a.last_association_timestamp, n.changed)) AS t ";
    } else {
      $sql .= " n.changed, c.last_comment_timestamp) AS t ";
    }
    $sql .= "FROM {node} n JOIN {node_comment_statistics} c ON n.nid = c.nid ";
    if ($assoc) {
      $sql .= " LEFT OUTER JOIN {association_statistics} a ON n.nid = a.nid ";
    }
    $sql .= "WHERE type = '%s' AND status = 1 ";
    $sql .= "ORDER BY";
    $sql .= " t DESC";
    $sql = db_rewrite_sql($sql);
    $result = pager_query($sql, $limit, $element, NULL, $this->name);
    while ($row = db_fetch_array($result)) {
      $nodes[] = node_load($row['nid']);
    }
    return array($nodes, theme('pager', NULL, $limit, $element));
  }

  /**
   * $node must be of the module's native type, not just any node
   */
  public function loadAssociated (&$node, $type = NULL) {
    $field = ((isset($type) && $type != '*') ? $type : 'node') . '_associations';
    if ($this->associatesWith($type)) {
      association_populate_node_associations($node, $type);
    } else {
      $node->field = array();
    }
    return $node->$field;
  }


  // ------------------------------------------------------- template methods

  protected function adminSettingsPrintFriendlyForm (&$form) {
    if (module_exists('print')) {
      $showPrintVar = 'print_display_' . $this->name;
      $form[$showPrintVar] = array(
        '#type' => 'checkbox',
        '#title' => t('Display print-friendly link on !type pages.', array('!type' => t($this->displayName))),
        '#default_value' => variable_get($showPrintVar, 1)
      );
    }
  }


  // ------------------------------------------------------- utility

  public function addAssociatedTypes () {
    $types = func_get_args();
    // this is special wildcard, means any type
    if (in_array('*', $types)) {
      variable_set($this->name . '_associates_with', array('*'));
    } else {
      $cur = variable_get($this->name . '_associates_with', array());
      $added = FALSE;
      foreach ($types as $type) {
        if (is_string($type) && !in_array($type, $cur)) {
          $cur[] = $type;
          $added = TRUE;
        }
      }
      if ($added) {
        variable_set($this->name . '_associates_with', $cur);
      }
    }
  }

  /**
   * check to see whether associations are enabled and nodes of this
   * type participate in associations with nodes of type $type (or
   * with any type if $type is NULL)
   */
  public function associatesWith ($type = NULL) {
    if (module_exists("association"))
      return association_check_type($this->name, $type);
    else
      return FALSE;
  }

}

  /**
   * Implementation of hook_form for the field to add a new association
   *
   * Added by MAG 6/3/2008
   * @param FormState $form_state The drupal form state
   * @param int $nid The ID of the node (article, interview, etc.) that this form will be adding to
   * @param string $name The name of the node type that this form represents (gene, researchstatement, etc.)
   * @return array An array to be used by drupal_get_form
   */
function scfnode_form_add_association(&$form_state, $nid, $name) {
    $form = array();
    /*$form['cancel'] = array(
      '#type' => 'image_button',
      '#src' => drupal_get_path('module', 'scf') . '/images/icon-x.gif',
      '#attributes' => array(
        'onclick' => "$('#association-addtermdiv-" . $name . "').slideUp('slow');return false;"
      ),
      '#executes_submit_callback' => FALSE,
      '#title' => 'Close'
    );*/
    
    /*$form['caption'] = array(
      '#type' => 'markup',
      '#value' => t('Add new term:')
    );*/
    
    $form['textfield'] = array(
      '#type' => 'textfield',
      '#autocomplete_path' => $name . '/autocomplete/title',
      '#size' => 20,
      '#id' => 'association-' . $name . '-text',
      '#name' => 'textfield',
    );
    $form['add'] = array(
      '#type' => 'button',
      '#value' => t('Add'),
      '#ahah' => array(
        'path' => 'association/ajax/add/' . $nid . '/' . $name . '/',
        'wrapper' => 'association-list-' . $name,
        'event' => 'click',
        'effect' => 'slide',
        'method' => 'append',
        'progress' => 'none',
      ),
      '#executes_submit_callback' => FALSE
    );
  
    $form['nid'] = array(
      '#type' => 'value',
      '#value' => $nid
    );
    return $form;
  }


