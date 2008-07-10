<?php

/**
 * <p>Terminology:</p>
 *
 * <dl>
 * <dt>base path:</dt>
 * <dd>directory where all pubnode docs are stored (e.g. "files/pubnode")</dd>
 *
 * <dt>doc ID:</dt>
 * <dd>unique ID found by hashing the uploaded file (e.g. "6025bdeb90c40b60f7c009a8869176c")</dd>
 *
 * <dt>pub path:</dt>
 * <dd>directory containing all files for a given pubnode 
 *     (e.g. "files/pubnode/6025bdeb90c40b60f7c009a8869176c")</dd>
 *
 * <dt>doc path:</dt>
 * <dd>file or directory that unambigously identifies where to go to process the source for this pubnode.  
 *     Normally (in the case of an XML pubnode format such as DocBook) it will be the path of the 
 *     file containing root element for the pubnode, relative to the pub path
 *     (e.g. "article.xml" or "xml/maindoc.xml").  
 *     <b>NOTE</b> that this file need not be a direct child of the pubnode
 *     path, but may be in a lower directory.</dd>
 *
 * </dl>
 *
 */
abstract class PubSourceImporter {
  
  public $pubpath;

  public $docpath;

  public $pdfpath;

  public $refspath;

  public $hashAlgorithm = "sha1";

  public function __construct () {
  }

  protected function constructPubPath ($docid) {
    return file_directory_path() . "/pubnode/$docid";
  }

  /**
   * @returns TRUE iff an importable document was found
   *
   * If import fails must return FALSE <b>and</b> set any necessary
   * form errors thru form_set_error()
   * 
   */
  public abstract function import (&$form_state);

  public abstract function willImport ($form_state);

  protected function hashFile ($filepath) {
    return hash_file($this->hashAlgorithm, $filepath);
  }

}
