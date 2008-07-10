<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class GeneModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("gene", "Gene");
  }

  // ------------------------------------------------------- template methods

  protected function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Title'),
      'has_body' => TRUE,
      'body_label' => t('Summary')
    );
  }

  protected function autocompleteFields () {
    return array('media_source', 'title');
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['gene/autocomplete/%/%'] = array(
      'title' => 'Member autocomplete',
      'page callback' => 'gene_autocomplete',
      'page arguments' => array(2, 3),
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    ); 
    $items['gene/list'] = array(
      'title' => t('Genes'),
      'page callback' => 'gene_list_view',
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT egid, symbol, names, phenotypes FROM {gene} WHERE vid = %d";
    $gene = db_fetch_object(db_query($sql, $node->vid));
    return $gene;
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
      '#maxlength' => 255,
      '#default_value' => $node->title,
      '#weight' => $wt++
    );
    
    $form['egid'] = array(
      '#type' => 'textfield',
      '#title' => t('Entrez Gene ID'),
      '#required' => TRUE,
      '#default_value' => isset($node->egid) ? $node->egid : '',
      '#weight' => $wt++
    );
    
    $form['symbol'] = array(
      '#type' => 'textfield',
      '#title' => t('Official symbol'),
      '#required' => FALSE,
      '#default_value' => isset($node->symbol) ? $node->symbol : '',
      '#weight' => $wt++
    );
    
    $form['names'] = array(
      '#type' => 'textarea',
      '#title' => t('Alternative names, symbols'),
      '#required' => FALSE,
      '#rows' => 2,
      '#default_value' => isset($node->names) ? $node->names : '',
      '#weight' => $wt++
    );
    
    $form['phenotypes'] = array(
      '#type' => 'textarea',
      '#title' => t('Phenotypes'),
      '#required' => FALSE,
      '#rows' => 2,
      '#default_value' => isset($node->phenotypes) ? $node->phenotypes : '',
      '#weight' => $wt++
    );
    
    $this->addNodeBodyField($form, $node, $wt++);
    
    return $form;
  }

  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    $this->addCss();
    $node = node_prepare($node, $teaser);
    if ($teaser) {
      // nothing yet
    } else {
      // undo the work of node_prepare; body is put in by theme
      unset($node->content['body']);
      $wt = 0;
      $node->content['main'] = array(
        '#value' => theme('gene_main', $node),
        '#weight' => $wt++
      );
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
  	// must call parent theme to get the node list template - MAG 6/3/2008
    $parent_theme = parent::theme();
  	return array_merge(
  	 $parent_theme,
  	 array(
	      'gene_main' => array(
	        'template' => 'gene-main',
	        'arguments' => array(
	          'node' => NULL
	        )
	      ),
	      'gene_table' => array(
	        'template' => 'gene-table',
	        'arguments' => array(
	          'nodes' => array(),
	          'pager' => '',
	          'title' => t('Genes')
	        )
	      )
	    )
    );
  }

  // ------------------------------------------------------- internal methods

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
function gene () {
  return GeneModule::getInstance();
}

