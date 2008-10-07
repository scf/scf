<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 RDF Store DELETE Query Handler
author:   Benjamin Nowack
version:  2008-04-03 (Tweak: Changed locking approach from "LOCK TABLE" to "GET LOCK")
*/

ARC2::inc('StoreQueryHandler');

class ARC2_StoreDeleteQueryHandler extends ARC2_StoreQueryHandler {

  function __construct($a = '', &$caller) {/* caller has to be a store */
    parent::__construct($a, $caller);
  }
  
  function ARC2_StoreDeleteQueryHandler($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {/* db_con */
    parent::__init();
    $this->store =& $this->caller;
    $this->handler_type = 'delete';
  }

  /*  */
  
  function runQuery($infos) {
    $this->infos = $infos;
    $con = $this->store->getDBCon();
    $t1 = ARC2::mtime();
    /* delete */
    if (!$this->v('construct_triples', array(), $this->infos['query'])) {
      $tc = $this->deleteTargetGraphs();
    }
    elseif (!$this->v('pattern', array(), $this->infos['query'])) {
      $tc = $this->deleteTriples();
    }
    else {
      $tc = $this->deleteConstructedGraph();
    }
    $t2 = ARC2::mtime();
    /* clean up */
    if ($tc) {
      $full = (rand(1, 100) == 1) ? 1 : 0;
      $this->cleanTableReferences($full);
    }
    if ($tc && (rand(1, 50) == 1)) $this->store->optimizeTables();
    $t3 = ARC2::mtime();
    $index_dur = round($t3 - $t2, 4);
    $dur = round($t3 - $t1, 4);
    return array(
      't_count' => $tc,
      'delete_time' => $dur,
      'index_update_time' => $index_dur,
    );
  }
  
  /*  */

  function deleteTargetGraphs() {
    $tbl_prefix = $this->store->getTablePrefix();
    $r = 0;
    foreach ($this->infos['query']['target_graphs'] as $g) {
      if ($g_id = $this->getTermID($g, 'g')) {
        $rs = mysql_query('DELETE FROM ' . $tbl_prefix . 'g2t WHERE g = ' .$g_id);
        $r += mysql_affected_rows();
      }
    }
    return $r;
  }
  
  /*  */
  
  function deleteTriples() {
    $r = 0;
    $dbv = $this->store->getDBVersion();
    $tbl_prefix = $this->store->getTablePrefix();
    /* graph restriction */
    $tgs = $this->infos['query']['target_graphs'];
    $gq = '';
    foreach ($tgs as $g) {
      if ($g_id = $this->getTermID($g, 'g')) {
        $gq .= $gq ? ', ' . $g_id : $g_id;
      }
    }
    $gq = $gq ? ' AND G.g IN (' . $gq . ')' : '';
    /* triples */
    foreach ($this->infos['query']['construct_triples'] as $t) {
      $q = '';
      $skip = 0;
      foreach (array('s', 'p', 'o') as $term) {
        if (isset($t[$term . '_type']) && preg_match('/(var)/', $t[$term . '_type'])) {
          //$skip = 1;
        }
        else {
          $term_id = $this->getTermID($t[$term], $term);
          $q .= $q ? ' AND ' : '';
          $q .= 'T.' . $term . '=' . $term_id;
        }
      }
      if ($skip) {
        continue;
      }
      if ($gq) {
        $sql = ($dbv < '04-01') ? 'DELETE ' . $tbl_prefix . 'g2t' : 'DELETE G';
        $sql .= '
          FROM ' . $tbl_prefix . 'g2t G 
          JOIN ' . $this->getTripleTable() . ' T ON (T.t = G.t' . $gq . ')
          WHERE ' . $q . '
        ';
      }
      else {
        $sql = ($dbv < '04-01') ? 'DELETE ' . $this->getTripleTable() : 'DELETE T';
        $sql .= ' FROM ' . $this->getTripleTable() . ' T WHERE ' . $q;
      }
      $rs = mysql_query($sql);
      if ($er = mysql_error()) {
        $this->addError($er .' in ' . $sql);
      }
      $r += mysql_affected_rows();
    }
    return $r;
  }
  
  /*  */
  
  function deleteConstructedGraph() {
    ARC2::inc('StoreConstructQueryHandler');
    $h =& new ARC2_StoreConstructQueryHandler($this->a, $this->store);
    $sub_r = $h->runQuery($this->infos);
    $triples = ARC2::getTriplesFromIndex($sub_r);
    $tgs = $this->infos['query']['target_graphs'];
    $this->infos = array('query' => array('construct_triples' => $triples, 'target_graphs' => $tgs));
    return $this->deleteTriples();
  }
  
  /*  */
  
  function cleanTableReferences($full = 0) {
    /* table lock */
    if (!$this->store->getLock()) return $this->addError('Could not get lock in "cleanTableReferences"');
    $con = $this->store->getDBCon();
    $tbl_prefix = $this->store->getTablePrefix();
    //$tbls = $full ? array('triple' => 'g2t', 'triple_backup' => 'triple') : array('triple' => 'g2t');
    $tbls = array('triple' => 'g2t');
    foreach ($tbls as $t1 => $t2) {
      mysql_query('UPDATE '. $tbl_prefix . $t1 . ' SET misc = 1');
      $sql = '
        UPDATE '. $tbl_prefix . $t1 . '  
        JOIN ' . $tbl_prefix . $t2 . ' ON ('. $tbl_prefix . $t1 . '.t = '. $tbl_prefix . $t2 . '.t)
        SET '. $tbl_prefix . $t1 . '.misc = 0
      ';
      mysql_query($sql);
      mysql_query('DELETE FROM '. $tbl_prefix . $t1 . ' WHERE misc = 1');
    }
    /* release lock */
    $this->store->releaseLock();
  }
  
  /*  */

}
