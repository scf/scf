<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 Extractor
author:   Benjamin Nowack
version:  2007-10-31
*/

ARC2::inc('MicroformatsExtractor');

class ARC2_AdrFoafExtractor extends ARC2_MicroformatsExtractor {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_AdrFoafExtractor($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
    $this->terms = array(
      /* root  */
      'adr',
      /* skipped (not supported or handled by other extractors) */

      /* props */
      'type', 'post-office-box', 'street-address', 'extended-address', 'region', 'locality', 'postal-code', 
      'country-name',
    );
    $this->a['ns']['vcard'] = 'http://www.w3.org/2001/vcard-rdf/3.0#';
  }

  /*  */
  
  function extractRDF() {
    foreach ($this->nodes as $n) {
      if (!$vals = $this->v('class m', array(), $n['a'])) continue;
      if (!in_array('adr', $vals)) continue;
      /* node  */
      $t_vals = array(
        's' => $this->getResID($n, 'adr'),
      );
      $t = '';
      /* context */
      list ($t_vals, $t) = $this->extractContext($n, $t_vals, $t);
      /* properties */
      foreach ($this->terms as $term) {
        $m = 'extract' . $this->camelCase($term);
        if (method_exists($this, $m)) {
          list ($t_vals, $t) = $this->$m($n, $t_vals, $t);
        }
      }
      /* result */
      $doc = $this->getFilledTemplate($t, $t_vals, $n['doc_base']);
      $this->addTs(ARC2::getTriplesFromIndex($doc));
    }
  }
  
  /*  */
  
  function extractSimple($n, $t_vals, $t, $cls, $prop = '') {
    if ($sub_ns = $this->getSubNodesByClass($n, $cls)) {
      $tc = 0;
      $prop = $prop ? $prop : 'vcard:' . strtoupper($cls);
      foreach ($sub_ns as $sub_n) {
        $var = $this->normalize($cls) . '_'. $tc;
        if ($t_vals[$var] = $this->getNodeContent($sub_n)) {
          $t .= '?s ' . $prop . ' ?' . $var . ' . ';
          $tc++;
        }
      }
    }
    return array($t_vals, $t);
  }

  /*  */
  
  function extractContext($n, $t_vals, $t) {
    if ($id = $this->getContainerResIDByClass($n, 'vcard', 'vcard')) {
      $t_vals['context'] = $id . '_agent';
      $t .= '?context vcard:ADR ?s . ';
    }
    return array($t_vals, $t);
  }
  
  /*  */

  function extractPostOfficeBox($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'post-office-box', 'vcard:Pobox');
  }
  
  /*  */

  function extractStreetAddress($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'street-address', 'vcard:Street');
  }
  
  /*  */

  function extractExtendedAddress($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'extended-address', 'vcard:Extadd');
  }
  
  /*  */

  function extractRegion($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'region', 'vcard:Region');
  }
  
  /*  */

  function extractLocality($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'locality', 'vcard:Locality');
  }
  
  /*  */

  function extractPostalCode($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'postal-code', 'vcard:Pcode');
  }
  
  /*  */

  function extractCountryName($n, $t_vals, $t) {
    return $this->extractSimple($n, $t_vals, $t, 'country', 'vcard:Country');
  }
  
  /*  */

  function extractType($n, $t_vals, $t) {
    if ($sub_ns = $this->getSubNodesByClass($n, 'type')) {
      foreach ($sub_ns as $sub_n) {
        $type = preg_match('/^([^\s]+)/', $this->getNodeContent($sub_n), $m) ? $m[1] : '';
        $t .= $type ? '?s rdf:type vcard:' . $type . ' . ' : '';
      }
    }
    return array($t_vals, $t);
  }
  
  /*  */

}
