<?php

pubnode_load_include("PubSourceImporter.php");

class PubSourceHttpImporter extends PubSourceImporter {
  
  protected $fieldname;
  protected $savename = 'article.xml';

  public function __construct ($fieldname = 'httpget_main') {
    $this->fieldname = $fieldname;
  }

  public function willImport ($form_state) {
    return !empty($form_state['values'][$this->fieldname]);
  }

  public function import (&$form_state, $docid = NULL) {
    $url = $form_state['values'][$this->fieldname];
    if (empty($url)) {
      return FALSE;
    }
    $response = drupal_http_request($url);
    // drupal_set_message("URL request ($url) returned status code $response->code.");
    if ($response->code != 200) {
      drupal_set_message("URL request ($url) returned unexpected status code $response->code.", 'error');
      return FALSE;
    }
    // else
    $content = $response->data;
    if (empty($docid)) {
      $docid = hash($this->hashAlgorithm, $content);
    }
    $pubpath = $this->constructPubPath($docid);
    file_check_directory($pubpath, FILE_CREATE_DIRECTORY);

    $savename = $this->savename;
    if (empty($savename)) {
      $savename = preg_replace('{[\#\?].*\Z}', '', $url);
      $savename = preg_replace('{\.([^.])+\Z}', '\#$1', $savename);
      $savename = preg_replace('/[^a-zA-Z0-9_\#-]/', '_', $savename);
      $savename = preg_replace('/\#/', '.', $savename);
    }

    $destfile = "$pubpath/$savename";
    file_save_data($content, $destfile, FILE_EXISTS_REPLACE);
    
    drupal_set_message(t("Saved !num bytes to file !file",
                         array('!num' => strlen($content), '!file' => $destfile)));
    $this->pubpath = $pubpath;
    $this->docpath = $savename;
    return TRUE;
  }


}
