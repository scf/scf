<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 Extractor
author:   Benjamin Nowack
version:  2007-10-30
*/

ARC2::inc('MicroformatsExtractor');

class ARC2_RelTagDcExtractor extends ARC2_MicroformatsExtractor {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_RelTagDcExtractor($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->terms = array('tag');
    $this->a['ns']['dc'] = 'http://purl.org/dc/elements/1.1/';
    $this->containers = array(
      '|page|', 'vcard', 'vevent', 'hfeed', 'hentry', 'hreview', 'xfolkentry', 'hresume', 'adr', 'geo'
    );
    
  }

  /*  */
  
  function extractRDF() {
    $tc = 0;
    $t = '';
    $t_vals = array();
    foreach ($this->nodes as $n) {
      if ($n['tag'] != 'a') continue;
      if (!$href = $this->v('href iri', '', $n['a'])) continue;
      if (!$rels = $this->v('rel m', array(), $n['a'])) continue;
      if (!$vals = array_intersect($this->terms, $rels)) continue;
      if (in_array('category', $this->v('class m', array(), $n['a']))) continue;/* hcard/hcalendar categories */
      $parts = preg_match('/^(.*\/)([^\/]+)\/?$/', $href, $m) ? array('space' => $m[1], 'tag' => rawurldecode($m[2])) : array('space' => '', 'tag' => '');
      if ($tag = $parts['tag']) {
        $t_vals['s_' . $tc] = $this->getContainerResIDByClass($n, $this->containers);
        $t_vals['tag_' . $tc] = $tag;
        $t .= '?s_' . $tc . ' dc:subject ?tag_' . $tc . ' . ';
        $tc++;
      }
    }
    $doc = $this->getFilledTemplate($t, $t_vals, $n['doc_base']);
    $this->addTs(ARC2::getTriplesFromIndex($doc));
  }
  
  /*  */

  
}
