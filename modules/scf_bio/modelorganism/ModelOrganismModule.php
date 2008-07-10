<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class ModelOrganismModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("modelorganism", "Model Organism");
    $this->permName = "model organism";
  }

  // ------------------------------------------------------- template methods

  protected function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Name'),
      'has_body' => TRUE,
      'body_label' => t('Summary')
    );
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['modelorganism/list'] = array(
      'title' => t('Model Organisms'),
      'page callback' => 'modelorganism_list_view',
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT strain FROM {modelorganism} WHERE vid = %d";
    $mo = db_fetch_object(db_query($sql, $node->vid));
    return $mo;
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
    
    $form['strain'] = array(
      '#type' => 'textarea',
      '#title' => t('Strain'),
      '#required' => FALSE,
      '#default_value' => isset($node->strain) ? $node->strain : '',
      '#rows' => 2,
      '#weight' => $wt++
    );
    
    $this->addNodeBodyField($form, $node, $wt++);
    
    return $form;
  }


  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    // there is no css for this module yet
    // $this->addCss();
    $node = node_prepare($node, $teaser);
    if ($teaser) {
      // nothing yet
    } else {
      // undo the work of node_prepare; body is put in by theme
      unset($node->content['body']);
      $wt = 0;
      $node->content['main'] = array(
        '#value' => theme('modelorganism_main', $node),
        '#weight' => $wt++
      );
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
    return array(
      'modelorganism_main' => array(
        'template' => 'modelorganism-main',
        'arguments' => array(
          'node' => NULL,
          )
      ),
      'modelorganism_table' => array(
        'template' => 'modelorganism-table',
        'arguments' => array(
          'nodes' => array(),
          'pager' => '',
          'title' => t('Model Organisms')
        )
      )
    );
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
function modelorganism () {
  return ModelOrganismModule::getInstance();
}

