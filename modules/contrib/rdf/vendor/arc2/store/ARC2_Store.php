<?php
/*
homepage: http://arc.semsol.org/
license:  http://arc.semsol.org/license

class:    ARC2 RDF Store
author:   Benjamin Nowack
version:  2008-04-03 (Tweak: Changed locking approach from "LOCK TABLE" to "GET LOCK")
*/

ARC2::inc('Class');

class ARC2_Store extends ARC2_Class {

  function __construct($a = '', &$caller) {
    parent::__construct($a, $caller);
  }
  
  function ARC2_Store($a = '', &$caller) {
    $this->__construct($a, $caller);
  }

  function __init() {/* db_con */
    parent::__init();
    $this->table_lock = 0;
    $this->triggers = $this->v('store_triggers', array(), $this->a);
  }

  /*  */
  
  function getName() {
    return $this->v('store_name', 'arc', $this->a);
  }

  function getTablePrefix() {
    if (!isset($this->tbl_prefix)) {
      $r = $this->v('db_table_prefix', '', $this->a);
      $r .= $r ? '_' : '';
      $r .= $this->getName() . '_';
      $this->tbl_prefix = $r;
    }
    return $this->tbl_prefix;;
  }

  /*  */
  
  function createDBCon() {
    foreach (array('db_host' => 'localhost', 'db_user' => '', 'db_pwd' => '', 'db_name' => '') as $k => $v) {
      $this->a[$k] = $this->v($k, $v, $this->a);
    }
    if (!$db_con = mysql_connect($this->a['db_host'], $this->a['db_user'], $this->a['db_pwd'])) {
      return $this->addError(mysql_error());
    }
    $this->a['db_con'] =& $db_con;
    if (!mysql_select_db($this->a['db_name'], $db_con)) {
      return $this->addError(mysql_error());
    }
    if ($this->getDBVersion() >= '04-01-00') {
      mysql_query("SET NAMES 'utf8'");
    }
    return true;
  }
  
  function getDBCon() {
    if (!isset($this->a['db_con'])) {
      if (!$this->createDBCon()) {
        return false;
      }
    }
    return $this->a['db_con'];
  }
  
  function closeDBCon() {
    if ($this->v('db_con', false, $this->a)) {
      @mysql_close($this->a['db_con']);
      unset($this->a['db_con']);
    }
  }
  
  function getDBVersion() {
    if (!$this->v('db_version')) {
      $this->db_version = preg_match("/^([0-9]+)\.([0-9]+)\.([0-9]+)/", mysql_get_server_info(), $m) ? sprintf("%02d-%02d-%02d", $m[1], $m[2], $m[3])  : '00-00-00';
    }
    return $this->db_version;
  }
  
  /*  */

  function getTables() {
    return array('triple', 'g2t', 'id2val', 's2val', 'o2val', 'setting');
    return array('triple', 'triple_backup', 'g2t', 'id2val', 's2val', 'o2val', 'setting');
  }  
  
  /*  */

  function isSetUp() {
    if ($con = $this->getDBCon()) {
      $tbl = $this->getTablePrefix() . 'setting';
      return mysql_query("SELECT 1 FROM " . $tbl . " LIMIT 0") ? 1 : 0;
    }
  }
  
  function setUp($force = 0) {
    if (($force || !$this->isSetUp()) && ($con = $this->getDBCon())) {
      if ($this->getDBVersion() < '04-00-04') {
        /* UPDATE + JOINs */
        return $this->addError('MySQL version not supported. ARC requires version 4.0.4 or higher.');
      }
      ARC2::inc('StoreTableManager');
      $mgr = new ARC2_StoreTableManager($this->a, $this);
      $mgr->createTables();
    }
  }
  
  /*  */
  
  function hasSetting($k) {
    $tbl = $this->getTablePrefix() . 'setting';
    $sql = "SELECT val FROM " . $tbl . " WHERE k = '" .md5($k). "'";
    $rs = mysql_query($sql, $this->getDBCon());
    return ($rs && ($row = mysql_fetch_array($rs))) ? 1 : 0;
  }
  
  function getSetting($k, $default = 0) {
    $tbl = $this->getTablePrefix() . 'setting';
    $sql = "SELECT val FROM " . $tbl . " WHERE k = '" .md5($k). "'";
    $rs = mysql_query($sql, $this->getDBCon());
    if ($rs && ($row = mysql_fetch_array($rs))) {
      return unserialize($row['val']);
    }
    return $default;
  }
  
  function setSetting($k, $v) {
    $con = $this->getDBCon();
    $tbl = $this->getTablePrefix() . 'setting';
    if ($this->hasSetting($k)) {
      $sql = "UPDATE " .$tbl . " SET val = '" . mysql_real_escape_string(serialize($v)) . "' WHERE k = '" . md5($k) . "'";
    }
    else {
      $sql = "INSERT INTO " . $tbl . " (k, val) VALUES ('" . md5($k) . "', '" . mysql_real_escape_string(serialize($v)) . "')";
    }
    return mysql_query($sql);
  }
  
