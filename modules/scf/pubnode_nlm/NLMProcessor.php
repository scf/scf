<?php

pubnode_load_include("PubSourceXmlProcessor.php");

/**
 */
class NLMProcessor extends PubSourceXmlProcessor {
  
  public function __construct () {
    parent::__construct("nlm");
  }

  public function fileMatchesFormat ($file) {
    return ($this->fileCheckPreamblePattern($file, "//NLM//")
      && $this->fileCheckRootElement($file, "article"));
  }

  public function extractBody () {
    $reldir = dirname($this->getAbsoluteDocPath());
    $body = $this->transformXmlForView("body", array('FILES_PREFIX' => file_create_url($reldir)));
    return $this->stripXmlPreamble($body);
  }

  protected function viewXslBasePath () {
    return drupal_get_path('module', 'pubnode_nlm') . '/xsl';
  }

  /**
   * template method: use to set up xml catalog for your transforms
   */
  protected function setupXmlCatalog () {
    $module_dir = dirname(__FILE__);
    $paths = array();
    foreach (array("2.3", "2.2", "2.1", "2.0") as $ver) {
      $paths[] = $module_dir . '/dtd/' . $ver . "/catalog.xml";
    }
    $this->prependXmlCatalog($paths);
  }
    
  public function addCss () {
    $path = drupal_get_path('module', 'pubnode_nlm') . '/css';
    drupal_add_css("$path/nlm.css");
  }

  public function addJs () {
    if (module_exists('nodeproxy')) {
      $path = drupal_get_path('module', 'pubnode_nlm') . '/js';
      if (user_access('create proxied nodes')) {
        drupal_add_js("$path/pubnode_nlm_nodeproxy.js");
      }
    }
  }

}
