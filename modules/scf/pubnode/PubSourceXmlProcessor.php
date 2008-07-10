<?php

/**
 * TODO: make extend more general PubSourceProcessor if there are ever
 * any that are non-XML based.
 */
abstract class PubSourceXmlProcessor {
  
  public $doctype;

  // the following are determined via load($pubPath)
  public $pubpath;
  public $docpath;
  public $pdfpath;

  // internal use
  private $savedCatalogPath;

  protected function __construct ($doctype) {
    $this->doctype = $doctype;
    // override in subclass e.g. if xsl subdirectory is not same as doctype name
  }

  public function load ($importer) {
    $this->pubpath = $importer->pubpath;
    $this->docpath = (!empty($importer->docpath)) ? $importer->docpath : $this->findDoc();
    if (!empty($this->docpath)) {
      $this->pdfpath = (!empty($importer->pdfpath)) ? $importer->pdfpath : $this->findPdf();
      return TRUE;
    }
    // else
    return FALSE;
  }

  public function extractTitle () {
    return $this->docFragment($this->transformXmlForView("title"));
  }

  public function extractTeaser () {
    return $this->docFragment($this->transformXmlForView("teaser"));
  }

  public function extractBody () {
    return $this->stripXmlPreamble($this->transformXmlForView("body"));
  }

  public function extractTOC () {
    return $this->stripXmlPreamble($this->transformXmlForView("toc"));
  }

  /**
   * Returns a sequential array of arrays, where each sub-array is of form:
   * @code
   *   array(
   *     'lastname' => 'Jones',
   *     'firstnames' => 'John D.',
   *     'email' => 'jdj@unh.edu',
   *     'affiliation' => 'UNH Rat Control Lab'
   *   )
   * @endcode
   */
  public function extractAuthors () {
    return $this->parseAuthors($this->transformXmlForView("authors"));
  }

  /**
   * TODO: move to static util function
   */
  protected function stripXmlPreamble ($xmlStr) {
    $xmlStr = preg_replace('{\A\s*<\?xml[^>]+\?>\s*}i', '', $xmlStr, 1);
    $xmlStr = preg_replace('{\A\s*<!doctype[^>]+>\s*}i', '', $xmlStr, 1);
    return $xmlStr;
  }

  protected function docFragment ($xmlStr) {
    $xmlStr = $this->stripXmlPreamble($xmlStr);
    return preg_replace('{\A\s*<div>\s*(.*)\s*</div>\s*}', '$1', $xmlStr, 1);
  }

  /**
   * @return the path of a document contained within pub path, or NULL
   *   if this pubpath does not contain a document that can be processed
   *   by this processor.
   */
  public function findDoc () {
    // drupal_set_message("locating main doc");
    $files = $this->findCandidateFiles();
    foreach ($files as $file) {
      if ($this->fileMatchesFormat($file)) {
        return $this->stripPubPath($file);
      }
    }
    // else
    return NULL;
  }


  public function stripPubPath ($path) {
    $pp = $this->pubpath;
    $len = strlen($pp);
    if ($pp == substr($path, 0, $len)) {
      $path = substr($path, $len);
      if (substr($path, 0, 1) == '/')
        return substr($path, 1);
      else
        return $path;
    }
    return $path;
  }

  /**
   * @returns The first PDF file found in the archive.
   */
  public function findPdf () {
    $files = $this->candidateFiles('pdf');
    if (isset($files[0])) {
      return $this->stripPubPath($files[0]);
    }
    // else
    return NULL;
  }

  /**
   * parse the return from the author extraction XSL (e.g.:
   *    'Jones^John D.^jdj@unh.edu^UNH Rat Control Lab|Smith^Jeanne-Marie Q.^jms@gmail.com^Pizza Hut'
   */
  protected function parseAuthors ($str) {
    $ret = array();
    $authors = explode('|', $str);
    foreach ($authors as $author) {
      list($lastname, $firstnames, $email, $affiliation) = explode('^', $author);
      $ret[] = array(
        'lastname' => trim($lastname),
        'firstnames' => trim($firstnames),
        'email' => trim($email),
        'affiliation' => trim($affiliation)
      );
    }
    return $ret;
  }

  /**
   * override if main doc may have other than ".xml" extension
   */
  protected function findCandidateFiles () {
    return $this->candidateFiles('xml');
  }

  /**
   * Find files of the given extension type, looking at the root of
   * the pub path and two levels down (in case uploaded zip archive has
   * a shallow directory substructure).
   */
  protected function candidateFiles ($ext) {
    $extUp = strtoupper($ext);
    $pp = $this->pubpath;
    return array_unique(
      glob($pp . "/*.$ext")
      + glob($pp . "/*.$extUp")
      + glob($pp . "/*/*.$ext")
      + glob($pp . "/*/*.$extUp")
      + glob($pp . "/*/*/*.$ext")
      + glob($pp . "/*/*/*.$extUp"));
  }

