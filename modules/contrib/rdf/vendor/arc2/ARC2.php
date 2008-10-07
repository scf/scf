<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 core class (static, not instantiated)
author:   Benjamin Nowack
version:  2008-04-03 (Tweaks in ARC2_RdfaExtractor
                      Fix in ARC2_MicroformatsExtractor
                      Fix in ARC2_RDFExtractor
                      Fix in ARC2_DcExtractor
                      Addition in ARC2_LegacyXMLParser
                      Addition in ARC2_SemHTMLParser)
*/

class ARC2 {

  function getVersion() {
    return '2008-04-09';
  }

  /*  */
  
  function setStatic($val) {
    static $arc_static = '';
    if ($val) $arc_static = $val;
    if (!$val) return $arc_static;
  }
  
  function getStatic() {
    return ARC2::setStatic('');
  }
  
  /*  */
  
  function getIncPath($f = '') {
    $r = realpath(dirname(__FILE__)) . '/';
    $dirs = array(
      'plugin' => 'plugins',
      'trigger' => 'triggers',
      'store' => 'store', 
      'serializer' => 'serializers', 
      'extractor' => 'extractors', 
      'parser' => 'parsers', 
    );
    foreach ($dirs as $k => $dir) {
      if (preg_match('/' . $k . '/i', $f)) {
        return $r .= $dir . '/';
      }
    }
    return $r;
  }
  
  function getScriptURI() {
    if (isset($_SERVER) && isset($_SERVER['SERVER_NAME'])) {
      return preg_replace('/^([a-z]+)\/.*$/', '\\1', strtolower($_SERVER['SERVER_PROTOCOL'])) . '://' . $_SERVER['SERVER_NAME'] . $_SERVER['SCRIPT_NAME'];
    }
    elseif (isset($_SERVER['SCRIPT_FILENAME'])) {
      return 'file://' . realpath($_SERVER['SCRIPT_FILENAME']);
    }
    return 'http://localhost/unknown_path';
  }
  
  function inc($f) {
    $prefix = 'ARC2';
    if (preg_match('/^([^\_]+)\_(.*)$/', $f, $m)) {
      $prefix = $m[1];
      $f = $m[2];
    }
    $inc_path = ARC2::getIncPath($f);
    $path = $inc_path . $prefix . '_' . urlencode($f) . '.php';
    if (file_exists($path)) {
      include_once($path);
      return 1;
    }
    if ($prefix != 'ARC2') {
      $path = $inc_path . strtolower($prefix) . '/' . $prefix . '_' . urlencode($f) . '.php';
      if (file_exists($path)) {
        include_once($path);
        return 1;
      }
    }
    return 0;
  }
  
  /*  */

  function mtime(){
    list($msec, $sec) = explode(" ", microtime());
    return ((float)$msec + (float)$sec);
  }
  
  function x($re, $v, $options = 'si') {
    return preg_match("/^\s*" . $re . "(.*)$/" . $options, $v, $m) ? $m : false;
  }

  /*  */

  function getFormat($val, $mtype = '', $ext = '') {
    ARC2::inc('getFormat');
    return ARC2_getFormat($val, $mtype, $ext);
  }
  
  function splitURI($v) {
    $parts = preg_match('/^(.*[\/\#])([^\/\#]+)$/', $v, $m) ? array($m[1], $m[2]) : array($v);
    $specials = array(
      'http://www.w3.org/XML/1998/namespace',
      //'http://www.w3.org/1999/xhtml',
    );
    foreach ($specials as $ns) {
      if (strpos($ns, $parts[0]) === 0) {
        $suffix = substr($ns, strrpos($ns, '/')+1);
        $parts[0] .= $suffix;
        $parts[1] = substr($parts[1], strlen($suffix));
      }
    }
    return $parts;
  }
  
  /*  */

  function getSimpleIndex($triples, $flatten_objects = 1, $vals = '') {
    $r = array();
    $added = array();
    foreach ($triples as $t) {
      $skip_t = 0;
      foreach (array('s', 'p', 'o') as $term) {
        $$term = $t[$term];
        /* template var */
        if (isset($t[$term . '_type']) && ($t[$term . '_type'] == 'var')) {
          $val = isset($vals[$$term]) ? $vals[$$term] : '';
          $skip_t = isset($vals[$$term]) ? $skip_t : 1;
          $type = '';
          $type = !$type && isset($vals[$$term . ' type']) ? $vals[$$term . ' type'] : $type;
          $type = !$type && preg_match('/^\_\:/', $val) ? 'bnode' : $type;
          if ($term == 'o') {
            $type = !$type && (preg_match('/\s/s', $val) || !preg_match('/\:/', $val)) ? 'literal' : $type;
            $type = !$type && !preg_match('/[\/]/', $val) ? 'literal' : $type;
          }
          $type = !$type ? 'iri' : $type;
          $t[$term . '_type'] =  $type;
          $$term = $val;
        }
      }
      if ($skip_t) {
        continue;
      }
      if (!isset($r[$s])) $r[$s] = array();
      if (!isset($r[$s][$p])) $r[$s][$p] = array();
      if ($flatten_objects) {
        if (!in_array($o, $r[$s][$p])) $r[$s][$p][] = $o;
      }
      else {
        $o = array('val' => $o);
        foreach (array('lang', 'type', 'dt') as $suffix) {
          if (isset($t['o_' . $suffix]) && $t['o_' . $suffix]) {
            $o[$suffix] = $t['o_' . $suffix];
          }
          elseif (isset($t['o ' . $suffix]) && $t['o ' . $suffix]) {
            $o[$suffix] = $t['o ' . $suffix];
          }
        }
        $id = $s . ' ' . $p . ' ' . print_r($o, 1);
        if (!isset($added[$id])) {
          $r[$s][$p][] = $o;
          $added[$id] = 1;
        }
      }
    }
    return $r;
  }
  
