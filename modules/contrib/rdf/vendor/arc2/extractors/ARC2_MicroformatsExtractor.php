<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 microformats Extractor
author:   Benjamin Nowack
version:  2008-04-09 (Fix: document URL was set to base URL)
*/

ARC2::inc('RDFExtractor');

class ARC2_MicroformatsExtractor extends ARC2_RDFExtractor {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_MicroformatsExtractor($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->a['ns']['foaf'] = 'http://xmlns.com/foaf/0.1/';
  }

  /*  */
  
  function getNodeContent($n) {
    $r = '';
    if (isset($n['a']['content'])) {
      $r = $n['a']['content'];
    }
    elseif (($n['tag'] == 'abbr') && isset($n['a']['title'])) {
      $r = $n['a']['title'];
    }
    else {
      $r = $this->getPlainContent($n);
    }
    return $r;
  }

  /*  */

  function getDocID($n) {
    $id = $n['id'];
    $k = 'doc_' . $id;
    if (!isset($this->caller->cache[$k])) {
      $p_ids = array($id);
      $r = $n['doc_url'];
      do  {
        $proceed = 0;
        foreach(array('hentry', 'hreview') as $cls) {
          if ($this->hasClass($n, $cls) && ($sub_r = $this->getBookmark($n))) {
            $r = $sub_r;
            break;
          }
        }
        if ($n = $this->getParentNode($n)) {
          $p_ids[] = $n['id'];
          $proceed = 1;
        }
      } while ($proceed);
      foreach ($p_ids as $p_id) {
        $this->caller->cache['doc_' . $p_id] = $r;
      }
    }
    return $this->caller->cache[$k];
  }
  
  function getDocOwnerID($n) {
    return '_:owner_of_' . $this->normalize($this->getDocID($n));
  }
  
  function getResID($n, $type = '') {
    $id = $n['id'];
    $k = 'id_' . $id;
    if (!isset($this->caller->cache[$k])) {
      $m = 'get' .ucfirst($type). 'ResLabel';
      $sub_r = method_exists($this, $m) ? $this->$m($n) : $this->getResLabel($n);
      $this->caller->cache[$k] = '_:r' . $this->normalize($n['id'] . '_' . $sub_r);
    }
    return $this->caller->cache[$k];
  }
  
  /*  */

  function getContainerResIDByClass($n, $cls, $type = '') {
    if ($pn = $this->getParentNodeByClass($n, $cls)) {
      return $this->getResID($pn, $type);
    }
    $cls_str = is_array($cls) ? join('_', $cls) : $cls;
    return preg_match('/\|page\|/', $cls_str) ? $this->getDocID($n) : '';
  }
  
  /*  */

  function getResLabel($n) {
    $id = $n['id'];
    $k = 'label_' . $id;
    if (!isset($this->caller->cache[$k])) {
      $r = '';
      if ($v = $this->v('title', '', $n['a'])) {
        $r = $v;
      }
      elseif ($v = $this->v('alt', '', $n['a'])) {
        $r = $v;
      }
      else {
        foreach (array('fn', 'summary', 'entry-title') as $cls) {
          if ($this->hasClass($n, $cls) && ($r = $this->getPlainContent($n))) {
            break;
          }
          $sub_nodes = $this->getSubNodes($n);
          foreach ($sub_nodes as $sub_n) {
            if ($sub_r = $this->getResLabel($sub_n)) {
              $r = $sub_r;
              break;
            }
          }
        }
      }
      $this->caller->cache[$k] = $r;
    }
    return $this->caller->cache[$k];
  }
  
  function getVcardResLabel($n, $type = '') {
    $id = $n['id'];
    $k = 'label_' . $id;
    if (!isset($this->caller->cache[$k])) {
      $r = '';
      foreach (array('fn', 'n', 'nickname') as $cls) {
        if ($this->hasClass($n, $cls) && ($r = $this->getPlainContent($n))) {
          break;
        }
        $sub_nodes = $this->getSubNodes($n);
        foreach ($sub_nodes as $sub_n) {
          if ($sub_r = $this->getVcardResLabel($sub_n)) {
            $r = $sub_r;
            break;
          }
        }
      }
      $this->caller->cache[$k] = $r;
    }
    return $this->caller->cache[$k];
  }
  
  function getOrgResLabel($n, $type = '') {
    $id = $n['id'];
    $k = 'label_' . $id;
    if (!isset($this->caller->cache[$k])) {
      $r = $this->getPlainContent($n);
      foreach (array('organization-name') as $cls) {
        if ($this->hasClass($n, $cls) && ($r = $this->getPlainContent($n))) {
          break;
        }
        $sub_nodes = $this->getSubNodes($n);
        foreach ($sub_nodes as $sub_n) {
          if ($sub_r = $this->getOrgResLabel($sub_n)) {
            $r = $sub_r;
            break;
          }
        }
      }
      $this->caller->cache[$k] = $r;
    }
    return $this->caller->cache[$k];
  }
  
  /*  */

  function getBookmark($n) {
    if ($this->hasRel($n, 'bookmark') && isset($n['a']['href iri'])) {
      return $n['a']['href iri'];
    }
    $sub_nodes = $this->getSubNodes($n);
    foreach ($sub_nodes as $sub_n) {
      if ($sub_r = $this->getBookmark($sub_n)) {
        return $sub_r;
      }
    }
    return 0;
  }

  /*  */

}
