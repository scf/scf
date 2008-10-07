<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 RDF/JSON Serializer
author:   Benjamin Nowack
version:  2007-10-29
*/

ARC2::inc('RDFSerializer');

class ARC2_RDFJSONSerializer extends ARC2_RDFSerializer {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_RDFJSONSerializer($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->content_header = 'application/json';
  }

  /*  */
  
  function getTerm($v, $term = 's') {
    if (!is_array($v)) {
      if (preg_match('/^\_\:/', $v)) {
        return ($term == 'o') ? $this->getTerm(array('val' => $v, 'type' => 'bnode'), 'o') : '"' . $v . '"';
      }
      return ($term == 'o') ? $this->getTerm(array('val' => $v, 'type' => 'uri'), 'o') : '"' . $v . '"';
    }
    if (!isset($v['type']) || !preg_match('/literal/', $v['type'])) {
      if ($term != 'o') {
        return $this->getTerm($v['val'], $term);
      }
      if (preg_match('/^\_\:/', $v['val'])) {
        return '{ "value" : "' . $v['val']. '", "type" : "bnode" }';
      }
      return '{ "value" : "' . $v['val']. '", "type" : "uri" }';
    }
    /* literal */
    $val = str_replace(array("\r\n", "\r", "\n", '"', '\\\"'), array('\r\n', '\r', '\n', '\"', '\\\\\"'), $v['val']);
    $r = '{ "value" : "' . $val. '", "type" : "literal"';
    $suffix = isset($v['dt']) ? ', "datatype" : "' . $v['dt'] . '"' : '';
    $suffix = isset($v['lang']) ? ', "lang" : "' . $v['lang'] . '"' : $suffix;
    $r .= $suffix . ' }';
    return $r;
  }
  
  function getSerializedIndex($index) {
    $r = '';
    $nl = "\n";
    foreach ($index as $s => $ps) {
      $r .= $r ? ',' . $nl . $nl : '';
      $r .= '  ' . $this->getTerm($s). ' : {';
      $first_p = 1;
      foreach ($ps as $p => $os) {
        $r .= $first_p ? $nl : ',' . $nl;
        $r .= '    ' . $this->getTerm($p). ' : [';
        $first_o = 1;
        if (!is_array($os)) {/* single literal o */
          $os = array(array('val' => $os, 'type' => 'literal'));
        }
        foreach ($os as $o) {
          $r .= $first_o ? $nl : ',' . $nl;
          $r .= '      ' . $this->getTerm($o, 'o');
          $first_o = 0;
        }
        $first_p = 0;
        $r .= $nl . '    ]';
      }
      $r .= $nl . '  }';
    }
    $r .= $r ? ' ' : '';
    return '{' . $nl . $r . $nl . '}';
  }
  
  /*  */

}
