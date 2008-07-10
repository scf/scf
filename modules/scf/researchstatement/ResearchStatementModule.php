<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class ResearchStatementModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("researchstatement", "Research Statement");
  }

  // ------------------------------------------------------- template methods

  protected function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Statement'),   // should be optional but must specify to get around bug in node.module
      'has_body' => FALSE,
    );
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['researchstatement/list'] = array(
      'title' => t('Research Statements'),
      'page callback' => 'researchstatement_list_view',
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    );
    $items['researchstatement/autocomplete/title/%'] = array(
      'title' => t('Autocomplete'),
      'page callback' => 'researchstatement_autocomplete',
      'page arguments' => array(2,3),
      'type' => MENU_CALLBACK,
      'access callback' => TRUE,
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT pubmedid FROM {researchstatement} WHERE vid = %d";
    $rs = db_fetch_object(db_query($sql, $node->vid));
    return $rs;
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
    
    // HACK use the title for the statement...
    $form['title'] = array(
      '#type' => 'textarea',
      '#rows' => 2,
      '#title' => check_plain($type->title_label),
      '#required' => TRUE,
      '#default_value' => $node->title,
      '#weight' => $wt++
    );

    $form['pubmedid'] = array(
      '#type' => 'textfield',
      '#title' => t('PubMed ID'),
      '#required' => FALSE,
      '#default_value' => isset($node->pubmedid) ? $node->pubmedid : '',
      '#weight' => $wt++
    );
    
    return $form;
  }


  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    $this->addCss();
    $node = node_prepare($node, $teaser);
    if ($teaser) {
      $node->content['pubmed_link'] = array(
        '#value' => theme('researchstatement_pubmed_link', $node),
        '#weight' => -10
      );
    } else {
      $wt = 0;
      $node->content['main'] = array(
        '#value' => theme('researchstatement_main', $node),
        '#weight' => $wt++
      );
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
  	// must call parent theme to get node list template - MAG 6/3/2008
  	$parent_themes = parent::theme();
    $new_themes = array(
      'researchstatement_main' => array(
        'template' => 'researchstatement-main',
        'arguments' => array(
          'node' => NULL
        )
      ),
      'researchstatement_list' => array(
        'arguments' => array(
          'nodes' => array(),
          'pager' => '',
          'title' => t('Research Statements')
        )
      ),
      'researchstatement_table' => array(
        'template' => 'researchstatement-table',
        'arguments' => array(
          'nodes' => array(),
          'pager' => '',
          'title' => t('Research Statements')
        )
      ),
      'researchstatement_pubmed_link' => array(
        'arguments' => array(
          'node' => NULL,
          'verbose' => TRUE
        )
      )
    );
    return array_merge($parent_themes, $new_themes);
  }

  public function autocompleteFields () {
    return array('title');
  }

  // --------------------------------------- Utility

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
function researchstatement () {
  return ResearchStatementModule::getInstance();
}

