<?php
// $Id: sparql.inc,v 1.2 2008/06/06 10:09:07 arto Exp $
/**
 * sparql.inc - SPARQL API query engine implementation.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package sparql.module
 */

//////////////////////////////////////////////////////////////////////////////
// Module settings

define('SPARQL_ENDPOINT', variable_get('sparql_endpoint', FALSE));

//////////////////////////////////////////////////////////////////////////////
// SPARQL API query constructors (DSL)

/**
 * @see http://www.w3.org/TR/rdf-sparql-query/#ask
 */
function sparql_ask() {
  $args = func_get_args();
  return call_user_func_array(array('SPARQL_Query', 'ask'), $args);
}

/**
 * @see http://www.w3.org/TR/rdf-sparql-query/#select
 */
function sparql_select() {
  $args = func_get_args();
  return call_user_func_array(array('SPARQL_Query', 'select'), $args);
}

/**
 * @see http://www.w3.org/TR/rdf-sparql-query/#construct
 */
function sparql_construct() {
  $args = func_get_args();
  return call_user_func_array(array('SPARQL_Query', 'construct'), $args);
}

/**
 * @see http://www.w3.org/TR/rdf-sparql-query/#describe
 */
function sparql_describe() {
  $args = func_get_args();
  return call_user_func_array(array('SPARQL_Query', 'describe'), $args);
}

//////////////////////////////////////////////////////////////////////////////
// SPARQL API query constructors (textual)

/**
 * Executes a SPARQL query in text form.
 */
function sparql_query($text, $options = array(), &$errors = NULL) {
  if (!isset($options['prefixes']) || $options['prefixes']) {
    $text = SPARQL_Query::prefixes() . "\n" . $text;
  }

  if (!empty($options['endpoint'])) {
    require_once drupal_get_path('module', 'sparql') . '/sparql.client.inc';
    return sparql_request($options['endpoint'], $text, $options, $errors);
  }

  return ($query = sparql_parse($text, $options, $errors)) ? $query->run() : NULL;
}

/**
 * Parses a SPARQL query in text form, returning a SPARQL_Query object.
 */
function sparql_parse($text, $options = array(), &$errors = NULL) {
  $parser = ARC2::getSPARQLParser();
  $parser->parse($text);

  if ($parser->getErrors()) {
    $errors = array();
    foreach ($parser->getErrors() as $error) {
      $errors[] = t('Malformed SPARQL query: %error.', array('%error' => $error));
    }
    return FALSE;
  }

  $ast = $parser->getQueryInfos();

  $query = call_user_func(array('SPARQL_Query', $ast['query']['type']));
  $query->distinct(!empty($ast['query']['distinct']));
  $query->reduced(!empty($ast['query']['reduced']));

  if (isset($ast['vars'])) {
    $query->vars = array_values($ast['vars']);
  }

  if (isset($ast['query']['result_vars'])) {
    foreach ($ast['query']['result_vars'] as $var) {
      // $var->val
    }
  }

  if (isset($ast['query']['result_iris'])) {
    $query->uris = $ast['query']['result_iris'];
  }

  if (isset($ast['query']['pattern'])) {
    $group = $ast['query']['pattern'];
    foreach ($group['patterns'] as $subpattern) {
      switch ($subpattern['type']) {
        case 'triples':
          foreach ($subpattern['patterns'] as $pattern) {
            $query->pattern(_rdf_deconstruct_arc2_triple($pattern));
          }
          break;
        case 'filter':
          $constraint = $subpattern['constraint'];
          //$query->filter($constraint['call']); // TODO
          break;
      }
    }
  }

  if (isset($ast['query']['order_infos'])) {
    foreach ($ast['query']['order_infos'] as $order_info) {
      if ($order_info['type'] == 'var') { // FIXME
        $query->order_by($order_info['val'], $order_info['direction']);
      }
    }
  }

  if (isset($ast['query']['offset'])) {
    $query->offset((int)$ast['query']['offset']);
  }

  if (isset($ast['query']['limit'])) {
    $query->limit((int)$ast['query']['limit']);
  }

  return $query;
}

//////////////////////////////////////////////////////////////////////////////
// SPARQL query builder

class SPARQL_Query implements IteratorAggregate {
  public $type;
  public $vars, $uris, $patterns, $filters;
  public $distinct = FALSE, $reduced = FALSE;
  public $order_by, $limit, $offset;

  static function prefixes() {
    $prefixes = array();
    foreach (rdf_get_namespaces() as $prefix => $base_uri) {
      if ($prefix != '_') { // FIXME
        $prefixes[] = "PREFIX $prefix: <$base_uri>";
      }
    }
    return implode("\n", $prefixes);
  }

  function __construct($type, array $options = array()) {
    $this->type = $type;
    foreach ($options as $k => $v) {
      $this->$k = $v;
    }
  }

  // Query forms
  // @see http://www.w3.org/TR/rdf-sparql-query/#QueryForms

  static function ask() {
    return new self('ask');
  }

  static function select() {
    $args = func_get_args();
    return new self('select', array('vars' => $args));
  }

  static function construct() {
    return new self('construct');
  }

  static function describe() {
    return new self('describe');
  }

  // Instance methods

  function distinct($value = TRUE) {
    $this->distinct = $value;
    return $this;
  }

  function reduced($value = TRUE) {
    $this->reduced = $value;
    return $this;
  }

  function pattern() {
    $this->patterns[] = (func_num_args() == 1) && is_array(func_get_arg(0)) ? func_get_arg(0) : func_get_args();
    return $this;
  }

  function filter() {
    if (!is_array($this->filters)) {
      $this->filters = array();
    }
    $this->filters[] = array();
    return $this;
  }

  function order_by($var, $sort = 'asc') {
    if (!is_array($this->order_by)) {
      $this->order_by = array();
    }
    $this->order_by[$var] = $sort;
    return $this;
  }

  function offset($offset) {
    $this->offset = $offset;
    return $this;
  }

  function limit($limit) {
    $this->limit = $limit;
    return $this;
  }

  function run() {
    require_once drupal_get_path('module', 'sparql') . '/sparql.engine.inc';
    return SPARQL_Engine::execute($this);
  }

  function getIterator() {
    return is_array(($result = $this->run())) ? new ArrayIterator($result) : $result;
  }

  function __toString() {
    // TODO
  }
}
