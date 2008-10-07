<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 base class
author:   Benjamin Nowack
version:  2008-01-15 (Tweak: Serializer calls (toTurtle etc.) use namespace information defined in the configuration now)
*/

class ARC2_Class {
  
  function __construct($a = '', &$caller) {
    $a = is_array($a) ? $a : array();
    $this->a = $a;
    $this->caller = &$caller;
    $this->__init();
  }
  
  function ARC2_Class($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {/* base, time_limit */
    $this->inc_path = ARC2::getIncPath();
    $this->base = $this->v('base', ARC2::getScriptURI(), $this->a);
    $this->errors = array();
    $this->warnings = array();
  }

  /*  */
  
  function v($name, $default = false, $o = false) {/* value if set */
    $o = ($o !== false) ? $o : $this;
    if (is_array($o)) {
      return isset($o[$name]) ? $o[$name] : $default;
    }
    return isset($o->$name) ? $o->$name : $default;
  }
  
  function v1($name, $default = false, $o = false) {/* value if 1 (= not empty) */
    $o = ($o !== false) ? $o : $this;
    if (is_array($o)) {
      return (isset($o[$name]) && $o[$name]) ? $o[$name] : $default;
    }
    return (isset($o->$name) && $o->$name) ? $o->$name : $default;
  }
  
  function m($name, $a = false, $default = false, $o = false) {/* call method */
    $o = ($o !== false) ? $o : $this;
    return method_exists($o, $name) ? $o->$name($a) : $default;
  }

  /*  */
  
  function addError($v) {
    if (!in_array($v, $this->errors)) {
      $this->errors[] = $v;
    }
    if ($this->caller && method_exists($this->caller, 'addError')) {
      $glue = strpos($v, ' in ') ? ' via ' : ' in ';
      $this->caller->addError($v . $glue . get_class($this));
    }
    return false;
  }
  
  function getErrors() {
    return $this->errors;
  }
  
  /*  */
  
  function splitURI($v) {
    return ARC2::splitURI($v);
  }

  /*  */

  function expandPName($v) {
    if (preg_match('/^([a-z0-9\_\-]+)\:([a-z0-9\_\-]+)$/i', $v, $m) && isset($this->ns[$m[1]])) {
      return $this->ns[$m[1]] . $m[2];
    }
    return $v;
  }

  function getPName($v) {
    if ($parts = $this->splitURI($v)) {
      foreach ($this->ns as $p => $ns) {
        if ($parts[0] == $ns) {
          return $p . ':' . $parts[1];
        }
      }
    }
    return $v;
  }

  /*  */
  
  function calcURI($path, $base = "") {
    /* quick check */
    if (preg_match("/^[a-z0-9\_]+\:/i", $path)) {/* abs path or bnode */
      return $path;
    }
    if (preg_match("/^\/\//", $path)) {/* net path, assume http */
      return 'http:' . $path;
    }
    /* other URIs */
    $base = $base ? $base : $this->base;
    $base = preg_replace('/\#.*$/', '', $base);
    if ($path === true) {/* empty (but valid) IRIref via turtle parser: <> */
      return $base;
    }
    $path = preg_replace("/^\.\//", '', $path);
    $root = preg_match('/(^[a-z0-9]+\:[\/]{1,2}[^\/]+)[\/|$]/i', $base, $m) ? $m[1] : $base; /* w/o trailing slash */
    $base .= ($base == $root) ? '/' : '';
    if (preg_match('/^\//', $path)) {/* leading slash */
      return $root . $path;
    }
    if (!$path) {
      return $base;
    }
    if (preg_match('/^([\#\?])/', $path, $m)) {
      return preg_replace('/\\' .$m[1]. '.*$/', '', $base) . $path;
    }
    if (preg_match('/^(\&)(.*)$/', $path, $m)) {/* not perfect yet */
      return preg_match('/\?/', $base) ? $base . $m[1] . $m[2] : $base . '?' . $m[2];
    }
    if (preg_match("/^[a-z0-9]+\:/i", $path)) {/* abs path */
      return $path;
    }
    /* rel path: remove stuff after last slash */
    $base = substr($base, 0, strrpos($base, '/')+1);
    /* resolve ../ */
    while (preg_match('/^(\.\.\/)(.*)$/', $path, $m)) {
      $path = $m[2];
      $base = ($base == $root.'/') ? $base : preg_replace('/^(.*\/)[^\/]+\/$/', '\\1', $base);
    }
    return $base . $path;
  }
  
  /*  */
  
  function calcBase($path) {
    $r = $path;
    $r = preg_replace('/\#.*$/', '', $r);/* remove hash */
    $r = preg_replace('/^\/\//', 'http://', $r);/* net path (//), assume http */
    if (preg_match('/^[a-z0-9]+\:/', $r)) {/* scheme, abs path */
      while (preg_match('/^(.+\/)(\.\.\/.*)$/U', $r, $m)) {
        $r = $this->calcURI($m[1], $m[2]);
      }
      return $r;
    }
    return 'file://'.realpath($r);/* rel path */
  }

  /*  */
  
  function toNTriples($v, $ns = '') {
    ARC2::inc('NTriplesSerializer');
    $conf = array('ns' => $ns ? $ns : isset($this->a['ns']) ? $this->a['ns'] : array());
    $ser = new ARC2_NTriplesSerializer($conf, $this);
    return (isset($v[0]) && isset($v[0]['s'])) ? $ser->getSerializedTriples($v) : $ser->getSerializedIndex($v);
  }
  
  function toTurtle($v, $ns = '') {
    ARC2::inc('TurtleSerializer');
    $conf = array('ns' => $ns ? $ns : isset($this->a['ns']) ? $this->a['ns'] : array());
    $ser = new ARC2_TurtleSerializer($conf, $this);
    return (isset($v[0]) && isset($v[0]['s'])) ? $ser->getSerializedTriples($v) : $ser->getSerializedIndex($v);
  }
  
  function toRDFXML($v, $ns = '') {
    ARC2::inc('RDFXMLSerializer');
    $conf = array('ns' => $ns ? $ns : isset($this->a['ns']) ? $this->a['ns'] : array());
    $ser = new ARC2_RDFXMLSerializer($conf, $this);
    return (isset($v[0]) && isset($v[0]['s'])) ? $ser->getSerializedTriples($v) : $ser->getSerializedIndex($v);
  }

  function toRDFJSON($v, $ns = '') {
    ARC2::inc('RDFJSONSerializer');
    $conf = array('ns' => $ns ? $ns : isset($this->a['ns']) ? $this->a['ns'] : array());
    $ser = new ARC2_RDFJSONSerializer($conf, $this);
    return (isset($v[0]) && isset($v[0]['s'])) ? $ser->getSerializedTriples($v) : $ser->getSerializedIndex($v);
  }

  /*  */

  function getFilledTemplate($t, $vals, $g = '') {
    $parser = ARC2::getTurtleParser();
    $parser->parse($g, $this->getTurtleHead() . $t);
    return $parser->getSimpleIndex(0, $vals);
  }
  
  function getTurtleHead() {
    $r = '';
    $ns = $this->v('ns', array(), $this->a);
    foreach ($ns as $k => $v) {
      $r .= "@prefix " . $k . ": <" .$v. "> .\n";
    }
    return $r;
  }

  /*  */

}
