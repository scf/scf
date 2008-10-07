<?php
// $Id: sparql.client.inc,v 1.1 2008/04/28 13:59:49 arto Exp $
/**
 * sparql.client.inc - SPARQL HTTP client implementation.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package sparql.module
 */

//////////////////////////////////////////////////////////////////////////////
// SPARQL HTTP client

function sparql_request($endpoint, $query, $options = array(), &$errors = NULL) {
  $options = array_merge(array('query' => $query, 'format' => 'application/sparql-results+json', 'output' => 'json'), $options);

  // HACK
  if (!isset($options['default-graph-uri']) && preg_match('!dbpedia!', $endpoint)) {
    $options['default-graph-uri'] = 'http://dbpedia.org';
  }

  $request = $endpoint . '?' . http_build_query($options, NULL, '&');
  $headers = array('Accept' => $options['format'] . ', */*');
  if ($response = drupal_http_request($request, $headers, 'GET')) {

    if (empty($response->code) && empty($response->data)) {
      $errors[] = t('Remote SPARQL query timed out; please try again.');
    }
    else if (!empty($response->error)) {
      $error = str_replace(SPARQL_Query::prefixes() . "\n", '', $response->data);
      $errors[] = t('Remote SPARQL endpoint returned error: %error', array('%error' => $error));
    }
    else if (preg_match('!^([\w]+/[^;]+)!', $response->headers['Content-Type'], $matches)) {

      switch ($format = $matches[1]) {
        case 'application/sparql-results+json':
          return sparql_parse_json_results($response->data);

        case 'application/sparql-results+xml':
          return sparql_parse_xml_results($response->data);

        default:
          foreach (rdf_get_formats('info', 'r') as $reader) {
            if ($reader->mime_type == $format) {
              if ($result = rdf_unserialize($response->data, array('format' => $reader->name, 'uri' => $endpoint)))
                return new ArrayIterator($result);
              break;
            }
          }
          $errors[] = t('Unknown SPARQL results format: %type.', array('%type' => $format));
          break;
      }
    }
  }

  return FALSE;
}

function sparql_parse_json_results($input) {
  $input  = json_decode($input);

  if (isset($input->boolean)) {
    return (bool)$input->boolean;
  }

  $output = array();
  foreach ($input->results->bindings as $row) {
    $item = array();
    foreach (get_object_vars($row) as $field => $literal) {
      $item[$field] = $literal->value;
    }
    $output[] = $item;
  }
  return $output;
}

function sparql_parse_xml_results($input) {
  // TODO
  drupal_set_message(t('Support for application/sparql-results+xml not implemented yet.'), 'error');
}
