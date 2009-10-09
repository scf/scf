<?php
// $Id: rdf.api.inc,v 1.37 2009/03/25 04:20:41 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// RDF API settings

define('RDF_ARC2_PATH',   drupal_get_path('module', 'rdf') . '/vendor/arc');
define('RDF_FORMAT',      variable_get('rdf_format', 'rdf+json'));
define('RDF_FORMAT_RSS',  variable_get('rdf_format_rss', 'rdf+xml'));
define('RDF_SITE_URI',    url(NULL, array('absolute' => TRUE)));

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
  $code[] = "static function uriref(\$property) { return rdf_uri('{$base_uri}' . \$property); }";
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
function rdf_count($subject = NULL, $predicate = NULL, $object = NULL, array $options = array()) {
  $results = rdf_query($subject, $predicate, $object, $options);
  return count(is_object($results) ? $results->to_array() : $results);
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
  $repos     = isset($options['repository']) ? $options['repository'] : NULL;

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
  $repos     = isset($options['repository']) ? $options['repository'] : NULL;

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
  $repos     = isset($options['repository']) ? $options['repository'] : NULL;

  return _rdf_invoke_op('delete', array($subject, $predicate, $object, $options), $repos);
}

//////////////////////////////////////////////////////////////////////////////
// RDF API data constructors

function rdf_triple($subject, $predicate, $object) {
  return func_get_args();
}

function rdf_mailto($email) {
  return rdf_uri('mailto:' . $email);
}

function rdf_uri($uri) {
  return RDF_URIRef::uri($uri);
}

function rdf_uriref($uri) { return rdf_uri($uri); } /* deprecated */
function r($uri)          { return rdf_uri($uri); }

function rdf_bnode($id = NULL) {
  return $id ? RDF_BNode::id($id) : RDF_BNode::generate();
}

function rdf_datetime($timestamp = NULL, $timezone = 0) {
  if (is_numeric($timestamp)) {
    $timestamp = (int)$timestamp;
  }
  else if (is_string($timestamp)) {
    $timestamp = strtotime($timestamp);
  }
  else if (is_null($timestamp)) {
    $timestamp = gmmktime();
  }
  $datetime = format_date($timestamp, 'custom', 'Y-m-d\TH:i:sO', $timezone);
  $datetime = str_replace('+0000', 'Z', $datetime); // clean up format_date() result
  $datetime = preg_replace('/([+-])(\d{2}):?(\d{2})$/', '\1\2:\3', $datetime);
  return rdf_literal($datetime, NULL, 'xsd:dateTime');
}

function rdf_literal($value, $language = NULL, $datatype = NULL) {
  $language = is_object($language) && isset($language->language) ? $language->language : $language;
  $language = ($language === TRUE) ? $GLOBALS['language']->language : $language; // for convenience
  return empty($language) && empty($datatype) ? $value : new RDF_Literal($value, $language, $datatype);
}

function rdf_var($name) {
  return new RDF_Variable($name);
}

function rdf_is_var($value) {
  return is_object($value) && ($value instanceof RDF_Variable);
}

function rdf_seq() {
  return new RDF_Seq(func_get_args());
}

function rdf_bag() {
  return new RDF_Bag(func_get_args());
}

function rdf_alt() {
  return new RDF_Alt(func_get_args());
}

function rdf_val_to_str($value) {
  return (is_object($value) && $value instanceof RDF_Literal) ? $value->value : (string)$value;
}

function rdf_str_to_val($string) {
  $value = $string;

  // Attempt to cast untyped values to the correct RDF datatypes:
  if (is_string($string)) {
    if (rdf_is_valid_uri($string)) {           // rdf:resource
      $value = rdf_uri($string);
    }
    else if (rdf_is_valid_datetime($string)) { // xsd:dateTime
      $tz = date_default_timezone_get();
      date_default_timezone_set('UTC');
      $value = rdf_datetime(strtotime($string));
      date_default_timezone_set($tz);
    }
    else if (is_numeric($string)) {            // xsd:numeric
      // TODO: should we cast to numeric types? this can be ambiguous...
    }
  }

  return $value;
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
  return preg_match('/^urn:/', $urn) || preg_match('/^mailto:/', $urn); // FIXME
}

