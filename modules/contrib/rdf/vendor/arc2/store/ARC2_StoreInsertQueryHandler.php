<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 RDF Store INSERT Query Handler
author:   Benjamin Nowack
version:  2007-08-24
*/

ARC2::inc('StoreQueryHandler');

class ARC2_StoreInsertQueryHandler extends ARC2_StoreQueryHandler {

  function __construct($a = '', &$caller) {/* caller has to be a store */
    parent::__construct($a, $caller);
  }
  
  function ARC2_StoreInsertQueryHandler($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {/* db_con */
    parent::__init();
    $this->store =& $this->caller;
  }

  /*  */
  
  function runQuery($infos, $keep_bnode_ids = 0) {
    $this->infos = $infos;
    $con = $this->store->getDBCon();
    /* insert */
    if (!$this->v('pattern', array(), $this->infos['query'])) {
      return $this->store->insert($this->infos['query']['construct_triples'], $this->infos['query']['target_graph'], $keep_bnode_ids);
    }
    else {
      ARC2::inc('StoreConstructQueryHandler');
      $h =& new ARC2_StoreConstructQueryHandler($this->a, $this->store);
      $sub_r = $h->runQuery($this->infos);
      return $this->store->insert($sub_r, $this->infos['query']['target_graph'], $keep_bnode_ids);
    }
  }
  
  /*  */

}
