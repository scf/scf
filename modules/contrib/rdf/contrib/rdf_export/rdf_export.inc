<?php
// $Id: rdf_export.inc,v 1.1 2008/02/11 21:54:18 arto Exp $
/**
 * rdf_export.inc - RDF Export serialization formats.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 * @subpackage rdf_export.module
 */

//////////////////////////////////////////////////////////////////////////////
// RDF/XML output

function rdf_serialize_xml($data, array $options = array()) {
  $namespaces = @$options['namespaces'];

  $xml = new XMLWriter(); // requires PHP 5.1.2+
  $xml->openMemory();
  $xml->setIndent(TRUE);
  $xml->setIndentString('  ');
  $xml->startDocument('1.0', 'utf-8');
  $xml->startElement('rdf:RDF');

  $namespaces = rdf_get_prefixes($data, $namespaces);
  foreach ($namespaces as $prefix => $uri) {
    $xml->writeAttribute('xmlns:' . $prefix, $uri);
  }

  foreach ($data as $subject => $predicates) {
    $xml->startElement('rdf:Description');
    $xml->writeAttribute('rdf:about', $subject);

    foreach ($predicates as $predicate => $objects) {
      $qname = rdf_uri_to_qname($predicate, $namespaces);

      foreach ($objects as $object) { // FIXME:

        if (!is_object($object)) { // plain literal
          $xml->writeElement($qname, (string)$object);
        }
        else if ($object instanceof RDF_Literal) { // typed literal
          $xml->startElement($qname);
          if ($object->language)
            $xml->writeAttribute('xml:lang', $object->language);
          if ($object->datatype)
            $xml->writeAttribute('rdf:datatype', $object->datatype);
          $xml->text((string)$object->value);
          $xml->endElement();
        }
        else { // RDF_URIRef, or the like
          $xml->startElement($qname);
          $xml->writeAttribute('rdf:resource', (string)$object);
          $xml->endElement();
        }

      }
    }

    $xml->endElement(); // rdf:Description
  }

  $xml->endElement(); // rdf:RDF
  $xml->endDocument();
  print $xml->outputMemory(TRUE);
}

//////////////////////////////////////////////////////////////////////////////
// TriX output

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

//////////////////////////////////////////////////////////////////////////////
// Turtle output

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
      $qname = rdf_uri_to_qname($predicate, $namespaces);

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

//////////////////////////////////////////////////////////////////////////////
// N-Triples output

function rdf_serialize_ntriples($data, array $options = array()) {
  $namespaces = @$options['namespaces'];

  foreach ($data as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      foreach ($objects as $object) {

        printf('<%s> <%s> ', (string)$subject, (string)$predicate);

        if (!is_object($object)) { // plain literal
          printf('"%s"', (string)$object); // FIXME: string escaping
        }
        else if ($object instanceof RDF_Literal) { // typed literal
          printf('"%s"', (string)$object->value); // FIXME: string escaping
          if ($object->language)
            printf('@%s', $object->language);
          if ($object->datatype)
            printf('^^%s', $object->datatype);
        }
        else { // RDF_URIRef, or the like
          printf('<%s>', (string)$object);
        }

        printf(" .\n");
      }
    }
  }
}
