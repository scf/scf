<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class InterviewModule extends ScfNodeModule {
  
  protected function __construct () {
    parent::__construct("interview", "Interview");
    $this->childTables = array("interview_participants", "interview_statements");
  }

  // ------------------------------------------------------- template methods

  public function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Title'),
      'description' => t('Interviews'),
      'has_body' => TRUE,
      'body_label' => t('Abstract'),
      'locked' => TRUE
    );
  }

  public function insertChildren (&$node) {
    $this->saveAuthorContrib($node);
    $this->insertParticipants($node);
    $this->insertStatements($node);
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['interview/js/participant/add'] = array(
      'title' => 'Javascript Add Participant Form',
      'page callback' => 'interview_js_participant_add',
      'access callback' => 'user_access',
      'access arguments' => array('edit own interviews'),
      'file' => 'interview.ahah.inc',
      'type' => MENU_CALLBACK
    );
    $items['interview/js/participant/delete/%'] = array(
      'title' => 'Javascript Delete Participant Form',
      'page callback' => 'interview_js_participant_delete',
      'page arguments' => array(4),
      'access callback' => 'user_access',
      'access arguments' => array('edit own interviews'),
      'file' => 'interview.ahah.inc',
      'type' => MENU_CALLBACK
    );
    $items['interview/js/statement/add/%'] = array(
      'title' => 'Javascript Add Statement Form',
      'page callback' => 'interview_js_statement_add',
      'page arguments' => array(4),
      'access callback' => 'user_access',
      'access arguments' => array('edit own interviews'),
      'file' => 'interview.ahah.inc',
      'type' => MENU_CALLBACK
    );
    $items['interview/js/statement/delete/%'] = array(
      'title' => 'Javascript Delete Statement Form',
      'page callback' => 'interview_js_statement_delete',
      'page arguments' => array(4),
      'access callback' => 'user_access',
      'access arguments' => array('edit own interviews'),
      'file' => 'interview.ahah.inc',
      'type' => MENU_CALLBACK
    );
    return $items;
  }


  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT author_cid, pubdate FROM {interview} WHERE vid = %d";
    $interview = db_fetch_object(db_query($sql, $node->vid));
    $ppts = $this->loadParticipants($node);
    $stmts = $this->loadStatements($node);
    if (isset($interview->author_cid) && $interview->author_cid > 0) {
      $contrib = node_load($interview->author_cid);
      if ($contrib) {
        $interview->author_contributor = $contrib;
        $interview->author_name = $contrib->title;
        $interview->author_capacity = $contrib->capacity;
      }
    }
    $interview->participants = $ppts;
    $interview->statements = $stmts;
    return $interview;
  }


  /****************************************************************************
   * @see hook_form()
   ****************************************************************************/
  public function form (&$node, &$form_state) {
    $type = $this->getNodeTypeInfo();
    
    // cache form so cached version can be manipulated in ahah callbacks
    $form = array(
      '#cache' => TRUE,
      );
    
    $wt = -99;
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => check_plain($type->title_label),
      '#required' => TRUE,
      '#default_value' => $node->title,
      '#weight' => $wt++
    );
    
    $form['pubdate'] = array(
      '#type' => 'value',
      '#value' => isset($node->pubdate) ? $node->pubdate : 0
    );
    
    $form['author_cid'] = array(
      '#type' => 'value',
      '#value' => isset($node->author_cid) ? $node->author_cid : 0
    );
    
    // this will normally be 0 unless a new member lookup occurs, in
    // which case a mid is written by the member autocomplete
    // javascript
    $form['author_mid'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($node->author_mid) ? $node->author_mid : 0
    );
    
    $form['author_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Author name'),
      '#default_value' => isset($node->author_name) ? $node->author_name : '',
      '#autocomplete_path' => 'member/autocomplete/name',
      '#size' => 32,
      '#weight' => $wt++
    );
    
    $form['author_capacity'] = array(
      '#type' => 'textfield',
      '#title' => t('Author title'),
      '#default_value' => isset($node->author_capacity) ? $node->author_capacity : '',
      '#description' => t('For example, "Staff Editor," "Senior Correspondent".'),
      '#size' => 64,
      '#maxlength' => 128,
      '#weight' => $wt++
    );
    
    $this->addNodeBodyField($form, $node, $wt++);
    
    // Add a wrapper for the participants and more button.
    $form['participant_wrapper'] = array(
      '#tree' => FALSE,
      '#type' => 'fieldset',
      '#title' => t('Participants'),
      '#description' => t('Identify the site members which participated in this interview.'),
      '#collapsible' => TRUE,
      '#weight' => $wt++
    );
    
    $prh = $this->getParticipantRowsHelper();
    $form['participant_wrapper']['participants'] = $prh->defineFormRows($node, $form_state);
    
    // We name our button 'interview_add' to avoid conflicts with other modules using
    // AHAH-enabled buttons with the id 'more' (since #tree is false for participant_wrapper)
    
    $form['participant_wrapper']['interview_add_participant'] = array(
      '#type' => 'submit',
      '#value' => t('More participants'),
      '#description' => t("If the amount of boxes above isn't enough, click here to add more participants."),
      '#submit' => array('interview_add_participant_submit'), // If no javascript action.
      '#ahah' => array(
        'path' => 'interview/js/participant/add',
        'wrapper' => 'interview-participants',
        'method' => 'replace',
        'effect' => 'fade',
        'progress' => 'none',
        ),
      '#weight' => 1
    );
    
    // Add a wrapper for the statements and more button.
    $form['statement_wrapper'] = array(
      '#tree' => FALSE,
      '#type' => 'fieldset',
      '#title' => t('Interview content'),
      '#description' => t('This is a list of statements made by the interview participants.'),
      '#collapsible' => TRUE,
      '#weight' => $wt++
    );
    
    $srh = $this->getStatementRowsHelper();
    $form['statement_wrapper']['statements'] = $srh->defineFormRows($node, $form_state);
    
    return $form;
  }

  /****************************************************************************
   * @see hook_validate()
   *
   * This is REALLY screwy, because both members and contribs are at play...
   ****************************************************************************/
  public function validate ($node) {
    if (isset($node->title)) {
      // Check for at least two options and validate amount of votes:
      $tally = 0;
      $interviewers = 0;
      // Renumber fields
      $ppts = array_values($node->participants);
      $uniq = array();
      foreach ($ppts as $i => $ppt) {
        $good = TRUE;
        $cid = isset($ppt['cid']) ? $ppt['cid'] : 0;
        $mid = isset($ppt['mid']) ? $ppt['mid'] : 0;
        if (!$cid) {
          $good = FALSE;
        }
        if ($good && $cid < 1) {
          $good = FALSE;
          form_set_error("participant][$i][cid", t('Illegal participant.'));
        }
        if (!$good) {
          if ($mid > 1) {
            $good = TRUE;
          }
        }
        if ($good && SCF_AUTHOR_AUTOCOMPLETE && empty($ppt['name'])) {
          $good = FALSE;
          form_set_error("participant][$i][name", t('Participant must have name.'));
        }
        if ($good && empty($ppt['label'])) {
          $good = FALSE;
          form_set_error("participant][$i][label", t('Participant must have key.'));
        }
        if ($good && !empty($ppt['interviewer']) && $ppt['interviewer'] > 0) {
          $interviewers++;
        }
        if ($good) {
          if ($cid > 1) {
            $contrib = node_load($cid);
            if ($contrib)
              $uid = $contrib->cuid;
          } else if ($mid > 1) {
            $member = node_load($mid);
            if ($member) {
              $uid = $member->muid;
            }
          }
          if (isset($uid)) {
            if (isset($uniq[$uid])) {
              form_set_error("participant][$i][cid", t('Duplicate participant.'));
            }
            $uniq[$uid] = TRUE;
          }
        }
      }
      
      $tally = count($uniq);
      if ($tally < 2) {
        form_set_error("participant", t('You must fill in at least two participants.'));
      }
      if ($interviewers == 0) {
        form_set_error("participant", t('You must specifiy at least one interviewer.'));
      } elseif ($interviewers == $tally) {
        form_set_error("participant", t('You must specifiy at least one interviewee.'));
      }
    }
  }

  /****************************************************************************
   * @see hook_node_form_submit()
   ****************************************************************************/
  public function nodeFormSubmit (&$form, &$form_state) {
    // Renumber fields
    $form_state['values']['participants'] = array_values($form_state['values']['participants']);
    // set pubdate as soon as it goes live
    if (empty($form_state['values']['pubdate']) && $form_state['values']['status']) {
      $form_state['values']['pubdate'] = time();
    }
  }

  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    $this->addCss();
    $node = node_prepare($node, $teaser);
    $node->content['byline'] = array(
      '#value' => theme('interview_byline', $node),
      '#weight' => -3
    );
    if (!$teaser) {
      $interviewers = array();
      $ppts = array();
      foreach ($node->participants as $ppt) {
        $ppt = (object) $ppt;
        if (isset($ppt->cid)) {
          $contrib = node_load($ppt->cid);
          if ($contrib) {
            if ($ppt->interviewer) {
              $interviewers[] = theme('contributor_embed', $contrib);
            } else {
              $ppts[] = theme('contributor_embed', $contrib);
            }
          }
        }
      }
      $node->content['interviewers'] = array(
        '#value' => implode('', $interviewers),
        '#weight' => 1
      );
      $node->content['participants'] = array(
        '#value' => implode('', $ppts),
        '#weight' => 2
      );
      $statements = '';
      $ppt_lookup = $this->nonemptyParticipantsArray($node);
      for ($i = 0; $i < count($node->statements); $i++) {
        $statements .= theme('interview_statement', $node, $i, $ppt_lookup);
      }
      $node->content['statements'] = array(
        '#value' => $statements,
        '#weight' => 3
      );
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_nodeapi()
   ****************************************************************************/
  public function nodeapi (&$node, $op, $a3 = NULL, $a4 = NULL) {
    switch ($op) {
      case 'view':
        // $a3 == $teaser
        if (!$a3 && $node->type == 'member') {
          $contribs = $this->listMemberContributions($node->muid);
          if (!empty($contribs)) {
            if (!isset($node->content['contributions'])) {
              $node->content['contributions'] = array(
                '#value' => '',
                '#weight' => 4
              );
            }
            $node->content['contributions']['#value'] .= theme('member_contribs', t('Interview'), t('Interviews'), $contribs);
          }
        }
        break;
      default:
        return parent::nodeapi($node, $op, $a3, $a4);
    }
  }

  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
    return array(
      'interview_node_form' => array(
        'arguments' => array('form' => NULL)
      ),
      'interview_form_participants' => array(
        'arguments' => array('form' => NULL)
      ),
      'interview_statement' => array(
        'template' => 'interview-statement',
        'arguments' => array(
          'node' => NULL,
          'statement_num' => 0,
          'participant_lookup' => array() // array of cid => ppt
        )
      ),
      'interview_byline' => array(
        'template' => 'interview-byline',
        'arguments' => array(
          'node' => NULL,
          )
      )
    );
  }

  // ------------------------------------------------------- internal methods

  private function listMemberContributions ($uid) {
    $sql = "SELECT DISTINCT n.nid FROM {node} n";
    $sql .= " INNER JOIN {interview} i ON i.vid = n.vid";
    $sql .= " INNER JOIN {interview_participants} ip ON ip.vid = i.vid";
    $sql .= " INNER JOIN {contributor} c ON ip.cid = c.nid";
    $sql .= " INNER JOIN {users} u ON c.cuid = u.uid";
    $sql .= " WHERE n.status = 1";
    $sql .= " AND u.uid = %d";
    $sql .= " ORDER BY i.pubdate ASC";
    $result = db_query($sql, $uid);
    $out = array();
    while ($nid = db_result($result)) {
      $node = node_load($nid);
      $item = l($node->title, 'node/'. $node->nid,
        !empty($node->comment_count) ? array('title' => format_plural($node->comment_count, '1 comment', '@count comments')) : array());
      if (isset($node->pubdate) && $node->pubdate) {
        $item .= " (" . scf_date_string($node->pubdate) . ")";
      }
      $out[] = $item;
    }
    return $out;
  }

  private function insertParticipants ($node) {
    foreach ($node->participants as $i => $ppt) {
      $ppt = (object) $ppt;
      $cid = isset($ppt->cid) ? $ppt->cid : 0;
      $mid = isset($ppt->mid) ? $ppt->mid : 0;
      if (!$cid && $mid) {
        $mem = node_load($mid);
        if ($mem) {
          $cid = contributor_create_from_user($mem->muid);
          $node->participants[$i]['cid'] = $cid;
        }
      }
    }
    $this->insertIndexedChildren($node, $node->participants, "interview_participants", "cid");
  }
  
  private function insertStatements ($node) {
    $this->insertIndexedChildren($node, $node->statements, "interview_statements");
  }

  /**
   * Insert or update contributor for each revision of an Interviewer
   */
  private function saveAuthorContrib (&$node) {
    if (isset($node->author_mid) && $node->author_mid > 0) {
      $member = node_load($node->author_mid);
      if ($member) {
        $muser = user_load($member->muid);
        $contrib = array(
          'capacity' => $node->author_capacity
        );
        $cid = contributor_create_from_user($muser, $contrib);
        if ($cid) {
          $node->author_cid = $cid;
        }
      }
    } else if (isset($node->author_cid) && $node->author_cid > 0) {
      $contrib = node_load($node->author_cid);
      $contrib->title = $node->author_name;
      $contrib->capacity = $node->author_capacity;
      node_save($contrib);
    } else {
      // do nothing: must either have a known mid (from form
      // autocompletion on the current submit) or cid (from previous
      // submit) to save any contrib info
    }
  }

  private function loadParticipants ($node) {
    $sql = "SELECT ip.idx, n.title AS name, ip.cid, ip.label, ip.interviewer"
      . " FROM {interview_participants} ip"
      . " INNER JOIN {interview} i ON ip.vid = i.vid"
      . " INNER JOIN {contributor} c ON ip.cid = c.nid"
      . " INNER JOIN {node} n ON c.nid = n.nid"
      . " WHERE i.vid = %d"
      . " ORDER BY ip.idx ASC";
    $results = db_query($sql, $node->vid);
    return $this->renumberResultRows($results);
  }

  private function loadStatements ($node) {
    $sql = "SELECT st.idx, st.cid, st.statement, st.statement_format, st.image"
      . " FROM {interview_statements} st"
      . " INNER JOIN {interview} i ON st.vid = i.vid"
      . " WHERE i.vid = %d"
      . " ORDER BY st.idx ASC";
    $results = db_query($sql, $node->vid);
    return $this->renumberResultRows($results);
  }

  private function getParticipantRowsHelper () {
    static $pr;
    if (!isset($pr)) {
      $this->requireLocalFile('interview.ahah', 'inc');
      $pr = new ParticipantRows();
    }
    return $pr;
  }

  private function getStatementRowsHelper () {
    static $sr;
    if (!isset($sr)) {
      $this->requireLocalFile('interview.ahah', 'inc');
      $sr = new StatementRows();
    }
    return $sr;
  }


  /**
   * return an array of cid => participant
   */
  private function nonemptyParticipantsArray ($node) {
    $ppts = array();
    if (isset($node->participants)) {
      foreach ($node->participants as $ppt) {
        $ppt = (object) $ppt;
        if (!empty($ppt) && !empty($ppt->cid)) {
          $ppts[$ppt->cid] = $ppt;
        }
      }
    }
    return $ppts;
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
function interview () {
  return InterviewModule::getInstance();
}

