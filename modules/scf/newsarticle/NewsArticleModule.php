<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

class NewsArticleModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("newsarticle", "News Article");
    $this->permName = "news article";
  }

  // ------------------------------------------------------- template methods

  protected function extraNodeInfo () {
    return array(
      'has_title' => TRUE,
      'title_label' => t('Title'),
      'has_body' => TRUE,
      'body_label' => t('Abstract')
    );
  }

  protected function autocompleteFields () {
    return array('media_source');
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['newsarticle/autocomplete/media_source/%'] = array(
      'title' => 'Member autocomplete',
      'page callback' => 'newsarticle_autocomplete',
      'page arguments' => array(2, 3),
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    );
    $items['newsarticle/list'] = array(
      'title' => t('News'),
      'page callback' => 'newsarticle_list_view',
      'access callback' => TRUE,
      'type' => MENU_CALLBACK
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT author_name, media_source, url, pubdate FROM {newsarticle} WHERE vid = %d";
    $na = db_fetch_object(db_query($sql, $node->vid));
    $na->datestr = scf_date_string($na->pubdate);
    return $na;
  }


  /****************************************************************************
   * @see hook_validate()
   ****************************************************************************/
  public function validate ($node) {
    // As of PHP 5.1.0, strtotime returns FALSE instead of -1 upon failure.
    if (!empty($node->datestr) && strtotime($node->datestr) < 0) {
      form_set_error('datestr', t('Please specify a valid date.'));
    }
  }

  /****************************************************************************
   * @see hook_submit()
   ****************************************************************************/
  public function nodeFormSubmit (&$form, &$form_state) {
    $form_state['values']['pubdate'] = strtotime($form_state['values']['datestr']);
  }

  /****************************************************************************
   * @see hook_form()
   ****************************************************************************/
  public function form (&$node, &$form_state) {
    $type = $this->getNodeTypeInfo();
    
    $wt = -15;
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => check_plain($type->title_label),
      '#required' => TRUE,
      '#default_value' => $node->title,
      '#weight' => $wt++
    );
    
    $form['author_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Author name'),
      '#default_value' => isset($node->author_name) ? $node->author_name : '',
      '#size' => 32,
      '#maxlength' => 32,
      '#weight' => $wt++
    );
    
    $form['media_source'] = array(
      '#type' => 'textfield',
      '#title' => t('Media Source'),
      '#default_value' => isset($node->media_source) ? $node->media_source : '',
      '#description' => t('Online publisher name.'),
      '#autocomplete_path' => 'newsarticle/autocomplete/media_source',
      '#size' => 64,
      '#maxlength' => 128,
      '#weight' => $wt++
    );
    
    $form['url'] = array(
      '#type' => 'textfield',
      '#title' => t('URL'),
      '#default_value' => isset($node->url) ? $node->url : '',
      '#description' => t('Article URL.'),
      '#size' => 80,
      '#maxlength' => 512,
      '#weight' => $wt++
    );
    
    $form['datestr'] = array(
      '#type' => 'textfield',
      '#title' => t('Publication date'),
      '#default_value' => isset($node->datestr) ? $node->datestr : scf_date_string(time()),
      '#maxlength' => 18,
      '#description' => t('Format: %date', array('%date' => scf_date_string(time()))),
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
    $node->content['byline'] = array(
      '#value' => theme('newsarticle_byline', $node),
      '#weight' => -3
    );
    if (!$teaser) {
      $node->content['url'] = array(
        '#value' => theme('more_link', $node->url, t('Read more about this...')),
        '#weight' => 2
      );
    }
    return $node;
  }


  /****************************************************************************
   * @see hook_theme()
   ****************************************************************************/
  public function theme () {
    return array(
      'newsarticle_byline' => array(
        'template' => 'newsarticle-byline',
        'arguments' => array(
          'node' => NULL,
          )
      )
    );
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
function newsarticle () {
  return NewsArticleModule::getInstance();
}