  function getTriplesFromIndex($index) {
    $r = array();
    foreach ($index as $s => $ps) {
      foreach ($ps as $p => $os) {
        foreach ($os as $o) {
          $r[] = array(
            's' => $s,
            'p' => $p,
            'o' => $o['val'],
            's_type' => preg_match('/^\_\:/', $s) ? 'bnode' : 'iri',
            'o_type' => $o['type'],
            'o_dt' => isset($o['dt']) ? $o['dt'] : '',
            'o_lang' => isset($o['lang']) ? $o['lang'] : '',
          );
        }
      }
    }
    return $r;
  }
  
  function getMergedIndex() {
    $r = array();
    $added = array();
    foreach (func_get_args() as $index) {
      foreach ($index as $s => $ps) {
        if (!isset($r[$s])) $r[$s] = array();
        foreach ($ps as $p => $os) {
          if (!isset($r[$s][$p])) $r[$s][$p] = array();
          foreach ($os as $o) {
            $id = md5($s . ' ' . $p . ' ' . print_r($o, 1));
            if (!isset($added[$id])) $r[$s][$p][] = $o;
            $added[$id] = 1;
          }
        }
      }
    }
    return $r;
  }
  
  function getCleanedIndex() {/* removes triples from a given index */
    $indexes = func_get_args();
    $r = $indexes[0];
    for ($i = 1, $i_max = count($indexes); $i < $i_max; $i++) {
      $index = $indexes[$i];
      foreach ($index as $s => $ps) {
        if (!isset($r[$s])) continue;
        foreach ($ps as $p => $os) {
          if (!isset($r[$s][$p])) continue;
          $r_os = $r[$s][$p];
          $new_os = array();
          foreach ($r_os as $r_o) {
            $r_o_val = is_array($r_o) ? $r_o['val'] : $r_o;
            $keep = 1;
            foreach ($os as $o) {
              $del_o_val = is_array($o) ? $o['val'] : $o;
              if ($del_o_val == $r_o_val) {
                $keep = 0;
                break;
              }
            }
            if ($keep) {
              $new_os[] = $r_o;
            }
          }
          if ($new_os) {
            $r[$s][$p] = $new_os;
          }
          else {
            unset($r[$s][$p]);
          }
        }
      }
    }
    /* check r */
    $has_data = 0;
    foreach ($r as $s => $ps) {
      if ($ps) {
        $has_data = 1;
        break;
      }
    }
    return $has_data ? $r : array();
  }
  
  /*  */
  
  function getComponent($name, $a = '') {
    ARC2::inc($name);
    $prefix = 'ARC2';
    if (preg_match('/^([^\_]+)\_(.+)$/', $name, $m)) {
      $prefix = $m[1];
      $name = $m[2];
    }
    $cls = $prefix . '_' . $name;
    return new $cls($a, new stdClass());
  }

  function getRDFParser($a = '') {
    return ARC2::getComponent('RDFParser', $a);
  }

  function getRDFXMLParser($a = '') {
    return ARC2::getComponent('RDFXMLParser', $a);
  }

  function getTurtleParser($a = '') {
    return ARC2::getComponent('TurtleParser', $a);
  }

  function getSemHTMLParser($a = '') {
    return ARC2::getComponent('SemHTMLParser', $a);
  }

  function getSPARQLParser($a = '') {
    return ARC2::getComponent('SPARQLParser', $a);
  }

  function getSPARQLPlusParser($a = '') {
    return ARC2::getComponent('SPARQLPlusParser', $a);
  }

  function getSPARQLXMLResultParser($a = '') {
    return ARC2::getComponent('SPARQLXMLResultParser', $a);
  }

  function getStore($a = '') {
    return ARC2::getComponent('Store', $a);
  }

  function getStoreEndpoint($a = '') {
    return ARC2::getComponent('StoreEndpoint', $a);
  }

  function getTurtleSerializer($a = '') {
    return ARC2::getComponent('TurtleSerializer', $a);
  }

  function getRDFXMLSerializer($a = '') {
    return ARC2::getComponent('RDFXMLSerializer', $a);
  }

  function getNTriplesSerializer($a = '') {
    return ARC2::getComponent('NTriplesSerializer', $a);
  }

  function getRDFJSONSerializer($a = '') {
    return ARC2::getComponent('RDFJSONSerializer', $a);
  }

  /*  */
  
}
