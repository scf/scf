<?php
// $Id: rdf.services.inc,v 1.5 2009/02/16 13:27:09 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Services API hooks

/**
 * Implementation of hook_service().
 */
function rdf_service() {
  return array(
    array(
      '#method'   => 'rdf.insert',
      '#callback' => 'rdf_service_insert',
      '#return'   => 'boolean',
      '#args'     => array(
        array('#name' => 'statement', '#type' => 'array', '#optional' => FALSE, '#description' => t('Subject, predicate, and object.')),
      ),
      '#help'     => t('Inserts a new statement into the database.'),
    ),
    array(
      '#method'   => 'rdf.delete',
      '#callback' => 'rdf_service_delete',
      '#return'   => 'boolean',
      '#args'     => array(
        array('#name' => 'statement', '#type' => 'array', '#optional' => FALSE, '#description' => t('Subject, predicate, and object.')),
      ),
      '#help'     => t('Deletes an existing statement from the database.'),
    ),
    array(
      '#method'   => 'rdf.query',
      '#callback' => 'rdf_service_query',
      '#return'   => 'struct',
      '#args'     => array(
        array('#name' => 'pattern', '#type' => 'array', '#optional' => TRUE, '#description' => t('Subject, predicate, and object.')),
      ),
      '#help'     => t('Finds all statements in the database matching a given triple pattern.'),
    ),
  );
}

function rdf_service_insert(array $statement) {
  return !!call_user_func_array('rdf_insert', $statement);
}

function rdf_service_delete(array $statement) {
  return !!call_user_func_array('rdf_delete', $statement);
}

function rdf_service_query(array $pattern = array()) {
  return rdf_deobjectify(rdf_normalize(call_user_func_array('rdf_query', $pattern)));
}
