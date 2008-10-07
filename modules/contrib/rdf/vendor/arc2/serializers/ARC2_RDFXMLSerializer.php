<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 RDF/XML Serializer
author:   Benjamin Nowack
version:  2007-09-22
*/

ARC2::inc('RDFSerializer');

class ARC2_RDFXMLSerializer extends ARC2_RDFSerializer {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_RDFXMLSerializer($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->content_header = 'application/rdf+xml';
  }

  /*  */
  
  function getTerm($v, $type) {
    if (!is_array($v)) {/* iri or bnode */
      if (preg_match('/^\_\:(.*)$/', $v, $m)) {
        return ' rdf:nodeID="' . $m[1] . '"';
      }
      if ($type == 's') {
        return ' rdf:about="' . htmlspecialchars($v) . '"';
      }
      if ($type == 'p') {
        if ($pn = $this->getPName($v)) {
          return $pn;
        }
        return 0;
      }
      if ($type == 'o') {
        $v = $this->expandPName($v);
        if (!preg_match('/^[a-z0-9]+\:[^\s]+$/is', $v)) return $this->getTerm(array('val' => $v, 'type' => 'literal'), $type);
        return ' rdf:resource="' . htmlspecialchars($v) . '"';
      }
      if ($type == 'dt') {
        $v = $this->expandPName($v);
        return ' rdf:datatype="' . htmlspecialchars($v) . '"';
      }
      if ($type == 'lang') {
        return ' xml:lang="' . htmlspecialchars($v) . '"';
      }
    }
    if (!preg_match('/literal/', $v['type'])) {
      return $this->getTerm($v['val'], 'o');
    }
    /* literal */
    $dt = isset($v['dt']) ? $v['dt'] : '';
    $lang = isset($v['lang']) ? $v['lang'] : '';
    if ($dt == 'http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral') {
      return ' rdf:parseType="Literal">' . $v['val'];
    }
    elseif ($dt) {
      return $this->getTerm($dt, 'dt') . '>' . $v['val'];
    }
    elseif ($lang) {
      return $this->getTerm($lang, 'lang') . '>' . $v['val'];
    }
    return '>' . htmlspecialchars($v['val']);
  }
  
  function getHead() {
    $r = '';
    $nl = "\n";
    $r .= '<?xml version="1.0"?>';
    $r .= $nl . '<rdf:RDF';
    $first_ns = 1;
    foreach ($this->used_ns as $v) {
      $r .= $first_ns ? ' ' : $nl . '  ';
      $r .= 'xmlns:' . $this->nsp[$v] . '="' .$v. '"';
      $first_ns = 0;
    }
    $r .= '>';
    return $r;
  }
  
  function getFooter() {
    $r = '';
    $nl = "\n";
    $r .= $nl . $nl . '</rdf:RDF>';
    return $r;
  }
  
  function getSerializedIndex($index) {
    $r = '';
    $nl = "\n";
    foreach ($index as $s => $ps) {
      $r .= $r ? $nl . $nl : '';
      $s = $this->getTerm($s, 's');
      $r .= '  <rdf:Description' .$s . '>';
      $first_p = 1;
      foreach ($ps as $p => $os) {
        if ($p = $this->getTerm($p, 'p')) {
          $r .= $nl . str_pad('', 4);
          $first_o = 1;
          if (!is_array($os)) {/* single literal o */
            $os = array(array('val' => $os, 'type' => 'literal'));
          }
          foreach ($os as $o) {
            $o = $this->getTerm($o, 'o');
            $r .= $first_o ? '' : $nl . '    ';
            $r .= '<' . $p;
            $r .= $o;
            $r .= preg_match('/\>/', $o) ? '</' . $p . '>' : '/>'; 
            $first_o = 0;
          }
          $first_p = 0;
        }
      }
      $r .= $r ? $nl . '  </rdf:Description>' : '';
    }
    $r = $this->getHead() . $nl . $nl . $r . $this->getFooter();
    return $r;
  }
  
  /*  */

}
