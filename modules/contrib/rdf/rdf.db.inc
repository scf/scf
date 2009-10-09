<?php
// $Id: rdf.db.inc,v 1.15 2009/03/25 05:28:34 arto Exp $

//////////////////////////////////////////////////////////////////////////////

define('RDF_DB_TABLE_DEFAULT', 'rdf_data');
define('RDF_DB_TABLE_PREFIX',  'rdf_data_');
define('RDF_DB_CHECK_DUPLICATES', TRUE);

//////////////////////////////////////////////////////////////////////////////
// RDF API hooks

/**
 * Implementation of hook_rdf_namespaces().
 */
function rdf_db_rdf_namespaces() {
  $namespaces = array();
  $result = db_query('SELECT v.prefix, v.uri FROM {rdf_namespaces} v');
  while ($row = db_fetch_object($result)) {
    $namespaces[$row->prefix] = $row->uri;
  }
  return $namespaces;
}

/**
 * Implementation of hook_rdf_repositories().
 */
function rdf_db_rdf_repositories() {
  $repos = array();
  foreach (rdf_db_get_repository_tables() as $name => $table) {
    $repos[$name] = array(
      'title'      => rdf_db_load_repository($name)->title,
      'type'       => 'local',
      'persistent' => TRUE,
      'mutable'    => TRUE,
      'enabled'    => TRUE,
      'statements' => rdf_db_count_repository_triples($name != 'local' ? $name : NULL),
      'module'     => 'rdf',
      'callbacks'  => array(
        'insert'   => array('function' => 'rdf_db_rdf_insert', 'arguments' => array($table)),
        'delete'   => array('function' => 'rdf_db_rdf_delete', 'arguments' => array($table)),
        'query'    => array('function' => 'rdf_db_rdf_query',  'arguments' => array($table)),
        'flush'    => array('function' => 'rdf_db_rdf_flush',  'arguments' => array($table)),
      ),
    );
  }
  return $repos;
}

/**
 * Implementation of hook_rdf_contexts().
 */
function rdf_db_rdf_contexts() {
  $contexts = array();
  foreach (rdf_db_get_repository_tables() as $name => $table) {
    $result = db_query('SELECT DISTINCT g.uri g FROM {' . db_escape_table($table) . '} d INNER JOIN {rdf_resources} g ON d.gid = g.rid');
    while ($row = db_fetch_object($result)) {
      $contexts[$row->g] = TRUE;
    }
  }
  return array_keys($contexts);
}

/**
 * Implementation of hook_rdf_insert().
 */
function rdf_db_rdf_insert($table, $subject, $predicate, $object, array $options = array()) {
  global $user;
  $options = array_merge(array('uid' => $user->uid, 'created' => time()), $options);
  $record  = _rdf_db_make_record($subject, $predicate, $object, $options);

  // Prevent duplicate record creation, at the administrator's discretion:
  if (variable_get('rdf_db_prevent_duplicates', FALSE)) {
    if (($id = db_result(_rdf_db_select_statements($table, $record, 1)))) {
      return (int)$id; // previously existing duplicate record found
    }
  }

  return drupal_write_record($table, $record) ? (int)$record->did : FALSE;
}

/**
 * Implementation of hook_rdf_delete().
 */
function rdf_db_rdf_delete($table, $subject, $predicate, $object, array $options = array()) {
  $schema = rdf_db_get_schema($table);
  $record = _rdf_db_make_record($subject, $predicate, $object, $options);

  $conditions = $values = array();
  foreach (get_object_vars($record) as $key => $value) {
    if (!is_null($value)) {
      $conditions[] = $key . ' = ' . db_type_placeholder($schema['fields'][$key]['type']);
      $values[] = $value;
    }
  }

  return !!db_query('DELETE FROM {' . $table . '}' . (empty($conditions) ? '' : (' WHERE ' . implode(' AND ', $conditions))), $values);
}

/**
 * Implementation of hook_rdf_query().
 */
function rdf_db_rdf_query($table, $subject, $predicate, $object, array $options = array()) {
  $result = db_query(_rdf_db_sql_select($table, $subject, $predicate, $object, $options));

  $data = array();
  while ($row = db_fetch_object($result)) {
    $object = $row->o ? rdf_uri($row->o) : rdf_literal($row->data, $row->lang, $row->type);
    $data[] = array($row->s, $row->p, $object);
  }
  return $data;
}

//////////////////////////////////////////////////////////////////////////////
// RDF DB repository API

function rdf_db_load_namespace($prefix) {
  return db_fetch_object(db_query("SELECT v.* FROM {rdf_namespaces} v WHERE v.prefix = '%s'", $prefix));
}

