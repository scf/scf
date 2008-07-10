<?php

pubnode_load_include("PubSourceXmlProcessor.php");

define('DOCBOOK_XML_CAT', 'docbook-xsl-1.73.2/catalog.xml');

/**
 */
class DocBook4Processor extends PubSourceXmlProcessor {
  
  public function __construct () {
    parent::__construct("docbook4");
  }

  public function fileMatchesFormat ($file) {
    return ($this->fileCheckPreamblePattern($file, "DocBook XML")
      && $this->fileCheckRootElement($file, "article"));
  }

  public function extractBody () {
    $body = $this->transformXmlForView("body", array("img.src.path" => "files/xxxx/"));
    // HACK: rewrite img src urls, because no good way to change it by passing a param to the xsl
    // (the $img.src.path setting above doesn't work because unparsed-entity-uri() is returning 
    // an absolute filesystem path, which causes $img.src.path to be ignored)
    $reldir = dirname($this->getAbsoluteDocPath());
    // $reldir is something like:
    // 'sites/all/files/pubnode/078d96b2d2b634e6b1ac5e503ffacee4694b5303/Genome_Biology_Gupta/xml'
    drupal_set_message('reldir = ' . $reldir);
    $pattern = '{src=(["\'])([^"\']+)/' . $reldir . '}';
    //$pattern = '{src=./var/www/vhosts/scf6/htdocs/' . $reldir . '}';
    $body = preg_replace($pattern, "src=$1/$reldir", $body);
    return $body;    
  }

  /**
   * template method: use to set up xml catalog for your transforms
   */
  protected function setupXmlCatalog () {
    // FIXME: get abs system file path...
    $this->prependXmlCatalog($this->viewXslBasePath() . '/' . DOCBOOK_XML_CAT);
  }
    
  protected function viewXslBasePath () {
    return drupal_get_path('module', 'pubnode_docbook4') . '/xsl';
  }

}