function rdf_is_valid_curie($curie) {
  return preg_match('/^\[?[\w\-]+:[\w\-]*\]?$/', (string)$curie); // FIXME
}

function rdf_is_valid_qname($qname) {
  return preg_match('/^[\w\-]+:[\w\-]+$/', (string)$qname); // FIXME
}

function rdf_is_valid_prefix($qname) {
  $namespaces = rdf_get_namespaces();
  list($prefix,) = explode(':', $qname, 2);
  return isset($namespaces[$prefix]);
}

function rdf_is_valid_datetime($string) {
  return preg_match('/(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):?(\d{2})?/', $string);
}

function rdf_qname_to_uri($qname) {
  if (rdf_is_valid_qname($qname)) {
    $namespaces = rdf_get_namespaces();
    list($prefix, $local_part) = explode(':', $qname, 2);
    if (isset($namespaces[$prefix])) {
      return $namespaces[$prefix] . $local_part;
    }
  }
  return $qname; // FIXME!
}

function rdf_qname_to_uriref($qname) {
  return rdf_uri(rdf_qname_to_uri($qname)); // TODO: cache?
}

function rdf_uri_to_qname($uri, $gensym = TRUE, $fail = NULL) {
  global $rdf_namespaces;
  if (empty($rdf_namespaces)) {
    rdf_get_namespaces();
  }

  if (!rdf_is_valid_uri($uri = (string)$uri)) {
    return $uri;
  }

  $best_prefix = $best_match = '';

  // Attempt to find the longest substring match
  foreach ($rdf_namespaces as $prefix => $match) {
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
    $rdf_namespaces[$prefix] = substr($uri, 0, -strlen($local_part));
    return implode(':', array($prefix, $local_part));
  }

  //trigger_error('Could not convert URI ' . $uri . ' to QName', E_USER_WARNING);
  return $fail;
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
  switch ($name == 'p' ? 'iri' : $triple[$name . '_type']) {
    case 'uri':
    case 'iri':
      return rdf_uri($triple[$name]);
    case 'bnode':
      return rdf_bnode($triple[$name]);
    case 'literal':
    case 'literal1':
    case 'literal2':
    case 'literal_long1':
    case 'literal_long2':
      return rdf_literal($triple[$name], $triple[$name . '_lang'], $triple[$name . '_datatype']);
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
      if (!is_array($objects)) {
        $output[] = array($subject, $predicate, $objects);
      }
      else {
        foreach ($objects as $object) {
          $output[] = array($subject, $predicate, $object);
        }
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
      return rdf_uri($value['value']); // FIXME
    case 'uri':
      return rdf_uri($value['value']);
    case 'literal':
      return !isset($value['lang']) && !isset($value['datatype']) ? $value['value'] : rdf_literal($value['value'], $value['lang'], $value['datatype']);
  }
}

function rdf_deobjectify($input) {
  $output = array();
  foreach ($input as $s => $ps) {
    foreach ($ps as $p => $os) {
      $os = is_array($os) ? $os : array($os);
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
  if (!isset($formats[$format])) {
    return FALSE;
  }
  ob_start() && call_user_func($formats[$format]->serialize, $data, $options);
  return ob_get_clean();
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
  print serialize(rdf_deobjectify($data));
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
  print drupal_to_js(rdf_deobjectify($data));
}

/**
 * @see http://n2.talis.com/wiki/RDF_JSON_Specification
 */
function rdf_unserialize_json($json, array $options = array()) {
  return rdf_objectify(json_decode((string)$json, TRUE));
}

function rdf_unserialize_xml($text, array $options = array()) {
  $parser = ARC2::getRDFXMLParser();
  $parser->parse(@$options['uri'], $text);
  return array_map('_rdf_deconstruct_arc2_triple', $parser->getTriples());
}

function rdf_unserialize_turtle($text, array $options = array()) {
  $parser = ARC2::getTurtleParser();
  $parser->parse(@$options['uri'], $text);
  return array_map('_rdf_deconstruct_arc2_triple', $parser->getTriples());
}

function rdf_unserialize_ntriples($text, array $options = array()) {
  return rdf_unserialize_turtle($text, $options);
}

function rdf_serialize_xml($data, array $options = array()) {
  $namespaces = array_merge(rdf_get_namespaces(), is_array(@$options['namespaces']) ? @$options['namespaces'] : array());
  $namespaces = rdf_get_prefixes($data, $namespaces);

  $xml = new XMLWriter(); // requires PHP 5.1.2+
  $xml->openMemory();
  $xml->setIndent(TRUE);
  $xml->setIndentString('  ');
  $xml->startDocument('1.0', 'utf-8');
  $xml->startElement('rdf:RDF');

  // Output a default namespace prefix, if one was specified:
  $base_prefix = NULL;
  if (!empty($options['base'])) {
    $base_prefix = array_search($options['base'], $namespaces);
    $xml->writeAttribute('xmlns', $options['base']);
  }

  // Output all namespace prefixes:
  foreach ($namespaces as $prefix => $uri) {
    if (!$base_prefix || $base_prefix != $prefix) {
      $xml->writeAttribute('xmlns:' . $prefix, $uri);
    }
  }

  $bnode_map = array();
  $bnode_id = 0;
  foreach ($data as $subject => $predicates) {
    $type = 'rdf:Description';

    // Support the typed node element syntax where possible:
    foreach ($predicates as $predicate => &$objects) {
      if (($qname = rdf_uri_to_qname($predicate)) == 'rdf:type') {
        $type = rdf_uri_to_qname(array_shift($objects));
        if (empty($objects)) {
          unset($predicates[$predicate]);
        }
        break;
      }
    }
    unset($objects);

    $qname = $base_prefix ? preg_replace('/^' . preg_quote($base_prefix, '/') . ':/', '', $type) : $type;
    $xml->startElement($qname);

    if (RDF_BNode::match($subject)) {
      $xml->writeAttribute('rdf:nodeID', !isset($bnode_map[$subject]) ? ($bnode_map[$subject] = 'b' . ++$bnode_id) : $bnode_map[$subject]);
    }
    else { // RDF_URIRef
      $xml->writeAttribute('rdf:about', $subject);
    }

    foreach ($predicates as $predicate => $objects) {
      $qname = rdf_uri_to_qname($predicate);
      $qname = $base_prefix ? preg_replace('/^' . preg_quote($base_prefix, '/') . ':/', '', $qname) : $qname;

      foreach (is_array($objects) ? $objects : array($object) as $object) {

        if (!is_object($object)) { // plain literal
          $xml->writeElement($qname, (string)$object);
        }
        else if ($object instanceof RDF_Literal) { // typed literal
          $xml->startElement($qname);
          if ($object->language) {
            $xml->writeAttribute('xml:lang', $object->language);
          }
          if ($object->datatype) {
            $xml->writeAttribute('rdf:datatype', $object->datatype);
          }
          $xml->text((string)$object->value);
          $xml->endElement();
        }
        else if ($object instanceof RDF_BNode) { // blank node
          $xml->startElement($qname);
          $xml->writeAttribute('rdf:nodeID', !isset($bnode_map[(string)$object]) ? ($bnode_map[(string)$object] = 'b' . ++$bnode_id) : $bnode_map[(string)$object]);
          $xml->endElement();
        }
        else { // RDF_URIRef, or the like
          $xml->startElement($qname);
          $xml->writeAttribute('rdf:resource', (string)$object);
          $xml->endElement();
        }

      }
    }

    $xml->endElement(); // rdf:Description, or $type
  }

  $xml->endElement(); // rdf:RDF
  $xml->endDocument();
  print $xml->outputMemory(TRUE);
}

function rdf_serialize_trix($data, array $options = array()) {
  $namespaces = @$options['namespaces'];

  $xml = new XMLWriter(); // requires PHP 5.1.2+
  $xml->openMemory();
  $xml->setIndent(TRUE);
  $xml->setIndentString('  ');
  $xml->startDocument('1.0', 'utf-8');
  $xml->startElement('TriX');
  $xml->writeAttribute('xmlns', 'http://www.w3.org/2004/03/trix/trix-1/');

  $xml->startElement('graph'); // FIXME?
  foreach ($data as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      foreach ($objects as $object) {
        $xml->startElement('triple');
        $xml->writeElement('uri', (string)$subject);
        $xml->writeElement('uri', (string)$predicate);

        if (!is_object($object)) { // plain literal w/o xml:lang
          $xml->writeElement('plainLiteral', (string)$object);
        }
        else if ($object instanceof RDF_Literal) { // typed literal
          $xml->startElement(!$object->datatype ? 'plainLiteral' : 'typedLiteral');
          if ($object->language)
            $xml->writeAttribute('xml:lang', $object->language);
          if ($object->datatype)
            $xml->writeAttribute('datatype', $object->datatype);
          $xml->text((string)$object->value);
          $xml->endElement();
        }
        else { // RDF_URIRef, or the like
          $xml->writeElement('uri', (string)$object);
        }

        $xml->endElement(); // triple
      }
    }
  }
  $xml->endElement(); // graph

  $xml->endElement(); // TriX
  $xml->endDocument();
  print $xml->outputMemory(TRUE);
}

function rdf_serialize_turtle($data, array $options = array()) {
  $namespaces = @$options['namespaces'];
  $namespaces = rdf_get_prefixes($data, $namespaces);
  foreach ($namespaces as $prefix => $uri) {
    printf("@prefix %s: <%s> .\n", $prefix, $uri);
  }
  printf("\n");

  foreach ($data as $subject => $predicates) {
    printf("<%s>\n", (string)$subject);

    $new_subject = TRUE;
    foreach ($predicates as $predicate => $objects) {
      $qname = rdf_uri_to_qname($predicate);

      foreach ($objects as $object) {
        printf(!$new_subject ? " ;\n\t" : "\t");
        $new_subject = FALSE;

        if (!is_object($object)) { // plain literal
          printf('%s "%s"', $qname, (string)$object); // FIXME: string escaping
        }
        else if ($object instanceof RDF_Literal) { // typed literal
          printf('%s "%s"', $qname, (string)$object->value); // FIXME: string escaping
          if ($object->language)
            printf('@%s', $object->language);
          if ($object->datatype)
            printf('^^%s', $object->datatype);
        }
        else { // RDF_URIRef, or the like
          printf('%s <%s>', $qname, (string)$object);
        }
      }
    }

    printf(" .\n\n");
  }
}

function rdf_objects($objects) {
  return is_array($objects) ? $objects : array($objects);
}

function rdf_serialize_ntriples($data, array $options = array()) {
  $namespaces = @$options['namespaces'];

  foreach ($data as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      foreach (rdf_objects($objects) as $object) {

        printf('<%s> <%s> ', (string)$subject, (string)$predicate);

        if (!is_object($object)) { // plain literal
          printf('"%s"', _rdf_serialize_ntriples_escape((string)$object));
        }
        else if ($object instanceof RDF_Literal) { // typed literal
          printf('"%s"', _rdf_serialize_ntriples_escape((string)$object->value));
          if ($object->language)
            printf('@%s', $object->language);
          if ($object->datatype)
            printf('^^<%s>', $object->datatype);
        }
        else { // RDF_URIRef, or the like
          printf('<%s>', (string)$object);
        }

        printf(" .\n");
      }
    }
  }
}

function _rdf_serialize_ntriples_escape($string) {
  return str_replace(array('\\', '"', "\t", "\n", "\r"), array('\\\\', '\\"', '\\t', '\\n', '\\r'), $string);
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
      $qnames = array($predicate);
      foreach (is_array($objects) ? $objects : array($objects) as $object) {
        if (rdf_is_valid_uri((string)$object)) {
          $qnames[] = $object;
        }
      }
      foreach ($qnames as $qname) {
        list($prefix, ) = explode(':', rdf_uri_to_qname($qname));
        if ($prefix != '_' && !isset($result[$prefix]) && isset($namespaces[$prefix])) {
          $result[$prefix] = $namespaces[$prefix];
        }
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
function rdf_add_autodiscovery_link($title, $path, $format = RDF_FORMAT, array $options = array()) {
  $formats = rdf_get_formats();
  drupal_add_link(array_merge(array('rel' => 'meta', 'type' => $formats[$format]->mime_type, 'title' => $title, 'href' => $path), $options));
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

function rdf_register_namespace($prefix, $uri) {
  rdf_get_namespaces();
  $GLOBALS['rdf_namespaces'][$prefix] = $uri;
}

function rdf_get_namespaces($op = NULL) {
  global $rdf_namespaces;
  if (empty($rdf_namespaces)) {
    $rdf_namespaces = module_invoke_all('rdf_namespaces');
    ksort($rdf_namespaces);
  }

  switch ($op) {
    case 'rdfa': // only RDFa-enabled namespaces
      $defaults = array('rdf', 'dc', 'foaf');
      $defaults = array_combine($defaults, $defaults);
      return array_intersect_key($rdf_namespaces, array_filter(variable_get('rdf_rdfa_prefixes', $defaults), 'is_string'));
    default:     // all namespaces
      return $rdf_namespaces;
  }
}

function rdf_get_contexts() {
  return module_invoke_all('rdf_contexts');
}

function rdf_get_predicates($prefix) {
  $predicates = module_invoke_all('rdf_predicates');
  return isset($predicates[$prefix]) ? $predicates[$prefix] : array();
}

function rdf_get_type($value) {
  if (!is_object($value)) {
    return 'string'; // plain literal
  }
  else if ($value instanceof RDF_Literal) {
    return 'literal'; // language/datatype-tagged literal
  }
  else { // RDF_URIRef, or the like
    return 'uri'; // TODO: bnode
  }
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
    dc::creator      => rdf_uri('http://drupal.org/#' . DRUPAL_CORE_COMPATIBILITY),
  );
}

function rdf_get_feed_info($feed_id = NULL, $enabled_only = FALSE) {
  $feeds = module_invoke_all('rdf_feeds');
  if ($feed_id) {
    return isset($feeds[$feed_id]) ? (object)array_merge(array('id' => $feed_id), $feeds[$feed_id]) : NULL;
  }
  else {
    foreach ($feeds as $feed_id => &$feed) {
      $feeds[$feed_id] = $feed = (object)array_merge(array('id' => $feed_id), $feed);
      if ($enabled_only && !module_exists($feed->module)) {
        unset($feeds[$feed_id]);
      }
    }
    return $feeds;
  }
}

function rdf_get_feed_settings($feed) {
  $defaults = array('description_from_mission' => '', 'description' => '', 'update_period' => '', 'update_frequency' => '', 'update_base' => '', 'format' => RDF_FORMAT_RSS);
  if (($feed = is_object($feed) ? $feed : rdf_get_feed_info($feed))) {
    return ($feed->module == 'views') ? // special case for Views feeds
      rdf_views_get_feed_settings($feed->id, $defaults) :
      variable_get('rdf_feed[' . $feed->id . ']', $defaults);
  }
  return $defaults;
}

function rdf_set_feed_settings($feed, $settings) {
  if (($feed = is_object($feed) ? $feed : rdf_get_feed_info($feed))) {
    if ($settings) {
      variable_set('rdf_feed[' . $feed->id . ']', (array)$settings);
    }
    else {
      variable_del('rdf_feed[' . $feed->id . ']');
    }
  }
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
  $repos = !empty($repos) ? $repos : (isset($GLOBALS['rdf_repository']) ? $GLOBALS['rdf_repository'] : NULL);
  $repos = !empty($repos) && !is_array($repos) ? array($repos) : $repos;
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

class RDF_Callback {
  public function __construct($callback, array $arguments = array()) {
    $this->callback  = $callback;
    $this->arguments = $arguments;
  }

  public function call() {
    $arguments = func_get_args();
    return call_user_func_array($this->callback, array_merge($this->arguments, $arguments));
  }
}

/**
 * @see http://www.php.net/~helly/php/ext/spl/interfaceIterator.html
 */
class RDF_CallbackIterator implements Iterator {
  public function __construct($callback, array $args_lhs, $args_iter, array $args_rhs = array()) {
    $this->callback  = $callback;
    $this->args_lhs  = $args_lhs;
    $this->args_iter = $args_iter;
    $this->args_rhs  = $args_rhs;
    $this->done      = FALSE;
    $this->key       = NULL;
    $this->value     = NULL;
  }

  public function call($arg_iter) {
    $result = call_user_func_array($this->callback, array_merge($this->args_lhs, array($arg_iter), $this->args_rhs));
    list($this->key, $this->value) = each($result);
  }

  public function rewind()  {
    $this->done = FALSE;
    reset($this->args_iter);
    $this->next();
  }

  public function next()    {
    if (($kv = each($this->args_iter)) !== FALSE) {
      list($key, $value) = $kv;
      $this->call($value);
    }
    else {
      $this->done = TRUE;
    }
  }

  public function valid()   { return !$this->done; }
  public function current() { return $this->value; }
  public function key()     { return $this->key; }
}

/**
 * @see http://www.php.net/~helly/php/ext/spl/interfaceIteratorAggregate.html
 */
class RDF_QueryCallback extends RDF_Callback implements IteratorAggregate {
  public function call() {
    return call_user_func_array($this->callback, $this->arguments);
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

  public static function uri($uri, $class = __CLASS__) {
    if (is_object($uri) && $uri instanceof RDF_URIRef) {
      return $uri; // for convenience
    }
    if (!array_key_exists($uri, self::$resources)) {
      $resource = new $class($uri);
      self::$resources[$uri] = $resource;
      return $resource;
    }
    return self::$resources[$uri];
  }

  public function qname() {
    return rdf_uri_to_qname($this->uri);
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
  public static function match($uri) {
    return strpos($uri, 'http://bnode.net/') === 0;
  }

  public static function generate() {
    return self::id(rdf_get_uuid());
  }

  public static function id($id) {
    return self::uri('http://bnode.net/' . $id, __CLASS__);
  }

  public function qname() {
    return '_:' . $id;
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

  public function qname() {
    return rdf_uri_to_qname($this->datatype);
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
    return '"' . $this->value . '"' . // FIXME
      ($this->language ? '@' . $this->language : '') .
      ($this->datatype ? '^^<' . $this->qname() . '>' : '');
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

class RDF_Collection implements ArrayAccess {
  public $items, $uri;

  function __construct(array $items = array()) {
    $this->items = $items;
    $this->uri   = rdf_bnode();
  }

  function offsetExists($offset)      { return isset($this->items[$offset]); }
  function offsetGet($offset)         { return $this->items[$offset]; }
  function offsetSet($offset, $value) { return is_null($offset) ? $this->items[] = $value : $this->items[$offset] = $value; }
  function offsetUnset($offset)       { unset($this->items[$offset]); }

  function uriref()                   { return $this->uri; }

  function to_triples($indexed = FALSE) {
    $data = array(array($this->uri, 'rdf:type', rdf_qname_to_uriref($this->predicate())));
    $index = 0;
    foreach ($this->items as $item) {
      $data[] = array($this->uri, ($indexed ? 'rdf:_' . ++$index : 'rdf:li'), $item);
    }
    return $data;
  }
}

class RDF_Seq extends RDF_Collection {
  function predicate() { return 'rdf:Seq'; }
}

class RDF_Bag extends RDF_Collection {
  function predicate() { return 'rdf:Bag'; }
}

class RDF_Alt extends RDF_Collection {
  function predicate() { return 'rdf:Alt'; }
}
