<?php

require_once drupal_get_path('module', 'scf') . '/ScfNodeModule.php';

define('DEFAULT_DOCTYPE_VAR', 'pubnode_default_doctype');

class PubNodeModule extends ScfNodeModule {
  
  public function __construct () {
    parent::__construct("pubnode", "Publication Node");
    $this->childTables = array("pubnode_contributors");
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

  public function insertChildren (&$node) {
    $this->insertContributors($node);
  }

  // ------------------------------------------------------- drupal hooks

  /****************************************************************************
   * @see hook_menu()
   ****************************************************************************/
  public function menu () {
    $items['pubnode/js/upload'] = array(
      'page callback' => 'pubnode_js_upload',
      'access arguments' => array('upload files'),
      'type' => MENU_CALLBACK
    );
    $items['pubnode/js/contributor/add'] = array(
      'title' => 'Javascript Add Contributor Form',
      'page callback' => 'pubnode_js_contributor_add',
      'access callback' => 'user_access',
      'access arguments' => array('edit own pubnodes'),
      'file' => 'pubnode.ahah.inc',
      'type' => MENU_CALLBACK
    );
    $items['pubnode/js/contributor/delete/%'] = array(
      'title' => 'Javascript Delete Contributor Form',
      'page callback' => 'pubnode_js_contributor_delete',
      'page arguments' => array(4),
      'access callback' => 'user_access',
      'access arguments' => array('edit own pubnodes'),
      'file' => 'pubnode.ahah.inc',
      'type' => MENU_CALLBACK
    );
    return $items;
  }

  /****************************************************************************
   * @see hook_load()
   ****************************************************************************/
  public function load ($node) {
    $sql = "SELECT pgid, idx, display_title, global_id, doctype, docpath, pubpath, pdfpath, toc, pubdate FROM {pubnode} WHERE vid = %d";
    $pn = db_fetch_object(db_query($sql, $node->vid));
    $contribs = $this->loadContributors($node);
    $pn->contributors = $contribs;
    return $pn;
  }


  /****************************************************************************
   * Validator which will be called BEFORE hook_validate()
   * and only when the "import" button is pressed
   ****************************************************************************/
  public function validateImports (&$form, &$form_state) {
    //dvm($form);
    //dvm($form_state);
    $doctype = $form_state['values']['doctype'];
    if (!empty($doctype)) {
      $processors = $this->registeredProcessors();
      $pspec = $processors[$doctype];
      if (!empty($pspec)) {
        $importer = $this->determineImporter($form_state);
        if (!empty($importer)) {
          if ($importer->import($form_state)) {
            $processor = $this->createFromSpec($pspec);
            if ($processor->load($importer)) {
              $this->resetDocumentFields($form_state);
              $this->setDocumentFields($form_state, $processor);
              return TRUE;
            } 
            // else, invalid (processor couldn't load source)
            drupal_set_message(t("Cannot process source document."), 'error');
            return FALSE;
          }
          // else, invalid (a processor was selected but document failed to actually import)
          drupal_set_message(t("Cannot import source document."), 'error');
          return FALSE;
        }
        // else, invalid (no suitable importer found -- probably fields blank)
        drupal_set_message(t("No importer found or no source document specified."), 'error');
        return FALSE;
      }
      // else, invalid (invalid document type or no suitable processor found for doctype)
      drupal_set_message(t("No suitable document processor found for type !type", array('!type' => $doctype)), 'error');
      return FALSE;
    }
    // else, trivially valid (no doctype selected)...?
    // no processors were triggered so form considered valid
    // validation at least as far as importing is concerned
    return TRUE;
  }

  /****************************************************************************
   * @see hook_validate()
   ****************************************************************************/
  public function validate ($node) {
    //dvm($node);
    if (empty($node->title)) {
      form_set_error('title', t("Title field is required"));
    }
  }

  protected function resetDocumentFields (&$form_state) {
    // reset these here, so that uploading a bad zip still replaces the current one
    // HACK: can't set to NULL because they won't override the form values, so set them to ''
    $form_state['values']['pubpath'] = '';
    $form_state['values']['docpath'] = '';
    $form_state['values']['pdfpath'] = '';
    $form_state['values']['title'] = '';
    $form_state['values']['teaser'] = '';
    $form_state['values']['body'] = '';
    $form_state['values']['authors'] = '';
    $form_state['values']['toc'] = '';
  }

  protected function setDocumentFields (&$form_state, $proc) {
    drupal_set_message(t("Found !type document at !path", array('!type' => $proc->doctype, '!path' => $proc->getAbsoluteDocPath())));
    // drupal_set_message("set document fields, pubpath = '" . $proc->pubpath . "'");
    $form_state['values']['pubpath'] = $proc->pubpath;
    $form_state['values']['docpath'] = $proc->docpath;
    $form_state['values']['pdfpath'] = $proc->pdfpath;
    $displayTitle = $proc->extractTitle();
    $form_state['values']['display_title'] = $displayTitle;
    $form_state['values']['title'] = filter_xss($displayTitle, array());
    $form_state['values']['teaser'] = $proc->extractTeaser();
    $form_state['values']['body'] = $proc->extractBody();
    $form_state['values']['authors'] = $proc->extractAuthors();
    $form_state['values']['toc'] = $proc->extractTOC();
  }

  /****************************************************************************
   * @see hook_submit()
   ****************************************************************************/
  public function nodeFormSubmit (&$form, &$form_state) {
    // remove empty contribs
    $form_state['values']['contributors'] = array_values($form_state['values']['contributors']);

    $doctype = $form_state['values']['doctype'];
    $pubpath = $form_state['values']['pubpath'];
    $pdfpath = $form_state['values']['pdfpath'];
    if (isset($form_state['values']['pgid'])) {
      $pgid = intval($form_state['values']['pgid']);
      if ($pgid > 0) {
        $nid = isset($form_state['values']['nid']) ? $form_state['values']['nid'] : 0;
        $form_state['values']['idx'] = $this->getSiblingIndex($pgid, $nid);
      }
    }

    // set pubdate as soon as it goes live
    if (empty($form_state['values']['pubdate']) && $form_state['values']['status']) {
      $form_state['values']['pubdate'] = time();
    }

    /*
    drupal_set_message("Using DOC Path: " . $docpath);
    drupal_set_message("Using DOC Type: " . $doctype);
    drupal_set_message("Using Title: " . $form_state['values']['title']);
    drupal_set_message("Using Teaser: " . $form_state['values']['teaser']);
    drupal_set_message("Using Body: " . $form_state['values']['body']);
    drupal_set_message("Using TOC: " . $form_state['values']['toc']);
    */
  }

  /****************************************************************************
   * Submit callback for the "Reprocess" button
   ****************************************************************************/
  public function reprocess (&$form, &$form_state) {
    $pubpath = $form_state['values']['pubpath'];
    // drupal_set_message("reprocess, pubpath = '$pubpath'");
    if (!empty($pubpath)) {
      $doctype = $form_state['values']['doctype'];
      $importer = $this->createLocal("PubSourceDBImporter");
      $importer->import($form_state);
      $processor = $this->createProcessorForDocType($doctype);
      if ($processor->load($importer)) {
        // dvm($processor);
        $this->resetDocumentFields($form_state);
        $this->setDocumentFields($form_state, $processor);
      } else {
        drupal_set_message(t("Cannot process source document."), 'error');
      }
    }
  }

  /****************************************************************************
   * @see hook_form()
   ****************************************************************************/
  public function form (&$node, &$form_state) {
    $type = $this->getNodeTypeInfo();

    // allow file upload
    $form['#attributes'] = array('enctype' => "multipart/form-data");

    $form['pubpath'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($node->pubpath) ? $node->pubpath : ''
    );
    $form['docpath'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($node->docpath) ? $node->docpath : ''
    );
    $form['pdfpath'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($node->pdfpath) ? $node->pdfpath : ''
    );

    $wt = -50;

    if (empty($node->body)) {

      $form['import_wrapper'] = array(
        '#title' => t('Import a source document.'),
        '#type' => 'fieldset',
        '#collapsible' => FALSE,
        '#collapsed' => FALSE, 
        '#weight' => $wt++
      );

      $this->addImportFormFields($node, $form['import_wrapper']);

      $form['title'] = array(
        '#type' => 'hidden',
        '#title' => check_plain($type->title_label),
        //'#required' => TRUE
      );

      $form['display_title'] = array(
        '#type' => 'hidden',
        '#title' => t('Display title'),
        //'#required' => TRUE
      );

      $form['pubdate'] = array(
        '#type' => 'value',
        '#value' => isset($node->pubdate) ? $node->pubdate : 0
      );
    
      $form['toc'] = array(
        '#type' => 'value'
      );

      $form['teaser'] = array(
        '#type' => 'hidden'
      );
      
      $form['body'] = array(
        '#type' => 'value'
      );

      $form['format'] = array(
        '#type' => 'value',
        '#value' => FILTER_HTML_ESCAPE
      );

      $form['pgid'] = array(
        '#type' => 'hidden'
      );

      $form['global_id'] = array(
        '#type' => 'hidden'
      );

    } else {

      $form['title'] = array(
        '#type' => 'textarea',
        '#title' => check_plain($type->title_label),
        '#rows' => 3,
        //'#required' => TRUE,
        '#default_value' => $node->title,
        '#weight' => $wt++
      );
      
      $form['display_title'] = array(
        '#type' => 'textarea',
        '#title' => t('Display title'),
        '#rows' => 3,
        '#default_value' => isset($node->display_title) ? $node->display_title : '',
        //'#required' => TRUE,
        '#weight' => $wt++
      );
      
      $form['global_id'] = array(
        '#type' => 'textfield',
        '#title' => t('Global ID'),
        '#default_value' => isset($node->global_id) ? $node->global_id : '',
        //'#required' => TRUE,
        '#weight' => $wt++
      );

      $authors = isset($form_state['values']['authors']) ? $form_state['values']['authors'] : array();
      
      $form['authors'] = array(
        '#title' => t('Authors'),
        '#value' => theme('pubnode_authors', $authors),
        '#weight' => $wt++
      );
      
      // Add a wrapper for the contributors and more button.
      $form['contributor_wrapper'] = array(
        '#tree' => FALSE,
        '#type' => 'fieldset',
        '#title' => t('Contributors'),
        '#description' => t('Identify the site members which contributed to this pubnode.'),
        '#collapsible' => TRUE,
        '#weight' => $wt++
      );
      
      $prh = $this->getContributorRowsHelper();
      $form['contributor_wrapper']['contributors'] = $prh->defineFormRows($node, $form_state);
      
      // We name our button 'pubnode_add' to avoid conflicts with other modules using
      // AHAH-enabled buttons with the id 'more' (since #tree is false for contributor_wrapper)
      
      $form['contributor_wrapper']['pubnode_add_contributor'] = array(
        '#type' => 'submit',
        '#value' => t('More contributors'),
        '#description' => t("If the amount of boxes above isn't enough, click here to add more contributors."),
        '#submit' => array('pubnode_add_contributor_submit'), // If no javascript action.
        '#ahah' => array(
          'path' => 'pubnode/js/contributor/add',
          'wrapper' => 'pubnode-contributors',
          'method' => 'replace',
          'effect' => 'fade',
          'progress' => 'none',
          ),
        '#weight' => 1
      );

      $form['teaser'] = array(
        '#type' => 'textarea',
        '#title' => t('Teaser'),
        '#rows' => 8,
        '#default_value' => isset($node->teaser) ? $node->teaser : '',
        '#weight' => $wt++
      );
      
      $form['pgid'] = array(
        '#title' => t('Book'),
        '#type' => 'select',
        // FIXME: only lists top-level for now
        '#options' => array(0 => '') + pubgroup()->listOptions(0),
        '#default_value' => isset($node->pgid) ? $node->pgid : 0,
        '#weight' => $wt++
      );

      $form['toc'] = array(
        '#type' => 'value',
        '#value' => isset($node->toc) ? $node->toc : '',
        );
      
      $form['body'] = array(
        '#type' => 'value',
        '#value' => isset($node->body) ? $node->body : '',
        );
      
      
      $form['toc_preview'] = array(
        '#title' => t('TOC (preview)'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE, 
        '#weight' => $wt++
      );
      
      $form['toc_preview']['tp'] = array(
        '#value' => isset($node->toc) ? $node->toc : ''
      );
      
      $form['body_preview'] = array(
        '#title' => t('Body (preview)'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE, 
        '#weight' => $wt++
      );
      
      $form['body_preview']['bp'] = array(
        '#value' => isset($node->body) ? $node->body : ''
      );
      
      $form['format'] = array(
        '#type' => 'value',
        '#value' => FILTER_HTML_ESCAPE
      );
      
      $form['import_wrapper'] = array(
        '#title' => t('Replace source document.'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE, 
        '#description' => t('WARNING: this will replace the current contents of this node.'),
        '#weight' => $wt++
      );

      $this->addImportFormFields($node, $form['import_wrapper']);

      $form['import_wrapper']['reprocess_button'] = array(
        '#type' => 'submit',
        '#value' => t('Reprocess'),
        '#description' => t('<b>WARNING:</b> This will overwrite any of your title and/or teaser edits.  You should only need to do this if you have updated your SCF document processing module and want to reprocess the original source.'),
        // currently does exact same thing as preview...
        '#submit' => array('pubnode_reprocess', 'node_form_build_preview'),
        '#weight' => 100
      );

    }

    $form['idx'] = array(
      '#type' => 'value',
      '#value' => isset($node->idx) ? $node->idx : 0
      // '#value' => isset($form_state['values']['idx']) ? $form_state['values']['idx'] : 0
    );

    /*
    $form['upload_button'] = array(
      '#type' => 'submit',
      '#value' => t('Upload'),
      '#ahah' => array(
        'path' => 'pubnode/js/upload',
        'wrapper' => 'archive-wrapper',
        'progress' => array('type' => 'bar', 'message' => t('Please wait...')),
      ),
      '#submit' => array('node_form_submit_build_node'),
    );
    */
    
    return $form;
  }

  protected function addImportFormFields ($node, &$form, $wt = 0) {
    $processors = $this->registeredProcessors();
    if (empty($processors)) {
      $form['doctype'] = array(
        '#value' => t('No Document processors are defined (please enable a document processing module).')
      );
      return;
    }
    // ELSE
    $processor = NULL;
    $selector_weight = $wt - 1;
    foreach ($processors as $pspec) {
      $processor = $this->createFromSpec($pspec);
    }

    $form['upload_main'] = array(
      '#type' => 'file',
      '#title' => t('Upload document source from local disk.'),
      '#default_value' => '',
      '#weight' => $wt++
    );
    
    $form['orsep'] = array(
      '#value' => '<div>OR</div>',
      '#weight' => $wt++
    );
        
    $form['httpget_main'] = array(
      '#type' => 'textfield',
      '#title' => t('Import document source from URL.'),
      '#default_value' => '',
      '#maxlength' => 512,
      '#weight' => $wt++
    );

    $form['import_button'] = array(
      '#type' => 'submit',
      '#value' => t('Import'),
      // currently does exact same thing as preview...
      '#submit' => array('node_form_build_preview'),
      '#validate' => array('pubnode_validate_imports'),
      '#weight' => 10
    );

    $options = $this->docTypeOptions();
    if (count($options) == 1) {
      $typenames = array_values($options);
      $types = array_keys($options);
      $form['doctype_msg'] = array(
        '#value' => t("Assuming Document type '!type'", array('!type' => $typenames[0])),
        '#weight' => $selector_weight
      );
      $form['doctype'] = array(
        '#type' => 'value',
        '#value' => $types[0],
        '#weight' => $selector_weight
      );
    } else {
      $selector_type = "radios";
      if (count($options) > 4) {
        $selector_type = "selector";
      }
      
      $form['doctype'] = array(
        '#title' => t('Document type'),
        '#type' => 'radios',
        // FIXME: only lists top-level for now
        '#options' => $options,
        // CAREFUL: if this changes, detect change and re-process
        '#default_value' => isset($node->doctype) ? $node->doctype : variable_get(DEFAULT_DOCTYPE_VAR, ''),
        '#weight' => $selector_weight
      );
    }
  }


  /****************************************************************************
   * @see hook_view()
   ****************************************************************************/
  public function view ($node, $teaser = FALSE, $page = FALSE) {
    if ($teaser) {
      $node = node_prepare($node, $teaser);
    } else {
      $this->addCss();
      $this->addDocTypeJs($node);
      $this->addDocTypeCss($node);
      if (!empty($node->pdfpath)) {
        $href = $node->pubpath . '/' . $node->pdfpath;
        $node->content['pdflink'] = array(
          '#value' => l(t('Download (PDF)'), $href, array('attributes' => array('class' => 'pubnode-pdflink'))),
          '#weight' => -1
        );
      }
      $node->content['body'] = array(
        '#value' => $node->body,
        '#weight' => 0
      );
    }

    /*
    if ($page) {
      $node->content['edited_info'] = array(
        '#value' => '<div id="editedinfo">Edited by...</div>',
        '#weight' => 10
      );
    }
    */
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
            $pnname = node_get_types('name', $this->name);
            $node->content['contributions']['#value'] .= theme('member_contribs', $pnname, $pnname . 's', $contribs);
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
      'pubnode_authors' => array(
        'arguments' => array('authors' => NULL)
      ),
      'pubnode_form_contributors' => array(
        'arguments' => array('form' => NULL)
      ),
      'pubnode_node_form' => array(
        'arguments' => array('form' => NULL)
      )
    );
  }


  /****************************************************************************
   * @see hook_block()
   ****************************************************************************/
  public function block ($op = 'list', $delta = 'listing', $edit = array()) {
    if ($op == 'list') {
      $blocks = parent::block('list');
      $blocks['toc'] = array(
        'info' => t('Table of Contents'),
        'weight' => 0,
        'status' => 1,
        'cache' => BLOCK_NO_CACHE,
        'region' => 'right'
      );
      return $blocks;
    } else if ($op == 'view') {
      if ($delta == 'toc') {
        // abort if we're not doing a node 'view' page
        if (!iic_util_current_page_is_node_view())
          return;
        $node = iic_util_current_page_node();
        if ($node && $node->type == 'pubnode') {
          return array(
            'subject' => t('Table of Contents'),
            'content' => $node->toc
          );
        }
      } else {
        parent::block('view', $delta);
      }
      // else
      return NULL;
    }
  }

  /****************************************************************************
   * scf's admin_settings_subform hook
   * 
   * default impl just returns print-friendly checkbox
   ****************************************************************************/
  public function adminSettingsSubform () {
    $form = parent::adminSettingsSubform();
    $options = $this->docTypeOptions();
    $form[DEFAULT_DOCTYPE_VAR] = array(
      '#type' => 'radios',
      '#options' => $options,
      '#title' => t('Default document type for publications'),
      '#default_value' => variable_get(DEFAULT_DOCTYPE_VAR, '')
    );
    return $form;
  }


  public function addCss () {
    $path = drupal_get_path('module', 'pubnode');
    // NOTE: this one should be only doctype-independent stuff:
    // everything else should be in the processor-specific css (see
    // addDocTypeCss())
    drupal_add_css("$path/pubnode.css");
  }

  public function addDocTypeJs ($node) {
    $proc = $this->createProcessorForDocType($node->doctype);
    if (!empty($proc))
      $proc->addJs();
  }

  /**
   */
  protected function addDocTypeCss ($node) {
    $proc = $this->createProcessorForDocType($node->doctype);
    if (!empty($proc))
      $proc->addCss();
  }

  protected function docTypeOptions () {
    $options = array();
    $processors = $this->registeredProcessors();
    foreach ($processors as $pspec) {
      $options[$pspec->doctype] = $pspec->description;
    }
    return $options;
  }

  // --------------------------------------- menu callbacks

  public function jsUpload () {
  }

  // --------------------------------------- utility

  /**
   * $returns Ordered array of objects defining available doc importers (fields: {module, class})
   */
  protected function registeredImporters () {
    static $importers;
    if (!isset($importers)) {
      $importers = $this->registeredHandlers("importers");
    }
    return $importers;
  }
  
  /**
   * $returns Ordered array of objects defining available doc processors (fields: {module, doctype, class})
   */
  protected function registeredProcessors () {
    static $processors;
    if (!isset($processors)) {
      $processors = $this->registeredHandlers("processors");
    }
    return $processors;
  }
  
  /**
   * @param $type either "importer" or "processor"
   */
  protected function registeredHandlers ($type) {
    $handlers = array();
    $hook = "register_pubnode_$type";
    foreach (module_implements($hook) as $module) {
      $function = $module .'_'. $hook;
      $result = call_user_func_array($function, array());
      if (isset($result) && is_array($result)) {
        foreach ($result as $key => $subresult) {
          if (is_array($subresult)) {
            $subresult['module'] = $module;
            $handlers[$key] = (object) $subresult;
          }
        }
      }
    }
    return $handlers;
  }

  /**
   * determine
   */
  protected function getSiblingIndex ($pgid, $nid = 0) {
    // if this node is already in the $parent then 
    // just return its existing index
    if (isset($node->nid) && intval($node->pgid) == intval($pgid)) {
      return $node->idx;
    }
    if ($nid == 0) {
      $q = "SELECT COUNT(DISTINCT idx) FROM {pubnode} WHERE pgid = %d";
      $results = db_query($q, array($pgid));
      return db_result($results);
    } else {
      $q = "SELECT nid, idx FROM {pubnode} WHERE pgid = %d";
      $results = db_query($q, array($pgid));
      $max = -1;
      while ($pn = db_fetch_object($results)) {
        if (intval($pn->nid) == $nid) {
          return $pn->idx;
        }
        if ($pn->idx > $max) {
          $max = $pn->idx;
        }
      }
      return $max + 1;
    }
  }


  /** 
   * @param $spec any object containing a 'module' and a 'class' field.
   */
  protected function createFromSpec ($spec) {
    $cls = $spec->class;
    module_load_include("php", $spec->module, $cls);
    return new $cls;
  }


  /** 
   * @param $spec any object containing a 'module' and a 'class' field.
   */
  protected function createLocal ($cls) {
    pubnode_load_include("$cls.php");
    return new $cls;
  }


  protected function createProcessorForDocType ($doctype) {
    $processors = $this->registeredProcessors();
    $pspec = $processors[$doctype];
    if (empty($pspec))
      return NULL;
    return $this->createFromSpec($pspec);
  }

  /**
   * Check the form submission against all registered (and default)
   * importers and return an instantiation of the first one that is
   * willing to import the document.
   *
   * @return an importer or NULL
   */
  protected function determineImporter (&$form_state) {
    foreach ($this->registeredImporters() as $ispec) {
      $imp = $this->createFromSpec($ispec);
      if ($imp->willImport($form_state)) {
        return $imp;
      }
    }
    // else use the generic ones
    $imp = $this->createLocal("PubSourceZipUploadImporter");
    if ($imp->willImport($form_state))
      return $imp;
    // else
    $imp = $this->createLocal("PubSourceHttpImporter");
    if ($imp->willImport($form_state))
      return $imp;
    // else 
    return NULL;
  }

  private function getContributorRowsHelper () {
    static $cr;
    if (!isset($cr)) {
      $this->requireLocalFile('pubnode.ahah', 'inc');
      $cr = new ContributorRows();
    }
    return $cr;
  }

  private function listMemberContributions ($uid) {
    $sql = "SELECT DISTINCT n.nid FROM {node} n";
    $sql .= " INNER JOIN {pubnode} p ON p.vid = n.vid";
    $sql .= " INNER JOIN {pubnode_contributors} pc ON pc.vid = p.vid";
    $sql .= " INNER JOIN {contributor} c ON pc.cid = c.nid";
    $sql .= " INNER JOIN {users} u ON c.cuid = u.uid";
    $sql .= " WHERE n.status = 1";
    $sql .= " AND u.uid = %d";
    $sql .= " ORDER BY p.pubdate ASC";
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

  private function insertContributors ($node) {
    foreach ($node->contributors as $i => $auth) {
      $auth = (object) $auth;
      $cid = isset($auth->cid) ? $auth->cid : 0;
      $mid = isset($auth->mid) ? $auth->mid : 0;
      if (!$cid && $mid) {
        $mem = node_load($mid);
        if ($mem) {
          $cid = contributor_create_from_user($mem->muid);
          if ($cid) {
            $node->contributors[$i]['cid'] = $cid;
          }
        }
      }
    }
    $this->insertIndexedChildren($node, $node->contributors, "pubnode_contributors", "cid");
  }
  
  private function loadContributors ($node) {
    $sql = "SELECT pc.idx, n.title AS name, pc.cid"
      . " FROM {pubnode_contributors} pc"
      . " INNER JOIN {pubnode} p ON pc.vid = p.vid"
      . " INNER JOIN {contributor} c ON pc.cid = c.nid"
      . " INNER JOIN {node} n ON c.nid = n.nid"
      . " WHERE p.vid = %d"
      . " ORDER BY pc.idx ASC";
    $results = db_query($sql, $node->vid);
    return $this->renumberResultRows($results);
  }

  /**
   * return an array of cid => contributor
   */
  private function nonemptyContributorsArray ($node) {
    $auths = array();
    if (isset($node->contributors)) {
      foreach ($node->contributors as $auth) {
        $auth = (object) $auth;
        if (!empty($auth) && !empty($auth->cid)) {
          $auths[$auth->cid] = $auth;
        }
      }
    }
    return $auths;
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
function pubnode () {
  return PubNodeModule::getInstance();
}


