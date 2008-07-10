<?php

pubnode_load_include("PubSourceXmlProcessor.php");

/**
 */
class PLoSProcessor extends PubSourceXmlProcessor {
  
  public function __construct () {
    parent::__construct("plos");
  }

  public function fileMatchesFormat ($file) {
    return ($this->fileCheckPreamblePattern($file, "//NLM//")
      && $this->fileCheckRootElement($file, "article"));
  }

  public function extractBody () {
    $body = $this->transformXmlForView("body");
    // HACK: rewrite img src urls, because no good way to change it by passing a param to the xsl
    // (the $img.src.path setting above doesn't work because unparsed-entity-uri() is returning 
    // an absolute filesystem path, which causes $img.src.path to be ignored)
    $reldir = dirname($this->getAbsoluteDocPath());
    $pattern = '{src="([^"]+)/' . $reldir . '}';
    $body = preg_replace($pattern, 'src="' . file_create_url($reldir), $body);
    return $body;    
  }

  protected function viewXslBasePath () {
    return drupal_get_path('module', 'pubnode_plos') . '/xsl';
  }

  public function addCss () {
    $path = drupal_get_path('module', 'pubnode_plos') . '/css';
    drupal_add_css("$path/ViewNLM.css");
    //drupal_add_css("$path/pone_screen.css");
    //drupal_add_css("$path/pone_iepc.css");
  }

}
