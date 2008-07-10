<?php

require_once drupal_get_path('module', 'scf') . '/AhahFormRows.php';

// ------------------------------------------------------ Classes

class ContributorRows extends AhahFormRows {
  
  public function __construct () {
    parent::__construct('pubnode', 'contributor');
  }

  public function blankRowValuesArray () {
    static $blanks;
    if (!isset($blanks)) {
      $blanks = array(
        'mid' => 0,
        'cid' => 0
      );
      if (SCF_AUTHOR_AUTOCOMPLETE) {
        $blanks['name'] = '';
      }
    }
    return $blanks;
  }

  /**
   * Create the "contributors" subgroup of the form
   */
  public function defineFormRows (&$node, &$form_state) {
    if (isset($form_state['contributor_count'])) {
      $contributor_count = $form_state['contributor_count'];
    } else {
      $contributor_count = max(1, empty($node->contributors) ? 1 : count($node->contributors));
    }
    
    // Container for just the pubnode contributors.
    $form = array(
      '#prefix' => '<div id="pubnode-contributors">',
      '#suffix' => '</div>',
      '#theme' => 'pubnode_form_contributors'
    );
    
    // Add the current contributors to the form.
    for ($i = 0; $i < $contributor_count; $i++) {
      $vals = $this->blankRowValuesArray();
      if (isset($node->contributors[$i])) {
        $auth = (object) $node->contributors[$i];
        if (!empty($auth)) {
          $contrib = FALSE;
          if (!empty($auth->cid)) {
            $contrib = node_load($auth->cid);
          } else if (!empty($auth->mid)) {
            $mem = node_load($auth->mid);
            if ($mem) {
              // HACK HACK!!!!  This is not side-effect free: you can
              // create contributor nodes even on previewing an
              // pubnode....
              $cid = contributor_create_from_user($mem->muid);
              if ($cid) {
                // save in node so can be used for statement options on preview
                $node->contributors[$i]['cid'] = $cid;
                $contrib = node_load($cid);
              }
            }
          }
          if ($contrib) {
            $vals = array(
              'cid' => $contrib->nid,
              'mid' => 0
            );
            if (SCF_AUTHOR_AUTOCOMPLETE) {
              $vals['name'] = $contrib->title;
            }
          }
        }
      }
      $row = $this->defineFormRow();
      $this->setFormRowValues($row, $vals);
      $this->setRowIndex($row, $i);
      $form[$i] = $row;
    }
    
    return $form;
  }
  
  /**
   * Create a single row in the "contributors" subgroup of the form
   */
  protected function defineFormRow () {
    $form = array(
      '#tree' => TRUE,
      );
    
    if (SCF_AUTHOR_AUTOCOMPLETE) {
      $form['cid'] = array(
        '#type' => 'hidden',
        );
      
      $form['mid'] = array(
        '#type' => 'hidden',
        );
      
      $form['name'] = array(
        '#type' => 'textfield',
        '#title' => t('Contributor'),
        '#autocomplete_path' => 'member/autocomplete/name',
        '#size' => 32
      );

    } else {
      $form['cid'] = array(
        '#type' => 'hidden'
      );
      $form['mid_sav'] = array(
        '#type' => 'value'
      );

      $form['mid'] = array(
        '#type' => 'select',
        '#title' => t('Contributor'),
        '#options' => $this->authorOptions()
      );
    }
    
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => '-',
      '#description' => t("Remove this contributor."),
      '#submit' => array('pubnode_delete_contributor_submit'), // If no javascript action.
      '#ahah' => array(
        // path set separately based on row index
        // 'path' => 'pubnode/js/contributor/delete/' . $i,
        'wrapper' => 'pubnode-contributors',
        'method' => 'replace',
        'progress' => 'none',
        'effect' => 'fade',
        ),
      '#weight' => 1
    );
    
    return $form;
  }

  protected function setRowIndex (&$rowForm, $idx) {
    // Manually set the #parents property of these fields so that
    // their values appear in the $form_state['values']['contributors'] array.
    foreach ($rowForm as $field => $value) {
      if (is_array($rowForm[$field])) {
        $rowForm[$field]['#parents'] = array('contributors', $idx, $field);
      }
    }
    $rowForm['delete']['#ahah']['path'] = 'pubnode/js/contributor/delete/' . $idx;
  }

  protected function authorOptions () {
    static $authors;
    if (!isset($authors)) {
      $authors = array();
      $args = array();
      $rid = scf_author_rid();
      $checkrole = ($rid != DRUPAL_AUTHENTICATED_RID);
      $sql = "SELECT m.nid, m.sortname FROM {member} m";
      $sql .= " JOIN {node} n ON n.vid = m.vid";
      if ($checkrole) {
        $sql .= " JOIN {users} u ON m.muid = u.uid";
        $sql .= " JOIN {users_roles} ur ON u.uid = ur.uid";
      }
      $sql .= " WHERE n.status = 1";
      if ($checkrole) {
        $sql .= " AND ur.rid = %d";
        $args[] = $rid;
      }
      $sql .= " ORDER BY m.sortname ASC";
      $sql = db_rewrite_sql($sql);
      $result = db_query($sql, $args);
      while ($row = db_fetch_array($result)) {
        $authors[$row['nid']] = $row['sortname'];
      }
    }
    return $authors;
  }

}

// ------------------------------------------------------ Callbacks

/**
 * Menu callback for AHAH "add contributor" (always adds at the end)
 */
function pubnode_js_contributor_add () {
  $rows = new ContributorRows();
  $rows->insertRow();
}

/**
 * Menu callback for AHAH "delete contributor"
 */
function pubnode_js_contributor_delete ($pos = 0) {
  $rows = new ContributorRows();
  $rows->deleteRow($pos);
}


