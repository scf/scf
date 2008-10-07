<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 Turtle Serializer
author:   Benjamin Nowack
version:  2007-01-17 (Fix: Quotation guessing was still not correct)
*/

ARC2::inc('RDFSerializer');

class ARC2_TurtleSerializer extends ARC2_RDFSerializer {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_TurtleSerializer($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->content_header = 'application/x-turtle';
  }

  /*  */
  
  function getTerm($v) {
    if (!is_array($v)) {
      if (preg_match('/^\_\:/', $v)) {
        return $v;
      }
      if ($pn = $this->getPName($v)) {
        return $pn;
      }
      return '<' .$v. '>';
    }
    if (!isset($v['type']) || !preg_match('/literal/', $v['type'])) {
      return $this->getTerm($v['val']);
    }
    /* literal */
    $quot = '"';
    if (preg_match('/\"/', $v['val'])) {
      $quot = "'";
      if (preg_match('/\'/', $v['val'])) {
        $quot = '"""';
        if (preg_match('/\"\"\"/', $v['val']) || preg_match('/\"$/', $v['val']) || preg_match('/^\"/', $v['val'])) {
          $quot = "'''";
          $v['val'] = preg_replace("/'$/", "' ", $v['val']);
          $v['val'] = preg_replace("/^'/", " '", $v['val']);
          $v['val'] = str_replace("'''", '\\\'\\\'\\\'', $v['val']);
        }
      }
    }
    if ((strlen($quot) == 1) && preg_match('/[\x0d\x0a]/', $v['val'])) {
      $quot = $quot . $quot . $quot;
    }
    $suffix = isset($v['lang']) ? '@' . $v['lang'] : '';
    $suffix = isset($v['dt']) ? '^^' . $this->getTerm($v['dt']) : $suffix;
    return $quot .$v['val']. $quot . $suffix;
  }
  
  function getHead() {
    $r = '';
    $nl = "\n";
    foreach ($this->used_ns as $v) {
      $r .= $r ? $nl : '';
      $r .= '@prefix ' . $this->nsp[$v] . ': <' .$v. '> .';
    }
    return $r;
  }
  
  function getSerializedIndex($index) {
    $r = '';
    $nl = "\n";
    foreach ($index as $s => $ps) {
      $r .= $r ? ' .' . $nl . $nl : '';
      $s = $this->getTerm($s);
      $r .= $s;
      $first_p = 1;
      foreach ($ps as $p => $os) {
        $p = $this->getTerm($p);
        $r .= $first_p ? ' ' : ' ;' . $nl . str_pad('', strlen($s) + 1);
        $r .= $p;
        $first_o = 1;
        if (!is_array($os)) {/* single literal o */
          $os = array(array('val' => $os, 'type' => 'literal'));
        }
        foreach ($os as $o) {
          $r .= $first_o ? ' ' : ' ,' . $nl . str_pad('', strlen($s) + strlen($p) + 2);
          $o = $this->getTerm($o);
          $r .= $o;
          $first_o = 0;
        }
        $first_p = 0;
      }
    }
    $r .= $r ? ' .' : '';
    $r = $r ? $this->getHead() . $nl . $nl . $r : '';
    return $r;
  }
  
  /*  */

}
