<?php

pubnode_load_include("PubSourceImporter.php");

class PubSourceDBImporter extends PubSourceImporter {
  
  public function willImport ($form_state) {
    return !empty($form_state['values']['pubpath']);
  }

  public function import (&$form_state, $docid = NULL) {
    $this->pubpath = $form_state['values']['pubpath'];
    $this->docpath = $form_state['values']['docpath'];
    $this->pdfpath = $form_state['values']['pdfpath'];
    return TRUE;
  }

}
