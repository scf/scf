<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class AntibodyModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("antibody", "Antibody");
    $this->permNamePlural = "antibodies";
    $this->displayNamePlural = "Antibodies";
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
    $items['antibody/list'] = array(
      'title' => t('Antibodies'),
      'page callback' => 'antibody_list_view',
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT clonality FROM {antibody} WHERE vid = %d";
    $ab = db_fetch_object(db_query($sql, $node->vid));
    return $ab;
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
    
    $form['clonality'] = array(
      '#type' => 'radios',
      '#title' => t('Clonality'),
      '#required' => TRUE,
      '#options' => array('monoclonal' => 'monoclonal', 'polyclonal' => 'polyclonal'),
      '#default_value' => isset($node->clonality) ? $node->clonality : '',
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
        '#value' => theme('antibody_main', $node),
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
      'antibody_main' => array(
        'template' => 'antibody-main',
        'arguments' => array(
          'node' => NULL
        )
      ),
      'antibody_table' => array(
        'template' => 'antibody-table',
        'arguments' => array(
          'nodes' => array(),
          'pager' => '',
          'title' => t('Antibodies')
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
function antibody () {
  return AntibodyModule::getInstance();
}