  function removeSetting($k) {
    $tbl = $this->getTablePrefix() . 'setting';
    return mysql_query("DELETE FROM " . $tbl . " WHERE k = '" . md5($k) . "'", $this->getDBCon());
  }
  
  /*  */

  function reset($keep_settings = 0) {
    $con = $this->getDBCon();
    $tbls = $this->getTables();
    $prefix = $this->getTablePrefix();
    foreach ($tbls as $tbl) {
      if ($keep_settings && ($tbl == 'setting')) {
        continue;
      }
      mysql_query('TRUNCATE ' . $prefix . $tbl);
    }
  }
  
  function drop() {
    $con = $this->getDBCon();
    $tbls = $this->getTables();
    $prefix = $this->getTablePrefix();
    foreach ($tbls as $tbl) {
      mysql_query('DROP TABLE ' . $prefix . $tbl);
    }
  }
  
  function insert($doc, $g, $keep_bnode_ids = 0) {
    $doc = is_array($doc) ? $this->toTurtle($doc) : $doc;
    $infos = array('query' => array('url' => $g, 'target_graph' => $g));
    ARC2::inc('StoreLoadQueryHandler');
    $h =& new ARC2_StoreLoadQueryHandler($this->a, $this);
    $r = $h->runQuery($infos, $doc, $keep_bnode_ids); 
    $this->processTriggers('insert', $infos);
    return $r;
  }
  
  function delete($doc, $g) {
    if (!$doc) {
      $infos = array('query' => array('target_graphs' => array($g)));
      ARC2::inc('StoreDeleteQueryHandler');
      $h =& new ARC2_StoreDeleteQueryHandler($this->a, $this);
      $r = $h->runQuery($infos);
      $this->processTriggers('delete', $infos);
      return $r;
    }
  }
  
  function replace($doc, $g, $doc_2) {
    return array($this->delete($doc, $g), $this->insert($doc_2, $g));
  }
  
  /*  */
  
  function query($q, $result_format = '', $src = '', $keep_bnode_ids = 0, $log_query = 0) {
    if ($log_query) $this->logQuery($q);
    $con = $this->getDBCon();
    ARC2::inc('SPARQLPlusParser');
    $p = & new ARC2_SPARQLPlusParser($this->a, $this);
    $p->parse($q, $src);
    $infos = $p->getQueryInfos();
    if ($result_format == 'infos') return $infos;
    $infos['result_format'] = $result_format;
    if (!$p->getErrors()) {
      $qt = $infos['query']['type'];
      if (!in_array($qt, array('select', 'ask', 'describe', 'construct', 'load', 'insert', 'delete'))) {
        return $this->addError('Unsupported query type "'.$qt.'"');
      }
      $t1 = ARC2::mtime();
      $r = array('result' => $this->runQuery($infos, $qt, $keep_bnode_ids));
      $t2 = ARC2::mtime();
      $r['query_time'] = $t2 - $t1;
      /* query result */
      if ($result_format == 'raw') {
        return $r['result'];
      }
      if ($result_format == 'rows') {
        return $r['result']['rows'] ? $r['result']['rows'] : array();
      }
      if ($result_format == 'row') {
        return $r['result']['rows'] ? $r['result']['rows'][0] : array();
      }
      return $r;
    }
    return 0;
  }

  function runQuery($infos, $type, $keep_bnode_ids = 0) {
    ARC2::inc('Store' . ucfirst($type) . 'QueryHandler');
    $cls = 'ARC2_Store' . ucfirst($type) . 'QueryHandler';
    $h =& new $cls($this->a, $this);
    $r = $h->runQuery($infos, $keep_bnode_ids);
    $trigger_r = $this->processTriggers($type, $infos);
    return $r;
  }
  
  function processTriggers($type, $infos) {
    $r = array();
    $trigger_defs = $this->triggers;
    $this->triggers = array();
    if ($triggers = $this->v($type, array(), $trigger_defs)) {
      $r['trigger_results'] = array();
      $triggers = is_array($triggers) ? $triggers : array($triggers);
      foreach ($triggers as $trigger) {
        $trigger .= !preg_match('/Trigger$/', $trigger) ? 'Trigger' : '';
        if (ARC2::inc(ucfirst($trigger))) {
          $cls = 'ARC2_' . ucfirst($trigger);
          $config = array_merge($this->a, array('query_infos' => $infos));
          $trigger_obj = new $cls($config, $this);
          if (method_exists($trigger_obj, 'go')) {
            $r['trigger_results'][] = $trigger_obj->go();
          }
        }
      }
    }
    $this->triggers = $trigger_defs;
    return $r;
  }
  
