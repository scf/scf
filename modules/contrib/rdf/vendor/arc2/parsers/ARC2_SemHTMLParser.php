<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 RDF/XML Parser
author:   Benjamin Nowack
version:  2008-04-09 (Addition: doc_url is set)
*/

ARC2::inc('LegacyXMLParser');

class ARC2_SemHTMLParser extends ARC2_LegacyXMLParser {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_SemHTMLParser($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {/* reader */
    parent::__init();
    $this->triples = array();
    $this->target_encoding = '';
    $this->t_count = 0;
    $this->added_triples = array();
    $this->skip_dupes = false;
    $this->bnode_prefix = $this->v('bnode_prefix', 'arc'.substr(md5(uniqid(rand())), 0, 4).'b', $this->a);
    $this->bnode_id = 0;
    $this->auto_extract = $this->v('auto_extract', 1, $this->a);
    $this->extracted_formats = array();
    $this->cache = array();
    $this->detected_formats = array();
  }
  
  /*  */

  function x($re, $v, $options = 'si') {
    return ARC2::x($re, $v, $options);
  }

  function camelCase($v) {
    $r = ucfirst($v);
    while (preg_match('/^(.*)[\-\_ ](.*)$/', $r, $m)) {
      $r = $m[1] . ucfirst($m[2]);
    }
    return $r;
  }
  
  /*  */

  function setReader(&$reader) {
    $this->reader =& $reader;
  }
  
  function createBnodeID(){
    $this->bnode_id++;
    return '_:' . $this->bnode_prefix . $this->bnode_id;
  }
  
  function addT($t) {
    if (function_exists('html_entity_decode')) {
      $t['o'] = html_entity_decode($t['o']);
    }
    if ($this->skip_dupes) {
      $h = md5(print_r($t, 1));
      if (!isset($this->added_triples[$h])) {
        $this->triples[$this->t_count] = $t;
        $this->t_count++;
        $this->added_triples[$h] = true;
      }
    }
    else {
      $this->triples[$this->t_count] = $t;
      $this->t_count++;
    }
  }

  function getTriples() {
    return $this->v('triples', array());
  }

  function countTriples() {
    return $this->t_count;
  }
  
  function getSimpleIndex($flatten_objects = 1, $vals = '') {
    return ARC2::getSimpleIndex($this->getTriples(), $flatten_objects, $vals);
  }

  /*  */

  function parse($path, $data = '') {
    $this->nodes = array();
    $this->node_count = 0;
    $this->level = 0;
    /* reader */
    if (!$this->v('reader')) {
      ARC2::inc('Reader');
      $this->reader = & new ARC2_Reader($this->a, $this);
    }
    $this->reader->setAcceptHeader('Accept: text/html, application/xhtml, */*; q=0.9');
    $this->reader->activate($path, $data);
    $this->target_encoding = $this->reader->getEncoding(false);
    $this->x_base = isset($this->a['base']) && $this->a['base'] ? $this->a['base'] : $this->reader->base;
    $this->base = $this->x_base;
    $this->doc_url = $this->reader->base;
    /* parse */
    $rest = '';
    $this->cur_tag = '';
    while ($d = $this->reader->readStream(1)) {
      $rest = $this->processData($rest . $d);
    }
    $this->reader->closeStream();
    return $this->done();
  }
  
  /*  */

  function getEncoding() {
    return $this->target_encoding;
  }

  /*  */
  
  function done() {
    if ($this->auto_extract) {
      $this->extractRDF();
    }
  }
  
  /*  */

  function processData($v) {
    $sub_v = $v;
    do {
      $proceed = 1;
      if ((list($sub_r, $sub_v) = $this->xComment($sub_v)) && $sub_r) {
        $this->open(0, 'comment', array('val' => $sub_r));
        $this->close(0, 'comment');
        continue;
      }
      if ((list($sub_r, $sub_v) = $this->xDoctype($sub_v)) && $sub_r) {
        $this->open(0, 'doctype', array('val' => $sub_r));
        $this->close(0, 'doctype');
        /* RDFa detection */
        if (preg_match('/rdfa /i', $sub_r)) $this->detected_formats['rdfa'] = 1;
        continue;
      }
      if ($this->level && ((list($sub_r, $sub_v) = $this->xWS($sub_v)) && $sub_r)) {
        $this->cData(0, $sub_r);
      }
      elseif ((list($sub_r, $sub_v) = $this->xOpen($sub_v)) && $sub_r) {
        $this->open(0, $sub_r['tag'], $sub_r['a']);
        $this->cur_tag = $sub_r['tag'];
        if ($sub_r['empty']) {
          $this->close(0, $sub_r['tag'], 1);
          $this->cur_tag = '';
        }
        /* eRDF detection */
        if (isset($sub_r['a']['profile m']) && in_array('http://purl.org/NET/erdf/profile', $sub_r['a']['profile m'])) $this->detected_formats['erdf'] = 1;
      }
      elseif ((list($sub_r, $sub_v) = $this->xClose($sub_v)) && $sub_r) {
        if (preg_match('/^(area|base|br|col|frame|hr|input|img|link|xmeta|param)$/', $sub_r['tag'])) {
          /* already implicitly closed */
        }
        else {
          $this->close(0, $sub_r['tag']);
          $this->cur_tag = '';
        }
      }
      elseif ((list($sub_r, $sub_v) = $this->xCData($sub_v)) && $sub_r) {
        $this->cData(0, $sub_r);
      }
      else {
        $proceed = 0;
      }
    } while ($proceed);
    return $sub_v;
  }

  /*  */
  
  function xComment($v) {
    if ($r = $this->x('\<\!\-\-', $v)) {
      if ($sub_r = $this->x('(.*)\-\-\>', $r[1], 'Us')) {
        return array($sub_r[1], $sub_r[2]);
      }
    }
    return array(0, $v);
  }
  
  function xDoctype($v) {
    if ($r = $this->x('\<\!DOCTYPE', $v)) {
      if ($sub_r = $this->x('([^\>]+)\>', $r[1])) {
        return array($sub_r[1], $sub_r[2]);
      }
    }
    return array(0, $v);
  }
  
  function xWS($v) {
    if ($r = $this->x('(\s+)', $v)) {
      return array($r[1], $r[2]);
    }
    return array(0, $v);
  }
  
  /*  */

  function xOpen($v) {
    if ($r = $this->x('\<([^\s\/\>]+)([^\>]*)\>', $v)) {
      list($sub_r, $sub_v) = $this->xAttributes($r[2]);
      return array(array('tag' => strtolower($r[1]), 'a' => $sub_r, 'empty' => $this->isEmpty($r[1], $r[2])), $r[3]);
    }
    return array(0, $v);
  }
  
  /*  */

  function xAttributes($v) {
    $r = array();
    while ((list($sub_r, $v) = $this->xAttribute($v)) && $sub_r) {
      if ($sub_sub_r = $this->x('xmlns\:?(.*)', $sub_r['k'])) {
        $this->nsDecl(0, $sub_sub_r[1], $sub_r['val']);
        $r['xmlns'][$sub_sub_r[1]] = $sub_r['val'];
      }
      else {
        $r[$sub_r['k']] = $sub_r['val'];
        $r[$sub_r['k'] . ' m'] = $sub_r['vals'];
      }
    }
    return array($r, $v);
  }

  /*  */

  function xAttribute($v) {
    if ($r = $this->x('([^\s\=]+)\s*(\=)?\s*([\'\"]?)', $v)) {
      if (!$r[2]) {/* no '=' */
        if ($r[1] == '/') {
          return array(0, $r[4]);
        }
        return array(array('k' => $r[1], 'val' => 1, 'vals' => array(1)), $r[4]);
      }
      if (!$r[3]) {/* no quots */
        if ($sub_r = $this->x('([^\s]+)', $r[4])) {
          return array(array('k' => $r[1], 'val' => $sub_r[1], 'vals' => array($sub_r[1])), $sub_r[2]);
        }
        return array(array('k' => $r[1], 'val' => '', 'vals' => array()), $r[4]);
      }
      $val = '';
      $multi = 0;
      $sub_v = $r[4];
      while ($sub_v && (!$sub_r = $this->x('(\x5c\\' .$r[3]. '|\\' .$r[3]. ')', $sub_v))) {
        $val .= substr($sub_v, 0, 1);
        $sub_v = substr($sub_v, 1);
      }
      $sub_v = $sub_v ? $sub_r[2] : $sub_v;
      $vals = split(' ', $val);
      return array(array('k' => $r[1], 'val' => $val, 'vals' => $vals), $sub_v);
    }
    return array(0, $v);
  }
  
  /*  */

  function isEmpty($t, $v) {
    if (preg_match('/^(area|base|br|col|frame|hr|input|img|link|xmeta|param)$/', $t)) {
      return 1;
    }
    if (preg_match('/\/$/', $v)) {
      return 1;
    }
    return 0;
  }
  
  /*  */
  
  function xClose($v) {
    if ($r = $this->x('\<\/([^\s\>]+)\>', $v)) {
      return array(array('tag' => strtolower($r[1])), $r[2]);
    }
    return array(0, $v);
  }

  /*  */
  
  function xCData($v) {
    if (preg_match('/(script|style)/i', $this->cur_tag)) {
      if ($r = $this->x('(.+)(\<\/' . $this->cur_tag . '\>)', $v, 'Uis')) {
        return array($r[1], $r[2] . $r[3]);
      }
    }
    elseif ($r = $this->x('([^\<]+)', $v)) {
      return array($r[1], $r[2]);
    }
    return array(0, $v);
  }

  /*  */

  function extractRDF($formats = '') {
    $this->node_index = $this->getNodeIndex();
    $formats = !$formats ? $this->v1('sem_html_formats', 'erdf rdfa dc openid hcard-foaf xfn rel-tag-dc', $this->a) : $formats;
    $formats = split(' ', $formats);
    foreach ($formats as $format) {
      if (!in_array($format, $this->extracted_formats)) {
        $comp = $this->camelCase($format) . 'Extractor';
        if (ARC2::inc($comp)) {
          $cls = 'ARC2_' . $comp;
          $e = new $cls($this->a, $this);
          $e->extractRDF();
        }
        $this->extracted_formats[] = $format;
      }
    }
  }
  
  function getNode($id) {
    return isset($this->nodes[$id]) ? $this->nodes[$id] : 0;
  }
  
  /*  */
  
}