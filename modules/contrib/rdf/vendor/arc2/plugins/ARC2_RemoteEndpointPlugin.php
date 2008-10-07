<?php
/*
homepage: http://bzr.mfd-consult.dk/remote-endpoint/
license:  http://arc.semsol.org/license

class:    ARC2 Remote (SPARQL) Endpoint
author:   Morten Høybye Frederiksen
version:  2008-01-15 (initial)
*/

ARC2::inc('Store');

class ARC2_RemoteEndpointPlugin extends ARC2_Store {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }

  function ARC2_RemoteEndpointPlugin($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
  }

  /*
    isSetUp: Always true...
  */
	function isSetUp() {
		return true;
	}

  /*
    getEndpointURL: Return configured endpoint URL.
  */
	function getEndpointURL() {
		return $this->v('endpoint_url', 'http://localhost/', $this->a);
	}

  /*
    query: Handle query against remote endpoint.
  */
  function query($q, $result_format = '', $src = '', $keep_bnode_ids = 0) {
    /* Parse query */
    ARC2::inc('SPARQLPlusParser');
    $p = & new ARC2_SPARQLPlusParser($this->a, $this);
    $p->parse($q, $src);
    $infos = $p->getQueryInfos();
    $infos['result_format'] = $result_format;
    if (!$p->getErrors()) {
      /* Check and execute query */
      $qt = $infos['query']['type'];
      if (!in_array($qt, array('select', 'ask', 'describe', 'construct', 'load', 'insert', 'delete')))
        return $this->addError('Unsupported query type "'.$qt.'"');
      $t1 = ARC2::mtime();
      $r = array('result' => $this->runQuery($qt, $q));
      $t2 = ARC2::mtime();
      $r['query_time'] = $t2 - $t1;
      if ($result_format == 'raw')
        return $r['result'];
      if ($result_format == 'rows')
        return $r['result']['rows'] ? $r['result']['rows'] : array();
      if ($result_format == 'row')
        return $r['result']['rows'] ? $r['result']['rows'][0] : array();
      return $r;
    }
    return 0;
  }

  /*
    runQuery: Run query against remote endpoint.
  */
  function runQuery($type, $q) {
    /* load/insert/delete must use POST */
    if (in_array($type, array('load', 'insert', 'delete')))
      return $this->addError('Remote Endpoint: "' . $type . '" query must use POST, not supported (yet)');

    /* construct url */
    $url = $this->getEndpointURL();
    $url .= '?query=' . urlencode($q);

    /* reader */
    ARC2::inc('Reader');
    $reader =& new ARC2_Reader('', $this);
    $reader->setAcceptHeader('Accept: application/sparql-results+xml; q=0.9, application/rdf+xml; q=0.9, */*; q=0.1');
    $reader->activate($url);
    if ($reader->getErrors()) {
      $this->errors = $reader->errors;
      return false;
    }

    /* check result format */
    $format = $reader->getFormat();
    $mappings = array('rdfxml' => 'RDFXML', 'xml' => 'XML');
    if (!$format || !isset($mappings[$format])) {
      $reader->closeStream();
      return $this->addError('No remote endpoint result handler available for "' . $format . '", returned from "' . $this->getEndpointURL() . '"');
    }

    /* handle result */
    $m = 'parse' . $mappings[$format] . 'ResultDoc';
    if (method_exists($this, $m))
      $this->$m($url, $type, $reader);
    else
      return $this->addError('No remote endpoint result handler available for "' . $mappings[$format] . '"');
    return $this->result;
  }

  /*
    parseRDFXMLResultDoc: Parse query result into ARC simple index structure.
  */
  function parseRDFXMLResultDoc($url, $type, &$reader) {
    $parser = ARC2::getRDFXMLParser();
    $parser->setReader($reader);
    $parser->parse($url);
    if ($parser->getErrors()) {
      $this->errors = $parser->errors; # @@@ Could do with (improved) addError(s)?
      return;
    }
    $this->result = $parser->getSimpleIndex(false);
  }

  /*
    parseXMLResultDoc: Parse query result into ARC structure.
  */
  function parseXMLResultDoc($url, $type, &$reader) {
    $parser = ARC2::getSPARQLXMLResultParser();
    $parser->reader =& $reader; # @@@ Should really be: $parser->setReader($reader);
    $parser->parse($url);
    if ($parser->getErrors()) {
      $this->errors = $parser->errors; # @@@ Could do with (improved) addError(s)?
      return;
    }
    $this->result = $parser->getStructure();
    if ('ask'==$type)
      $this->result = $this->result['boolean'];
  }

}

?>