  protected function fileCheckRootElement ($file, $localname, $nsuri = NULL) {
    $parser = new XMLReader();
    // FIXME: props don't work until libxml2 version 2.6.28
    // Load DTD to get entity defs
    // $parser->setParserProperty(XMLReader::LOADDTD, TRUE);
    // $parser->setParserProperty(XMLReader::VALIDATE, FALSE);
    $parser->open($file);
    $found = FALSE;
    try {
      while (@$parser->read()) {
        // see if first element is "article"
        if ($parser->nodeType == XMLReader::ELEMENT) {
          if ($parser->name == $localname) {
            if ($nsuri === NULL || $nsuri == $parser->namespaceURI) {
              $found = TRUE;
            }
          }
          break;
        }
      }
    } catch (Exception $e) {
      // squelch exceptions: we're just testing to see if this doc contains 
      // a particular root element; if we get an error just return false;
    }
    $parser->close();
    return $found;
  }

  protected function fileCheckPreamblePattern ($file, $pattern) {
    $preamble = file_get_contents($file, 0, NULL, 0, 1000);
    if (strpos($preamble, $pattern) !== FALSE)
      return TRUE;
    // else
    return FALSE;
  }

  /**
   * @param $view what view of the document you want (e.g. "title",
   *   "teaser", "authors"...)
   * @param $params an array of name-value pairs to send to xslt proc
   */
  protected function transformXmlForView ($view, $params = NULL) {
    $libxml_opts = LIBXML_DTDLOAD | LIBXML_NONET | LIBXML_NOWARNING | LIBXML_NOENT;
    // $libxml_opts = LIBXML_DTDLOAD;
    $this->saveXmlCatalog();
    // Use the following to debug catalog resolution (may require restart of apache for some reason)
    // putenv("XML_DEBUG_CATALOG=1");
    // $_ENV["XML_DEBUG_CATALOG"] = "1";
    $this->setupXmlCatalog();
    // drupal_set_message("XML_CATALOG_FILES = " . getenv("XML_CATALOG_FILES"));
    // libxml_use_internal_errors(TRUE);
    // libxml_clear_errors();
    $xml = new DOMDocument();
    $xml->load($this->getAbsoluteDocPath(), $libxml_opts);
    //libxml_clear_errors();
    $xsl = new DOMDocument();
    $xsl->load($this->viewXslPath($view), $libxml_opts);
    $proc = new XSLTProcessor;
    $proc->importStyleSheet($xsl); // attach the xsl rules
    if (isset($params)) {
      $proc->setParameter('', $params);
    }
    $transformed = $proc->transformToXML($xml);
    $this->restoreXmlCatalog();
    return $transformed;
  }

  protected function viewXslPath ($view) {
    return $this->viewXslBasePath() . "/$view.xsl";
  }

  /**
   * template method: use to set up xml catalog for your transforms
   */
  protected function setupXmlCatalog () {
  }
    
  protected function saveXmlCatalog () {
    $this->savedCatalogPath = getenv("XML_CATALOG_FILES");
  }

  protected function restoreXmlCatalog () {
    $this->setXmlCatalogPath($this->savedCatalogPath);
  }

  protected function setXmlCatalogPath ($pathlist) {
    // REQUIRES safe mode off
    putenv("XML_CATALOG_FILES=$pathlist");
    $_ENV["XML_CATALOG_FILES"] = $pathlist;
  }

  protected function prependXmlCatalog ($paths) {
    $curcat = getenv("XML_CATALOG_FILES");
    if (empty($curcat)) {
      // FIXME: this is linux-specific...
      $curcat = "/etc/xml/catalog";
    }
    if (is_array($paths)) {
      // the XML_CATALOG_FILES list is space-separated
      $pathlist = implode(' ', $paths);
    } else {
      $pathlist = $paths;
    }
    // abort if already contains the path at beginning
    if (strpos($curcat, $pathlist) !== 0) {
      $this->setXmlCatalogPath(trim($pathlist . " " . $curcat));
    }
  }

  public function getAbsoluteDocPath () {
    // pubpath guaranteed non-empty
    return $this->pubpath . "/" . $this->docpath;
  }

  public function getAbsolutePdfPath () {
    // pubpath guaranteed non-empty
    return $this->pubpath . "/" . $this->pdfpath;
  }

  /**
   * template: override to add doctype-specific css
   */
  public function addCss () {
  }

  /**
   * template: override to add doctype-specific js
   */
  public function addJs () {
  }

  protected abstract function viewXslBasePath ();

}