function rdf_db_get_repository_tables($refresh = FALSE) {
  static $tables;
  if (!is_array($tables) || $refresh) {
    $result = db_query("SELECT name FROM {rdf_repositories} WHERE module = 'rdf' AND type = 'local' AND name != 'local' ORDER BY weight ASC");
    $tables = array('local' => RDF_DB_TABLE_DEFAULT);
    while ($row = db_fetch_object($result)) {
      if (db_table_exists($table = RDF_DB_TABLE_PREFIX . $row->name)) {
        $tables[$row->name] = $table;
      }
    }
  }
  return $tables;
}

function rdf_db_load_repository($name) {
  static $repos = array();
  if (!isset($repos[$name]) && ($row = db_fetch_array(db_query("SELECT * FROM {rdf_repositories} WHERE name = '%s'", $name)))) {
    $row = (object)array_merge($row, unserialize($row['options']), $row); // $row intentionally doubled, for key ordering & safety
    unset($row->options);
    $repos[$name] = $row;
  }
  return isset($repos[$name]) ? $repos[$name] : NULL;
}

function rdf_db_create_repository($name, array $options = array()) {
  $schema = rdf_db_get_schema(RDF_DB_TABLE_DEFAULT);
  db_create_table($results, db_escape_table(RDF_DB_TABLE_PREFIX . $name), $schema);

  $options = !empty($options) ? $options : array('title' => $name, 'description' => '');
  foreach (array('dc:title' => 'title', 'dc:description' => 'description') as $old_key => $new_key) {
    if (isset($options[$old_key])) {
      $options[$new_key] = $options[$old_key];
      unset($options[$old_key]);
    }
  }
  drupal_write_record('rdf_repositories', $record = (object)array(
    'name'    => $name,
    'module'  => 'rdf',
    'type'    => 'local',
    'enabled' => TRUE,
    'mutable' => TRUE,
    'weight'  => 0,
    'options' => serialize($options),
  ));

  rdf_db_get_schema(db_escape_table(RDF_DB_TABLE_PREFIX . $name), TRUE); // clear the schema cache
  return $results[0]['success'];
}

function rdf_db_update_repository($name, array $options = array()) {
  db_query("UPDATE {rdf_repositories} SET options = '%s' WHERE name = '%s'", serialize($options), $name);
}

function rdf_db_rename_repository($old_name, $new_name) {
  db_rename_table($results,
    db_escape_table(RDF_DB_TABLE_PREFIX . $old_name),
    db_escape_table(RDF_DB_TABLE_PREFIX . $new_name));
  db_query("UPDATE {rdf_repositories} SET name = '%s' WHERE name = '%s'", $new_name, $old_name);
  return $results[0]['success'];
}

function rdf_db_delete_repository($name) {
  db_drop_table($results, db_escape_table(RDF_DB_TABLE_PREFIX . $name));
  db_query("DELETE FROM {rdf_repositories} WHERE name = '%s'", $name);
  return $results[0]['success'];
}

function rdf_db_count_repository_triples($name = NULL, $refresh = FALSE) {
  static $cache = array();
  if (!isset($cache[$name]) || $refresh) {
    $table = db_escape_table(!$name ? RDF_DB_TABLE_DEFAULT : RDF_DB_TABLE_PREFIX . $name);
    $cache[$name] = db_result(db_query('SELECT COUNT(did) FROM {' . $table . '}'));
  }
  return $cache[$name];
}

function rdf_db_get_schema($table = RDF_DB_TABLE_DEFAULT, $rebuild = FALSE) {
  if (!($schema = drupal_get_schema($table, $rebuild))) {
    module_load_include('install', 'rdf');
    $rdf_schema = rdf_schema();
    $schema = isset($rdf_schema[$table]) ? $rdf_schema[$table] : array();
  }
  return $schema;
}