  /*  */
  
  function getTermID($val, $term = '', $id_col = 'cid') {
    $tbl = preg_match('/^(s|o)$/', $term) ? $term . '2val' : 'id2val';
    $col = preg_match('/^(s|o)$/', $term) ? $id_col : 'id';
    $con = $this->getDBCon();
    $sql = "SELECT " . $col . " AS id FROM " . $this->getTablePrefix() . $tbl . " WHERE val = BINARY '" . mysql_real_escape_string($val) . "' LIMIT 1";
    if (($rs = mysql_query($sql)) && mysql_num_rows($rs) && ($row = mysql_fetch_array($rs))) {
      return $row['id'];
    }
    return 0;
  }

  /*  */
  
  function getLock($t_out = 10, $t_out_init = '') {
    if (!$t_out_init) $t_out_init = $t_out;
    $con = $this->getDBCon();
    $l_name = $this->a['db_name'] . '.' . $this->getTablePrefix() . '.write_lock';
    if ($rs = mysql_query('SELECT IS_FREE_LOCK("' . $l_name. '") AS success')) {
      $row = mysql_fetch_array($rs);
      if (!$row['success']) {
        if ($t_out) {
          sleep(1);
          return $this->getLock($t_out - 1, $t_out_init);
        }
      }
      elseif ($rs = mysql_query('SELECT GET_LOCK("' . $l_name. '", ' . $t_out_init. ') AS success')) {
        $row = mysql_fetch_array($rs);
        return $row['success'];
      }
    }
    return 0;   
  }
  
  function releaseLock() {
    $con = $this->getDBCon();
    return mysql_query('DO RELEASE_LOCK("' . $this->a['db_name'] . '.' . $this->getTablePrefix() . '.write_lock")');
  }

  /*  */

  function optimizeTables($level = 2) {/* 1: triple + g2t, 2: triple + *2val, 3: all tables */
    $con = $this->getDBCon();
    $pre = $this->getTablePrefix();
    $tbls = $this->getTables();
    $sql = '';
    foreach ($tbls as $tbl) {
      if (($level < 3) && preg_match('/(backup|setting)$/', $tbl)) continue;
      if (($level < 2) && preg_match('/(val)$/', $tbl)) continue;
      $sql .= $sql ? ', ' : 'OPTIMIZE TABLE ';
      $sql .= $pre . $tbl;
    }
    mysql_query($sql);
    if ($err = mysql_error()) $this->addError($err . ' in ' . $sql);
  }

  /*  */

  function isConsolidated($after = 0) {
    return $this->getSetting('store_consolidation_uts') > $after ? 1 : 0;
  }
  
  function consolidate($res = '') {
    ARC2::inc('StoreInferencer');
    $c = new ARC2_StoreInferencer($this->a, $this);
    return $c->consolidate($res);
  }
  
  function consolidateIFP($ifp, $res = '') {
    ARC2::inc('StoreInferencer');
    $c = new ARC2_StoreInferencer($this->a, $this);
    return $c->consolidateIFP($ifp, $res);
  }
  
  function inferLabels($res = '') {
    ARC2::inc('StoreInferencer');
    $c = new ARC2_StoreInferencer($this->a, $this);
    return $c->inferLabels($res);
  }
  
  /*  */
  
  function changeNamespaceURI($old_uri, $new_uri) {
    ARC2::inc('StoreHelper');
    $c = new ARC2_StoreHelper($this->a, $this);
    return $c->changeNamespaceURI($old_uri, $new_uri);
  }
  
  /*  */
  
  function getResourceLabel($res) {
    $q = '
      SELECT ?label WHERE {
        <' . $res . '> ?p ?label .
        FILTER REGEX(str(?p), "(name|label|title|summary|nick|fn)$", "i") 
      }
      LIMIT 5
    ';
    $r = '';
    if ($rows = $this->query($q, 'rows')) {
      foreach ($rows as $row) {
        $r = strlen($row['label']) > strlen($r) ? $row['label'] : $r;
      }
    }
    if (!$r && preg_match('/^\_\:/', $res)) {
      return 'An unnamed resource';
    }
    return $r ? $r : preg_replace("/^(.*[\/\#])([^\/\#]+)$/", '\\2', $res);
  }
  
  function getResourcePredicates($res) {
    $r = array();
    if ($rows = $this->query('SELECT DISTINCT ?p WHERE { <' . $res . '> ?p ?o . }', 'rows')) {
      foreach ($rows as $row) {
        $r[$row['p']] = array();
      }
    }
    return $r;
  }
  
  /*  */
  
  function logQuery($q) {
    $fp = @fopen("arc_query_log.txt", "a");
    fwrite($fp, date('Y-m-d\TH:i:s\Z', time()) . ' : ' . $q . '' . "\n\n");
    @fclose($fp);
  }

  /*  */

}
