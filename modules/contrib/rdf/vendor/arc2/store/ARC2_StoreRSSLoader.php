<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 Store RSS(2) Loader
author:   Benjamin Nowack
version:  2008-02-10
*/

ARC2::inc('RSSParser');

class ARC2_StoreRSSLoader extends ARC2_RSSParser {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_StoreRSSLoader($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {
    parent::__init();
  }

  /*  */
  
  function addT($t) {
    $o_literal_types = array('literal', 'literal1', 'literal2', 'literal_long1', 'literal_long2');
    $o_type = in_array($t['o_type'], $o_literal_types) ? 'literal' : $t['o_type'];
    $this->caller->addT($t['s'], $t['p'], $t['o'], $t['s_type'], $o_type, $t['o_dt'], $t['o_lang']);
    $this->t_count++;
  }

  /*  */

}