function rdf_db_merge_duplicate_statements($table = RDF_DB_TABLE_DEFAULT) {
  static $fields = array('gid', 'sid', 'pid', 'oid', 'tid', 'lang', 'data');
  $columns = implode(', ', $fields);

  $count = 0;
  $duplicates = db_query("SELECT COUNT(*) AS dups, $columns FROM {" . db_escape_table($table) . "} GROUP BY $columns HAVING dups > 1");
  while ($duplicate = db_fetch_array($duplicates)) {
    unset($duplicate['dups']);

    $records = _rdf_db_select_statements($table, $duplicate, 100, 1);
    while ($record = db_fetch_object($records)) {
      db_query("DELETE FROM {" . db_escape_table($table) . "} WHERE did = %d", $record->did);
      $count++;
    }
  }

  if ($count > 0) {
    watchdog('rdf', 'Deleted @count duplicated RDF statement(s) in table %table.', array('@count' => $count, '%table' => $table), WATCHDOG_NOTICE, l(t('settings'), 'admin/settings/rdf'));
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF DB helper functions

function _rdf_db_make_record($subject, $predicate, $object, array $options = array()) {
  global $user;

  $is_uri     = is_object($object) && ($object instanceof RDF_URIRef);
  $is_literal = is_object($object) && ($object instanceof RDF_Literal);

  $record = (object)array();
  $record->uid     = isset($options['uid']) ? $options['uid'] : NULL;
  $record->created = isset($options['created']) ? $options['created'] : NULL;
  $record->gid     = !empty($options['context']) ? _rdf_db_uri_to_id($options['context']) : NULL;
  $record->gid     = !empty($options['graph']) ? _rdf_db_uri_to_id($options['graph']) : $record->gid;
  $record->sid     = $subject ? _rdf_db_uri_to_id($subject) : NULL;
  $record->pid     = $predicate ? _rdf_db_uri_to_id($predicate) : NULL;
  $record->oid     = $object && $is_uri ? _rdf_db_uri_to_id((string)$object) : NULL;
  $record->tid     = $object && rdf_get_datatype($object) ? _rdf_db_uri_to_id(rdf_qname_to_uri(rdf_get_datatype($object))) : NULL;
  $record->lang    = $object && $is_literal ? $object->language : NULL;
  $record->data    = $object && !$is_uri ? ($is_literal ? $object->value : (string)$object) : NULL;

  return $record;
}

function _rdf_db_uri_to_id($uri) {
  // TODO: CURIE-to-URI normalization, and lookup caching.
  return ($id = _rdf_db_uri_to_id_select($uri)) ? $id : _rdf_db_uri_to_id_insert($uri);
}

function _rdf_db_uri_to_id_insert($uri) {
  return @drupal_write_record('rdf_resources', $record = (object)array('uri' => $uri)) !== FALSE ? (int)$record->rid : FALSE;
}

function _rdf_db_uri_to_id_select($uri) {
  return (int)db_result(db_query("SELECT r.rid FROM {rdf_resources} r WHERE r.uri = '%s'", $uri));
}

function _rdf_db_sql_select($table, $subject, $predicate, $object, $options = array()) {
  $query = $where = array();

  $sql = 'SELECT g.uri g, s.uri s, p.uri p, o.uri o, t.uri type, d.lang, d.data FROM {%s} d
            LEFT JOIN  {rdf_resources} g ON d.gid = g.rid
            INNER JOIN {rdf_resources} s ON d.sid = s.rid
            INNER JOIN {rdf_resources} p ON d.pid = p.rid
            LEFT JOIN  {rdf_resources} o ON d.oid = o.rid
            LEFT JOIN  {rdf_resources} t ON d.tid = t.rid';
  $query[] = sprintf($sql, db_escape_table($table));

  if (isset($options['graph']) || isset($options['context'])) { // TODO: should decide on one.
    $graph = !empty($options['graph']) ? $options['graph'] : $options['context'];
    $where[] = sprintf("g.uri = '%s'", db_escape_string((string)$graph));
  }

  if ($subject) {
    $where[] = sprintf("s.uri = '%s'", db_escape_string((string)$subject));
  }

  if ($predicate) {
    $where[] = sprintf("p.uri = '%s'", db_escape_string((string)$predicate));
  }

  if ($object) {
    // Assume an object means an RDF_URIRef, but convert it to a URI string
    // using duck typing so that other classes can be substituted
    // TODO: support language-tagged and datatyped literals
    $where[] = sprintf(is_object($object) ? "o.uri = '%s'" : "d.data = '%s'", db_escape_string((string)$object));
  }

  $query[] = empty($where) ? '' : 'WHERE (' . implode(') AND (', $where) . ')';
  $query[] = 'ORDER BY d.did ASC';
  return implode("\n", $query);
}

function _rdf_db_delete_statements($table, $match, $limit = NULL, $offset = NULL) {
  return _rdf_db_query_statements('DELETE', $table, $match, $limit, $offset);
}

function _rdf_db_select_statements($table, $match, $limit = NULL, $offset = NULL) {
  unset($match->uid, $match->created); // never match on optional meta-meta fields
  return _rdf_db_query_statements('SELECT did', $table, $match, $limit, $offset);
}

function _rdf_db_query_statements($query, $table, $match, $limit = NULL, $offset = NULL) {
  $query .= ' FROM {' . db_escape_table($table) . '} WHERE ';
  $values = (array)$match;
  $fields = array();
  foreach ($values as $field => &$value) {
    if ($value === NULL) {
      // special handling for NULL values:
      unset($values[$field]);
      $fields[] = "($field IS NULL)";
    }
    else {
      // 'lang' and 'data' are text fields, the rest are foreign keys
      $fields[] = ($field == 'lang' || $field == 'data') ? "($field = '%s')" : "($field = %d)";
    }
  }
  $query .= implode(' AND ', $fields) . ' ORDER BY did ASC';
  $query .= $limit  ? (' LIMIT '  . (int)$limit)  : '';
  $query .= $offset ? (' OFFSET ' . (int)$offset) : '';
  return db_query($query, $values);
}
