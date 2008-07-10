<?php

require_once drupal_get_path('module', 'scf') . '/AhahFormRows.php';

// ------------------------------------------------------ Classes

class ParticipantRows extends AhahFormRows {
  
  public function __construct () {
    parent::__construct('interview', 'participant');
  }

  public function blankRowValuesArray () {
    static $blanks;
    if (!isset($blanks)) {
      $blanks = array(
        'mid' => 0,
        'cid' => 0,
        'interviewer' => 0,
        'label' => ''
      );
      if (SCF_AUTHOR_AUTOCOMPLETE) {
        $blanks['name'] = '';
      }
    }
    return $blanks;
  }

  /**
   * Create the "participants" subgroup of the form
   */
  public function defineFormRows (&$node, &$form_state) {
    if (isset($form_state['participant_count'])) {
      $participant_count = $form_state['participant_count'];
    } else {
      $participant_count = max(2, empty($node->participants) ? 2 : count($node->participants));
    }
    
    // Container for just the interview participants.
    $form = array(
      '#prefix' => '<div id="interview-participants">',
      '#suffix' => '</div>',
      '#theme' => 'interview_form_participants'
    );
    
    // Add the current participants to the form.
    for ($i = 0; $i < $participant_count; $i++) {
      $vals = $this->blankRowValuesArray();
      if (isset($node->participants[$i])) {
        $ppt = (object) $node->participants[$i];
        if (!empty($ppt)) {
          $contrib = FALSE;
          if (!empty($ppt->cid)) {
            $contrib = node_load($ppt->cid);
          } else if (!empty($ppt->mid)) {
            $mem = node_load($ppt->mid);
            if ($mem) {
              // HACK HACK!!!!  This is not side-effect free: you can
              // create contributor nodes even on previewing an
              // interview....
              $cid = contributor_create_from_user($mem->muid);
              if ($cid) {
                // save in node so can be used for statement options on preview
                $node->participants[$i]['cid'] = $cid;
                $contrib = node_load($cid);
              }
            }
          }
          if ($contrib) {
            $vals = array(
              'cid' => $contrib->nid,
              'mid' => 0,
              'interviewer' => $ppt->interviewer,
              'label' => $ppt->label
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
   * Create a single row in the "participants" subgroup of the form
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
        '#title' => t('Participant'),
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
        '#title' => t('Participant'),
        '#options' => $this->authorOptions()
      );
    }
    
    $form['interviewer'] = array(
      '#type' => 'checkbox',
      '#title' => t('Is interviewer?'),
      );
    
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Key'),
      '#size' => 8,
      '#maxlength' => 8,
      );
    
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => '-',
      '#description' => t("Remove this participant."),
      '#submit' => array('interview_delete_participant_submit'), // If no javascript action.
      '#ahah' => array(
        // path set separately based on row index
        // 'path' => 'interview/js/participant/delete/' . $i,
        'wrapper' => 'interview-participants',
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
    // their values appear in the $form_state['values']['participants'] array.
    foreach ($rowForm as $field => $value) {
      if (is_array($rowForm[$field])) {
        $rowForm[$field]['#parents'] = array('participants', $idx, $field);
      }
    }
    $rowForm['delete']['#ahah']['path'] = 'interview/js/participant/delete/' . $idx;
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


class StatementRows extends AhahFormRows {
  
  public function __construct () {
    parent::__construct('interview', 'statement');
  }

  public function blankRowValuesArray () {
    static $blanks;
    if (!isset($blanks)) {
      $blanks = array(
        'cid' => 0,
        'statement' => '',
        'statement_format' => filter_resolve_format(FILTER_FORMAT_DEFAULT),
        'image' => ''
      );
    }
    return $blanks;
  }

  /**
   * Create the "interview content" subgroup of the form
   */
  public function defineFormRows (&$node, &$form_state) {
    if (isset($form_state['statement_count'])) {
      $statement_count = $form_state['statement_count'];
    } else {
      $statement_count = max(1, empty($node->statements) ? 1 : count($node->statements));
    }
    
    // Container for just the interview statements.
    $form = array(
      '#prefix' => '<div id="interview-statements">',
      '#suffix' => '</div>',
      );
    
    $ppts = $this->participantSelectorOptionsFromNode($node);
    $ppt_options = $ppts;
    if (empty($ppt_options)) {
      $ppt_options = array('' => ' -- none -- ');
    } else {
      $ppt_options = array('' => '') + $ppt_options;
    }
    
    // Add the current statements to the form.
    for ($i = 0; $i < $statement_count; $i++) {
      $vals = $this->blankRowValuesArray();
      if (isset($node->statements[$i])) {
        $st = (object) $node->statements[$i];
        if (!empty($st)) {
          if (!empty($st->cid) && isset($ppts[$st->cid])) {
            $vals['cid'] = $st->cid;
          }
          if (!empty($st->statement)) {
            $vals['statement'] = $st->statement;
          }
          if (!empty($st->statement_format)) {
            $vals['statement_format'] = $st->statement_format;
          }
          if (!empty($st->image)) {
            $vals['image'] = $st->image;
          }
        }
      }
      $row = $this->defineFormRow();
      $this->setFormRowValues($row, $vals);
      $this->setRowIndex($row, $i);
      $row['cid']['#options'] = $ppt_options;
      $form[$i] = $row;
    }
    
    return $form;
  }

  /**
   * Create a single row in the "statements" subgroup of the form
   */
  function defineFormRow () {
    $wt = 0;
    
    $form = array(
      '#tree' => TRUE,
      );
    
    $form['cid'] = array(
      '#type' => 'select',
      '#title' => t('Particpant'),
      '#weight' => $wt++
    );
    
    $form['spec_chars'] = array(
      '#value' => l(t('Special characters'), ''),
      '#weight' => $wt++
    );
    
    $form['statement'] = array(
      '#type' => 'textarea',
      '#title' => t('Particpant statement'),
      '#weight' => $wt++
    );
    
    $form['statement_format'] = filter_form(FILTER_FORMAT_DEFAULT, $wt++);
    
    $form['image'] = array(
      '#type' => 'textfield',
      '#title' => t('Image'),
      '#weight' => $wt++
    );
    
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => '-',
      '#description' => t("Remove this statement."),
      '#submit' => array('interview_delete_statement_submit'), // If no javascript action.
      '#ahah' => array(
        // 'path' => 'interview/js/statement/delete/' . $i,
        'wrapper' => 'interview-statements',
        'method' => 'replace',
        'progress' => 'none',
        'effect' => 'fade',
        ),
      '#weight' => $wt++
    );
    
    $form['add'] = array(
      '#type' => 'submit',
      '#value' => '+',
      '#description' => t("Add another statement."),
      '#submit' => array('interview_add_statement_submit'), // If no javascript action.
      '#ahah' => array(
        // 'path' => 'interview/js/statement/add/' . $i,
        'wrapper' => 'interview-statements',
        'method' => 'replace',
        'progress' => 'none',
        'effect' => 'fade',
        ),
      '#weight' => $wt++
    );
    
    return $form;
  }


  protected function setRowIndex (&$rowForm, $idx) {
    // Manually set the #parents property of these fields so that
    // their values appear in the $form_state['values']['statements'] array.
    foreach ($rowForm as $field => $value) {
      if (is_array($value) && isset($value['#type'])) {
        // HACK to handle filters fieldset
        if ($value['#type'] == 'fieldset') {
          foreach ($rowForm[$field] as $subfield => $subvalue) {
            if (is_array($subvalue) && isset($subvalue['#type'])) {
              $rowForm[$field][$subfield]['#parents'] = array('statements', $idx, $field);
            }
          }
        } else {
          $rowForm[$field]['#parents'] = array('statements', $idx, $field);
        }
      }
    }
    $rowForm['delete']['#ahah']['path'] = 'interview/js/statement/delete/' . $idx;
    $rowForm['add']['#ahah']['path'] = 'interview/js/statement/add/' . ($idx + 1);
  }

  protected function setReferenceData (&$rowForm, $form) {
    $ppts = $this->participantSelectorOptionsFromForm($form);
    if (empty($ppts)) {
      $ppts = array('' => ' -- none -- ');
    } else {
      $ppts = array('' => '') + $ppts;
    }
    $rowForm['cid']['#options'] = $ppts;
  }

  /**
   * return an array of cid => participant name
   */
  private function participantSelectorOptionsFromForm ($form) {
    $ppts = array();
    if (isset($form['participant_wrapper']['participants'])) {
      foreach ($form['participant_wrapper']['participants'] as $ppt) {
        if (!empty($ppt) && !empty($ppt['cid']['#default_value'])) {
          $contrib = node_load($ppt['cid']['#default_value']);
          if ($contrib) {
            $ppts[$contrib->nid] = $contrib->title;
          }
        }
      }
    }
    return $ppts;
  }

  /**
   * return an array of cid => participant_name
   */
  private function participantSelectorOptionsFromNode ($node) {
    $ppts = array();
    if (isset($node->participants)) {
      foreach ($node->participants as $ppt) {
        $ppt = (object) $ppt;
        if (!empty($ppt) && !empty($ppt->cid)) {
          $contrib = node_load($ppt->cid);
          $ppts[$contrib->nid] = $contrib->title;
        }
      }
    }
    return $ppts;
  }

}

// ------------------------------------------------------ Callbacks

/**
 * Menu callback for AHAH "add participant" (always adds at the end)
 */
function interview_js_participant_add () {
  $rows = new ParticipantRows();
  $rows->insertRow();
}

/**
 * Menu callback for AHAH "delete participant"
 */
function interview_js_participant_delete ($pos = 0) {
  $rows = new ParticipantRows();
  $rows->deleteRow($pos);
}


/**
 * Menu callback for AHAH "add statement"
 * 
 * @param $pos
 *   Insert new statement at the $pos'th position (0-based), moving 
 *   all following statements down.
 */
function interview_js_statement_add ($pos = -1) {
  $rows = new StatementRows();
  $rows->insertRow($pos);
}

/**
 * Menu callback for AHAH "delete statement"
 */
function interview_js_statement_delete ($pos = 0) {
  $rows = new StatementRows();
  $rows->deleteRow($pos);
}
