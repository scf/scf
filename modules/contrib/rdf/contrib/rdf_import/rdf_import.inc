<?php
// $Id: rdf_import.inc,v 1.2 2008/02/20 20:59:02 arto Exp $
/**
 * rdf_import.inc - RDF Import unserialization formats.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 * @subpackage rdf_import.module
 */

//////////////////////////////////////////////////////////////////////////////
// RDF/XML input

function rdf_unserialize_xml($text, array $options = array()) {
  $parser = ARC2::getRDFXMLParser();
  $parser->parse(@$options['uri'], $text);
  return array_map('_rdf_deconstruct_arc2_triple', $parser->getTriples());
}

//////////////////////////////////////////////////////////////////////////////
// Turtle input

function rdf_unserialize_turtle($text, array $options = array()) {
  $parser = ARC2::getTurtleParser();
  $parser->parse(@$options['uri'], $text);
  return array_map('_rdf_deconstruct_arc2_triple', $parser->getTriples());
}

//////////////////////////////////////////////////////////////////////////////
// N-Triples input

function rdf_unserialize_ntriples($text, array $options = array()) {
  return rdf_unserialize_turtle($text, $options);
}
