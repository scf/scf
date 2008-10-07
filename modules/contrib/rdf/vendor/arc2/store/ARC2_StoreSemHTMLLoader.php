<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 Store SemHTML Loader
author:   Benjamin Nowack
version:  2007-10-08
*/

ARC2::inc('SemHTMLParser');

class ARC2_StoreSemHTMLLoader extends ARC2_SemHTMLParser {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_StoreSemHTMLLoader($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
  }

  /*  */
  
  function done() {
    $this->extractRDF();
  }
  
  function addT($t) {
    $o_literal_types = array('literal', 'literal1', 'literal2', 'literal_long1', 'literal_long2');
    $o_type = in_array($t['o_type'], $o_literal_types) ? 'literal' : $t['o_type'];
    $this->caller->addT($t['s'], $t['p'], $t['o'], $t['s_type'], $o_type, $t['o_dt'], $t['o_lang']);
    $this->t_count++;
  }

  /*  */

}
