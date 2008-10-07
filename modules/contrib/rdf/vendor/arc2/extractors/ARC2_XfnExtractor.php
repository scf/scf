<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 XFN Extractor
author:   Benjamin Nowack
version:  2007-10-05
*/

ARC2::inc('MicroformatsExtractor');

class ARC2_XfnExtractor extends ARC2_MicroformatsExtractor {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_XfnExtractor($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->terms = array(
      'contact', 'acquaintance', 'friend', 'met', 'co-worker', 'colleague', 'co-resident', 'neighbor',
      'child', 'parent', 'spouse', 'kin', 'muse', 'crush', 'date', 'sweetheart', 'me',
    );
    $this->a['ns']['xfn'] = 'http://gmpg.org/xfn/11#';
  }

  /*  */
  
  function extractRDF() {
    foreach ($this->nodes as $n) {
      if ($n['tag'] != 'a') continue;
      if (!$href = $this->v('href iri', '', $n['a'])) continue;
      if (!$rels = $this->v('rel m', array(), $n['a'])) continue;
      if (!$vals = array_intersect($this->terms, $rels)) continue;
      $t_vals = array(
        's' => $this->getDocOwnerID($n),
        's_page' => $this->getDocID($n),
        'o' => $this->getPersonID($n),
        'o_label' => $this->getResLabel($n),
        'o_page' => $href,
      );
      $t = '';
      foreach ($vals as $val) {
        $t .= '?s xfn:' .$val. ' ?o . ';
        $t .= ($val == 'me') ? '?s foaf:homepage ?o_page . ' : '';
      }
      if ($t) {
        $t .= '?s a foaf:Person ; foaf:homepage ?s_page . ';
        $t .= '?o a foaf:Person ; foaf:homepage ?o_page ';
        $t .= ($t_vals['o_label']) ? '; foaf:name ?o_label . ' : '. ';
        $doc = $this->getFilledTemplate($t, $t_vals, $n['doc_base']);
        $this->addTs(ARC2::getTriplesFromIndex($doc));
      }
    }
  }
  
  /*  */

  function getPersonID($n) {
    if ($this->hasClass($n, 'url') && ($id = $this->getContainerResIDByClass($n, 'vcard'))) {
      return $id;
    }
    return $this->getResID($n);
  }
  
  /*  */
  
}
