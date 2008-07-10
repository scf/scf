<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class PubGroupModule extends ScfNodeModule {
  
  public $levels = array("book");  

  public function __construct () {
    parent::__construct("pubgroup", "Publication Group");
  }

  // ------------------------------------------------------- template methods

  protected function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Title'),
      'has_body' => TRUE,
      'body_label' => t('Body')
    );
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    return array();
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT level, parentid, idx, image FROM {pubgroup} WHERE vid = %d";
    return db_fetch_object(db_query($sql, $node->vid));
  }


  /****************************************************************************
   * @see hook_validate()
   ****************************************************************************/
  public function validate ($node) {
  }

  /****************************************************************************
   * @see hook_submit()
   ****************************************************************************/
  public function nodeFormSubmit (&$form, &$form_state) {
  }

  /****************************************************************************
   * @see hook_form()
   ****************************************************************************/
  public function form (&$node, &$form_state) {
    $type = $this->getNodeTypeInfo();

    $wt = -10;

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => check_plain($type->title_label),
      '#required' => TRUE,
      '#default_value' => $node->title,
      '#weight' => $wt++
    );
      
    $this->addNodeBodyField($form, $node, $wt++);
    
    $form['image'] = array(
      '#type' => 'textfield',
      '#title' => t('Image'),
      '#default_value' => isset($node->image) ? $node->image : '',
      '#weight' => $wt++
    );
    
    $form['toc'] = array(
      '#type' => 'value',
      '#title' => t('TOC'),
      '#value' => isset($node->level) ? theme('pubgroup_toc', $node) : '',
      '#weight' => $wt++
    );
    
    return $form;
  }

  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    $this->addCss();
    if (!empty($node->image)) {
      $node->content['picture'] = array(
        '#value' => theme('image', $node->image),
        '#weight' => -1
      );
    }
    if ($teaser) {
      $node = node_prepare($node, $teaser);
    } else {
      $node->content['body'] = array(
        '#value' => $node->body,
        '#weight' => 0
      );
      $node->content['toc'] = array(
        '#value' => theme('pubgroup_toc', $node),
        '#weight' => 1
      );
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
    return array(
      'pubgroup_toc' => array(
        'arguments' => array('node' => NULL)
      )
    );
  }

  // --------------------------------------- menu callbacks

  public function jsUpload () {
  }

  // --------------------------------------- utility

  /**
   * @code
   *
   * // lev = 0
   * select * from pubnode pn
   * where pn.pgid = %d
   *
   * // lev = 1
   * select * from pubnode pn
   * join pubgroup pg0 on pn.pgid = pg0.nid
   * where pg0.parentid = %d
   *
   * // lev = 2
   * select * from pubnode pn
   * join pubgroup pg0 on pn.pgid = pg0.nid
   * join pubgroup pg1 on pg0.parentid = pg1.nid
   * where pg1.parentid = %d
   *
   * @endcode
   */
  public function generateTOC (&$node) {
    $lev = $node->level;
    $args = array();
    $q = "SELECT pn.nid FROM {pubnode} pn";
    for ($i = 0; $i < $lev; $i++) {
      $childpar = ($i == 0) ? "pn.pgid" : "pg$i.parentid";
      $q .= " JOIN {pubgroup} pg$i ON pg$i.nid = $childpar";
    }
    $targpar = ($lev == 0) ? "pn.pgid" : "pg" . $lev - 1 . ".parentid";
    $q .= " WHERE $targpar = %d ORDER BY pn.idx"; 
    $args[] = $node->nid;
    $results = db_query($q, $args);
    $toc = array();
    // FIXME: only works for lev==0 so far
    if ($lev == 0) {
      while ($nid = db_result($results)) {
        $pn = node_load($nid);
        $toc[] = l($pn->title, "node/$nid");
      }
    }
    return $toc;
  }

  /**
   * @returns e.g. array(nid1 => title1, nid2 => title2 ...)
   */
  public function listOptions ($parentId = 0) {
    $ret = array();
    $q = "SELECT n.nid, n.title FROM {pubgroup} pg JOIN {node} n ON n.vid = pg.vid WHERE pg.parentid = %d";
    $results = db_query($q, array($parentId));
    while ($pg = db_fetch_object($results)) {
      $ret[$pg->nid] = $pg->title;
    }
    return $ret;
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
function pubgroup () {
  return PubGroupModule::getInstance();
}

