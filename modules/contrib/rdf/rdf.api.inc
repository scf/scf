<?php
// $Id: rdf.api.inc,v 1.11 2008/03/15 11:42:34 arto Exp $
/**
 * rdf.api.inc - RDF API for Drupal 6.x & PHP 5.2+.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 */

//////////////////////////////////////////////////////////////////////////////
// RDF API settings

define('RDF_ARC2_PATH', drupal_get_path('module', 'rdf') . '/vendor/arc2');
define('RDF_FORMAT',    variable_get('rdf_format', 'rdf+json'));
define('RDF_SITE_URI',  url(NULL, array('absolute' => TRUE)));

rdf_define_vocabularies(); // TODO: replace with autoload-based solution

//////////////////////////////////////////////////////////////////////////////
// RDF API repository selection

/**
 * Limits future queries and operations to a particular repository.
 */
function rdf_use_repository($name = NULL) {
  if (is_null($name)) {
    unset($GLOBALS['rdf_repository']);
  }
  else {
    $GLOBALS['rdf_repository'] = $name;
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF API namespace support

function rdf_define_vocabularies() {
  $properties = rdf_rdf_properties();
  foreach (rdf_rdf_namespaces() as $prefix => $base_uri) {
    if (isset($properties[$prefix])) {
      rdf_define_vocabulary($prefix, $base_uri, $properties[$prefix]);
    }
  }
}

/**
 * Defines an RDF namespace class for predicate URI construction based on a
 * given base URI and a set of properties.
 */
function rdf_define_vocabulary($class_name, $base_uri, array $properties) {
  if (!class_exists('RDF_Namespace')) {
    eval('abstract class RDF_Namespace {}');
  }

  $code = array();
  $code[] = "abstract class $class_name extends RDF_Namespace {";
  foreach ($properties as $property) {
    $code[] = "\tconst $property = '{$base_uri}{$property}';";
  }
  $code[] = "static function uriref(\$property) { return rdf_uriref('{$base_uri}'. \$property); }";
  $code[] = '}';
  return eval(implode("\n", $code));
}

//////////////////////////////////////////////////////////////////////////////
// RDF API statement-centric queries

/**
 * Determines whether a given statement exists.
 *
 * @param $subject
 * @param $predicate
 * @param $object
 * @param $options
 * @return
 *   TRUE if the statement exists, FALSE otherwise.
 */
function rdf_exists($subject, $predicate = NULL, $object = NULL, array $options = array()) {
  return rdf_count($subject, $predicate, $object, $options) > 0;
}

/**
 * Returns the number of matching statements.
 *
 * @param $subject
 * @param $predicate
 * @param $object
 * @param $options
 * @return
 */
function rdf_count($subject, $predicate = NULL, $object = NULL, array $options = array()) {
  return count(rdf_query($subject, $predicate, $object, $options));
}

/**
 * @param $subject
 * @param $predicate
 * @param $default
 * @param $options
 */
function rdf_value($subject, $predicate, $default = NULL, array $options = array()) {
  foreach (rdf_query($subject, $predicate, NULL, $options) as $statement) {
    return $statement[2]; // object
  }
  return $default;
}

/**
 * Finds all statements matching a given triple pattern.
 *
 * @param $subject
 * @param $predicate
 * @param $object
 * @param $options
 * @return
 *   An instance of RDF_QueryIterator, yielding denormalized statements.
 */
function rdf_query($subject = NULL, $predicate = NULL, $object = NULL, array $options = array()) {
  $subject   = $subject   ? _rdf_query_arg($subject)   : $subject;
  $predicate = $predicate ? _rdf_query_arg($predicate) : $predicate;
  $repos     = isset($options['repository']) ? array($options['repository']) : NULL;

  $results   = new RDF_QueryIterator();
  foreach (_rdf_get_callbacks('query', $repos) as $callback) {
    list($callable, $callable_args) = $callback;
    $args = array($subject, $predicate, $object, $options);
    $args = !is_array($callable_args) ? $args : array_merge($callable_args, $args);
    $results->append(new IteratorIterator(new RDF_QueryCallback($callable, $args)));
  }
  return $results;
}

//////////////////////////////////////////////////////////////////////////////
// RDF API statement-centric operations

/**
 * Inserts multiple new statements.
 *
 * @param $statements
 * @return
 *   TRUE if all statements were successfully inserted, FALSE otherwise.
 */
function rdf_insert_all($statements, array $options = array()) {
  $result = TRUE;
  foreach ($statements as $statement) {
    $result = call_user_func_array('rdf_insert', array_merge($statement, array($options))) && $result;
  }
  return $result;
}

/**
 * Inserts a new statement.
 *
 * @param $subject
 * @param $predicate
 * @param $object
 * @param $options
 * @return
 *   TRUE, or a repository-specific non-NULL value, if the statement was
 *   successfully inserted; FALSE if an error occurred.
 */
function rdf_insert($subject, $predicate, $object, array $options = array()) {
  $subject   = $subject   ? _rdf_query_arg($subject)   : $subject;
  $predicate = $predicate ? _rdf_query_arg($predicate) : $predicate;
  $repos     = isset($options['repository']) ? array($options['repository']) : NULL;

  return _rdf_invoke_op('insert', array($subject, $predicate, $object, $options), $repos);
}

/**
 * Deletes an existing statement.
 *
 * @param $subject
 * @param $predicate
 * @param $object
 * @param $options
 * @return
 *   TRUE, or a repository-specific non-NULL value, if the statement was
 *   successfully deleted; FALSE if an error occurred.
 */
function rdf_delete($subject, $predicate, $object, array $options = array()) {
  $subject   = $subject   ? _rdf_query_arg($subject)   : $subject;
  $predicate = $predicate ? _rdf_query_arg($predicate) : $predicate;
  $repos     = isset($options['repository']) ? array($options['repository']) : NULL;

  return _rdf_invoke_op('delete', array($subject, $predicate, $object, $options), $repos);
}

//////////////////////////////////////////////////////////////////////////////
// RDF API data constructors

function rdf_triple($subject, $predicate, $object) {
  return func_get_args();
}

function rdf_mailto($email) {
  return rdf_uriref('mailto:' . $email);
}

function rdf_uriref($uri) {
  return RDF_URIRef::uri($uri);
}

function r($uri) { return rdf_uriref($uri); }

function rdf_bnode($id = NULL) {
  return $id ? RDF_BNode::id($id) : RDF_BNode::generate();
}

function rdf_datetime($timestamp = NULL) {
  return rdf_literal(gmstrftime('%Y-%m-%dT%H:%M:%SZ', !is_null($timestamp) ? (int)$timestamp : gmmktime()), NULL, 'xsd:dateTime');
}

function rdf_literal($value, $language = NULL, $datatype = NULL) {
  return new RDF_Literal($value, $language, $datatype);
}

function rdf_var($name) {
  return new RDF_Variable($name);
}

function rdf_is_var($value) {
  return is_object($value) && ($value instanceof RDF_Variable);
}

//////////////////////////////////////////////////////////////////////////////
// RDF API helper functions

function rdf_is_local_uri($uri) {
  $base_uri = $GLOBALS['base_url'] . '/'; // FIXME?
  return (strpos($uri, $base_uri) === 0) ? substr($uri, strlen($base_uri)) : FALSE;
}

function rdf_is_valid_uri($uri) {
  return rdf_is_valid_url($uri) || rdf_is_valid_urn($uri);
}

function rdf_is_valid_url($url) {
  static $allowed_characters = '[a-z0-9\/:_\-_\.\?\$,;~=#&%\+]';
  return preg_match("/^([a-z]+):\/\/" . $allowed_characters . "+$/i", (string)$url);
}

function rdf_is_valid_urn($urn) {
  return preg_match('/^mailto:/', $urn); // FIXME
}

function rdf_is_valid_curie($curie) {
  return preg_match('/^\[?[\w\-]+:[\w\-]*\]?$/', (string)$curie); // FIXME
}

function rdf_is_valid_qname($qname) {
  return preg_match('/^[\w\-]+:[\w\-]+$/', (string)$qname); // FIXME
}

function rdf_qname_to_uri($qname) {
  $namespaces = rdf_get_namespaces();
  list($prefix, $local_part) = explode(':', $qname, 2);
  if (isset($namespaces[$prefix])) {
    return $namespaces[$prefix] . $local_part;
  }
  return $qname; // FIXME!
}

function rdf_qname_to_uriref($qname) {
  return rdf_uriref(rdf_qname_to_uri($qname)); // TODO: cache?
}

function rdf_uri_to_qname($uri, &$namespaces, $gensym = TRUE) {
  $uri = (string)$uri;
  $best_prefix = $best_match = '';

  // Attempt to find the longest substring match
  foreach ($namespaces as $prefix => $match) {
    if (strpos($uri, $match) === 0 && strlen($match) > strlen($best_match)) {
      $best_match = $match;
      $best_prefix = $prefix;
    }
  }

  // If successful, life is easy
  if (!empty($best_prefix)) {
    $local_part = substr($uri, strlen($best_match));
    return implode(':', array($best_prefix, $local_part));
  }

  // No existing namespace prefix found, make one up
  if ($gensym && preg_match('@([\w\d-_]+)$@', $uri, $matches)) {
    static $gensym = 0;
    $prefix = 'g.' . ++$gensym; // Good ol' Lisp tradition continues...
    $local_part = $matches[1];
    $namespaces[$prefix] = substr($uri, 0, -strlen($local_part));
    return implode(':', array($prefix, $local_part));
  }

  //trigger_error('Could not convert URI ' . $uri . ' to QName', E_USER_WARNING);
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// ARC2 interoperability

function _rdf_deconstruct_arc2_triple($triple) {
  $subject   = _rdf_deconstruct_arc2_value($triple, 's');
  $predicate = _rdf_deconstruct_arc2_value($triple, 'p');
  $object    = _rdf_deconstruct_arc2_value($triple, 'o');
  return array($subject, $predicate, $object);
}

function _rdf_deconstruct_arc2_value($triple, $name) {
  switch ($name == 'p' ? 'iri' : $triple[$name .'_type']) {
    case 'iri':
      return rdf_uriref($triple[$name]);
    case 'bnode':
      return rdf_bnode($triple[$name]);
    case 'literal':
    case 'literal1':
    case 'literal2':
    case 'literal_long1':
    case 'literal_long2':
      return rdf_literal($triple[$name], $triple[$name .'_lang'], $triple[$name .'_dt']);
    case 'var':
      return rdf_var($triple[$name]);
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF/PHP normalization & serialization

function rdf_normalize($input) {
  $output = array();
  foreach ($input as $triple) {
    list($subject, $predicate, $object) = $triple;
    $output[(string)$subject][(string)$predicate][] = $object;
  }
  return $output;
}

function rdf_denormalize($input) {
  $output = array();
  foreach ($input as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      foreach ($objects as $object) {
        $output[] = array($subject, $predicate, $object);
      }
    }
  }
  return $output;
}

function rdf_objectify($input) {
  $output = array();
  foreach ($input as $s => &$ps) {
    foreach ($ps as $p => &$os) {
      foreach ($os as &$o) {
        $output[$s][$p][] = rdf_objectify_value($o);
      }
    }
  }
  return $output;
}

function rdf_objectify_value($value, &$bnodes = array()) {
  switch ($value['type']) {
    case 'bnode':
      return rdf_uriref($value['value']); // FIXME
    case 'uri':
      return rdf_uriref($value['value']);
    case 'literal':
      return !isset($value['lang']) && !isset($value['datatype']) ? $value['value'] : rdf_literal($value['value'], $value['lang'], $value['datatype']);
  }
}

function rdf_deobjectify($input) {
  $output = array();
  foreach ($input as $s => $ps) {
    foreach ($ps as $p => $os) {
      foreach ($os as $o) {
        $output[$s][$p][] = is_object($o) ? $o->to_array() : (!is_array($o) ? array('type' => 'literal', 'value' => (string)$o) : $o); // FIXME
      }
    }
  }
  return $output;
}

//////////////////////////////////////////////////////////////////////////////
// RDF/PHP serialization

function rdf_serialize($data, array $options = array()) {
  $data    = is_array($data) ? $data : rdf_normalize($data); // support RDF_QueryIterator
  $formats = rdf_get_formats('info', 'w');
  $format  = isset($options['format']) ? $options['format'] : RDF_FORMAT;
  if (isset($formats[$format]->file)) {
    require_once drupal_get_path('module', $formats[$format]->module) . '/' . $formats[$format]->file;
  }
  return isset($formats[$format]) ? call_user_func($formats[$format]->serialize, $data, $options) : FALSE;
}

function rdf_unserialize($text, array $options = array()) {
  $formats = rdf_get_formats('info', 'r');
  $format  = isset($options['format']) ? $options['format'] : RDF_FORMAT;
  if (isset($formats[$format]->file)) {
    require_once drupal_get_path('module', $formats[$format]->module) . '/' . $formats[$format]->file;
  }
  return isset($formats[$format]) ? call_user_func($formats[$format]->unserialize, $text, $options) : FALSE;
}

/**
 * @see http://n2.talis.com/wiki/RDF_JSON_Specification#rdf.2Fphp
 */
function rdf_serialize_php($data, array $options = array()) {
  return serialize(rdf_deobjectify($data));
}

/**
 * @see http://n2.talis.com/wiki/RDF_JSON_Specification#rdf.2Fphp
 */
function rdf_unserialize_php($text, array $options = array()) {
  return rdf_objectify(unserialize((string)$text));
}

/**
 * @see http://n2.talis.com/wiki/RDF_JSON_Specification
 */
function rdf_serialize_json($data, array $options = array()) {
  return drupal_to_js(rdf_deobjectify($data));
}

/**
 * @see http://n2.talis.com/wiki/RDF_JSON_Specification
 */
function rdf_unserialize_json($json, array $options = array()) {
  return rdf_objectify(json_decode((string)$json, TRUE));
}

//////////////////////////////////////////////////////////////////////////////
// RDF API query selectors

function rdf_select_resources($input) {
  return array_keys($input);
}

function rdf_select_predicates($input) {
  return !empty($input) ? array_unique(call_user_func_array('array_merge', rdf_select($input, FALSE, TRUE, FALSE))) : array();
}

function rdf_select_values($input) {
  return !empty($input) ? array_unique(call_user_func_array('array_merge', rdf_select($input, FALSE, FALSE, TRUE))) : array();
}

function rdf_select($input, $subject = TRUE, $predicate = FALSE, $object = FALSE) {
  $output = array();
  foreach ($input as $s => $ps) {
    foreach ($ps as $p => $os) {
      foreach ($os as $o) {
        $triple = array();
        if ($subject) $triple[] = $s;
        if ($predicate) $triple[] = $p;
        if ($object) $triple[] = $o;
        $output[] = $triple;
      }
    }
  }
  return $output;
}

function _rdf_filter($input, $subject = NULL, $predicate = NULL, $object = NULL, $options = array()) {
  extract($options, EXTR_SKIP | EXTR_REFS);

  $output = array();
  foreach ($input as $s => $ps) {

    foreach ($ps as $p => $os) {
      foreach ($os as $o) {
        if ((empty($callback) || $callback($s, $p, $o)) &&
            (!$subject   || $subject   == $s) &&
            (!$predicate || $predicate == $p) &&
            (!$object    || $object    == $o)) { // FIXME: RDF_Literals
          $output[$s][$p][] = $o;
        }
      }
    }

    if ($subject && $subject == $s)
      break; // shortcut
  }
  return $output;
}

//////////////////////////////////////////////////////////////////////////////
// RDF/PHP handling

// TODO: rename to rdf_predicates()?
function rdf_expand_qnames(array $input, $remove_empty = TRUE) {
  $output = array();
  foreach ($input as $qname => $data) {
    if (!empty($data)) {
      $output[rdf_qname_to_uri($qname)] = is_array($data) ? $data : array($data);
    }
  }
  return $output;
}

function rdf_get_prefixes(array $data, $namespaces = NULL) {
  $result = array('rdf' => 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
  $namespaces = is_array($namespaces) ? $namespaces : rdf_get_namespaces();

  foreach ($data as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      $qname = rdf_uri_to_qname($predicate, $namespaces);
      list($prefix, ) = explode(':', $qname);
      if (!isset($result[$prefix])) {
        $result[$prefix] = $namespaces[$prefix];
      }
    }
  }

  return $result;
}

//////////////////////////////////////////////////////////////////////////////
// RDF API functions

/**
 * Adds an RDF auto-discovery <link> tag to the page's <head> element.
 */
function rdf_add_autodiscovery_link($title, $path, $format = RDF_FORMAT) {
  $formats = rdf_get_formats();
  drupal_add_link(array('rel'   => 'meta', 'type'  => $formats[$format]->mime_type, 'title' => $title, 'href'  => $path));
}

function rdf_get_uuid() {
  return sprintf('%04x%04x-%04x-%04x-%04x-%04x%04x%04x',
    mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff),
    mt_rand(0, 0x0fff) | 0x4000,
    mt_rand(0, 0x3fff) | 0x8000,
    mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff));
}

function rdf_get_formats($what = 'info', $mode = '') {
  $formats = array();
  foreach (module_implements('rdf_formats') as $module) {
    if ($result = module_invoke($module, 'rdf_formats')) {
      foreach ($result as $k => &$v) {
        $formats[$k] = isset($formats[$k]) ? array_merge($formats[$k], $v) : $v;
        $formats[$k]['name'] = $k;
        $formats[$k]['module'] = $module;
      }
    }
  }
  ksort($formats);

  foreach ($formats as $k => &$v) {
    $formats[$k] = (object)$v;
  }

  if (preg_match('/^[rw]$/', $mode)) {
    foreach ($formats as $k => &$v) {
      if ((strpos($mode, 'r') !== FALSE && !isset($v->unserialize)) ||
          (strpos($mode, 'w') !== FALSE && !isset($v->serialize))) {
        unset($formats[$k]);
      }
    }
  }

  if ($what == 'names') {
    foreach ($formats as $k => &$v) { $formats[$k] = $v->title; }
  }

  return $formats;
}

function rdf_get_repositories($what = 'info', $filters = array()) {
  $repos = module_invoke_all('rdf_repositories');
  if (!empty($filters)) {
    foreach ($repos as $k => &$v) {
      foreach ($filters as $filter => $value) {
        if ($v[$filter] != $value) {
          unset($repos[$k]);
          continue 2;
        }
      }
    }
  }
  if ($what == 'names') {
    foreach ($repos as $k => &$v) {
      $repos[$k] = $v['title'];
    }
  }
  return $repos;
}

function rdf_get_namespaces() {
  static $namespaces = array();
  if (empty($namespaces)) {
    $namespaces = module_invoke_all('rdf_namespaces');
    ksort($namespaces);
  }
  return $namespaces;
}

function rdf_get_contexts() {
  return module_invoke_all('rdf_contexts');
}

function rdf_get_predicates($prefix) {
  $predicates = module_invoke_all('rdf_predicates');
  return isset($predicates[$prefix]) ? $predicates[$prefix] : array();
}

function rdf_get_datatype($value) {
  switch (gettype($value)) {
    case 'object':   return ($value instanceof RDF_Literal) ? $value->datatype : NULL;
    case 'NULL':     return NULL;
    case 'boolean':  return 'xsd:boolean';
    case 'integer':  return 'xsd:int';
    case 'double':   return 'xsd:double';
    case 'string':   //return 'xsd:string';
    default:         return NULL;
  }
}

function rdf_load_site() {
  return array(
    dc::type         => dcmitype::uriref('Service'),
    dc::format       => 'text/html',
    dc::language     => array_keys(language_list()),
    dc::title        => rdf_literal(variable_get('site_name', 'Drupal'), 'en'),
    dc::description  => variable_get('site_mission', ''),
    dc::publisher    => rdf_mailto(variable_get('site_mail', ini_get('sendmail_from'))),
    dc::creator      => rdf_uriref('http://drupal.org/#' . DRUPAL_CORE_COMPATIBILITY),
  );
}

//////////////////////////////////////////////////////////////////////////////
// Miscellaneous

function _rdf_query_arg($uri_or_qname) {
  if (is_string($uri_or_qname) && preg_match('/^[\w]+:[\w]+$/i', $uri_or_qname))
    return rdf_qname_to_uri($uri_or_qname);
  return is_object($uri_or_qname) ? (string)$uri_or_qname : $uri_or_qname;
}

function _rdf_invoke_op($name, $args, $repos = NULL, $default = FALSE) {
  call_user_func_array('module_invoke_all', array_merge(array('rdf'), $args)); // trigger hook_rdf()

  foreach (_rdf_get_callbacks($name, $repos) as $callback) {
    list($callable, $callable_args) = $callback;
    if ($result = call_user_func_array($callable, array_merge($callable_args, $args))) {
      return $result;
    }
  }
  return $default;
}

function _rdf_get_callbacks($op, $repos = NULL) {
  $callbacks = array();
  $repos = !empty($repos) ? $repos : (isset($GLOBALS['rdf_repository']) ? array($GLOBALS['rdf_repository']) : NULL);
  $repos = !empty($repos) ? array_intersect_key(rdf_get_repositories(), array_flip($repos)) : rdf_get_repositories();

  foreach ($repos as $repo) {
    if (isset($repo['callbacks'][$op])) {
      $callback = $repo['callbacks'][$op]['function'];
      if (is_callable($callback)) {
        $args = is_array($repo['callbacks'][$op]['arguments']) ? $repo['callbacks'][$op]['arguments'] : array();
        $callbacks[] = array($callback, $args);
      }
    }
  }

  return $callbacks;
}

//////////////////////////////////////////////////////////////////////////////
// RDF API classes

/**
 * @see http://www.php.net/~helly/php/ext/spl/interfaceIteratorAggregate.html
 */
class RDF_QueryCallback implements IteratorAggregate {
  public function __construct($callback, array $args = array()) {
    $this->callback = $callback;
    $this->args = $args;
  }

  public function call() {
    return call_user_func_array($this->callback, $this->args);
  }

  public function getIterator() {
    $result = $this->call();
    return is_object($result) ? $result : new ArrayIterator(is_array($result) ? $result : array());
  }
}

/**
 * @see http://www.php.net/~helly/php/ext/spl/classAppendIterator.html
 */
class RDF_QueryIterator extends AppendIterator {
  public function __construct() {
    parent::__construct();
    $this->rewind();
    foreach (func_get_args() as $iterator) {
      $this->append($iterator);
    }
  }

  public function key() {
    // By reindexing the aggregated results, we guarantee that e.g.
    // iterator_to_array() will work correctly despite overlapping keys
    // likely being returned by the inner iterators.
    return $this->index++;
  }

  public function rewind() {
    $this->index = 0;
    return parent::rewind();
  }

  public function to_array() {
    return iterator_to_array($this);
  }
}

class RDF_Statement {
  public $subject, $predicate, $object;

  public function __construct($subject, $predicate, $object) {
    foreach (get_defined_vars() as $k => $v) {
      $this->$k = $v;
    }
  }

  public function to_n3() {
    // TODO
  }
}

/**
 * @see http://www.w3.org/TR/rdf-concepts/#section-Graph-URIref
 */
class RDF_URIRef {
  public static $resources = array();
  public $uri;

  public static function uri($uri) {
    if (is_object($uri) && $uri instanceof RDF_URIRef) {
      return $uri; // for convenience
    }
    if (!array_key_exists($uri, self::$resources)) {
      $resource = new RDF_URIRef($uri);
      self::$resources[$uri] = $resource;
      return $resource;
    }
    return self::$resources[$uri];
  }

  public function to_array() {
    return array('type' => 'uri', 'value' => $this->uri);
  }

  public function to_n3() {
    return '<' . $this->uri . '>';
  }

  public function __toString() {
    return $this->uri;
  }

  protected function __construct($uri) {
    $this->uri = $uri;
  }
}

/**
 * @see http://www.w3.org/TR/rdf-concepts/#section-blank-nodes
 */
class RDF_BNode extends RDF_URIRef {
  public static function generate() {
    return self::id(rdf_get_uuid());
  }

  public static function id($id) {
    return self::uri('http://bnode.net/'. $id);
  }

  public function to_array() {
    return array('type' => 'bnode', 'value' => $this->uri);
  }

  public function to_n3() {
    return '_:' . substr($this->uri, strlen('http://bnode.net/')); // FIXME
  }
}

/**
 * @see http://www.w3.org/TR/rdf-concepts/#section-Graph-Literal
 */
class RDF_Literal {
  public $value, $language, $datatype;

  public function __construct($value, $language = NULL, $datatype = NULL) {
    $this->value = $value;
    $this->language = $language ? strtolower($language) : NULL;
    $this->datatype = $datatype ? rdf_qname_to_uri($datatype) : NULL;
  }

  public function to_array() {
    $array = array('type' => 'literal', 'value' => $this->value);
    if ($this->language) {
      $array['lang'] = $this->language;
    }
    if ($this->datatype) {
      $array['datatype'] = $this->datatype;
    }
    return $array;
  }

  public function to_n3() {
    return $this->__toString();
  }

  public function __toString() {
    $namespaces = rdf_get_namespaces();
    return '"'. $this->value .'"'. // FIXME
      ($this->language ? '@'. $this->language : '') .
      ($this->datatype ? '^^<'. rdf_uri_to_qname($this->datatype, $namespaces) .'>' : '');
  }
}

/**
 * @see http://www.w3.org/TR/rdf-concepts/#section-XMLLiteral
 */
class RDF_XMLLiteral {} // TODO

class RDF_Variable {
  public $name;

  function __construct($name) {
    $this->name = $name;
  }

  function to_n3() {
    return $this->__toString();
  }

  function __toString() {
    return '?' . $this->name;
  }
}
