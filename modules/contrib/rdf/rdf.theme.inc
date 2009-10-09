<?php
// $Id: rdf.theme.inc,v 1.9 2009/02/16 13:27:09 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks: Property-value tables

function theme_rdf_property_table($data) {
  $rows = array();

  if (!empty($data)) {
    $data = is_object($data) ? rdf_normalize($data) : $data;

    foreach ($data as $subject => $predicates) {
      foreach ($predicates as $predicate => $objects) {
        foreach ($objects as $object) {
          $rows[] = array(
            array('data' => theme('rdf_triple_cell', $predicate), 'header' => TRUE),
            theme('rdf_triple_cell', $object),
          );
        }
      }
    }
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No data.'), 'colspan' => 2));
  }

  return theme('table', array(), $rows, array('class' => 'rdf-properties'));
}

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks: Triple tables

function theme_rdf_triple_table($data, array $options = array()) {
  $head = array(t('Subject'), t('Predicate'), t('Object'));
  $rows = array();

  if (!empty($data)) {
    $data = is_object($data) ? rdf_normalize($data) : $data;

    foreach ($data as $subject => $predicates) {
      foreach ($predicates as $predicate => $objects) {
        foreach ($objects as $object) {
          $rows[] = theme('rdf_triple_row', $subject, $predicate, $object, $options);
        }
      }
    }
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No data.'), 'colspan' => 3));
  }

  return theme('table', $head, $rows, array('class' => 'rdf-triples'));
}

function theme_rdf_triple_row($subject, $predicate, $object, array $options = array()) {
  return array(
    theme('rdf_triple_cell', $subject, $options),
    theme('rdf_triple_cell', $predicate, $options),
    theme('rdf_triple_cell', $object, $options),
  );
}

function theme_rdf_triple_cell($value, array $options = array()) {
  $value = (string)$value; // FIXME

  if (rdf_is_valid_uri($value)) { // FIXME
    $uri = $value;
    $qname = @rdf_uri_to_qname($uri, FALSE);
    $title = !empty($options['brackets']) ? ($qname ? "[$qname]" : "<$uri>") : ($qname ? $qname : $uri);
    return _rdf_truncate_uri($uri, $title, $options);
  }
  else {
    return check_plain(truncate_utf8($value, 56, TRUE, TRUE));
  }
}

function theme_rdf_value($value) {
  switch (rdf_get_type($value)) {
    case 'bnode':
    case 'uri':
      return _rdf_truncate_uri((string)$value, rdf_uri_to_qname((string)$value, FALSE));
    case 'string':
      return check_plain(truncate_utf8($value, 56, TRUE, TRUE));
    case 'literal':
      $tooltip = !empty($value->language) ? '@' . $value->language : $value->qname();
      return '<span' . drupal_attributes(array('title' => $tooltip)) . '>' . check_plain($value->value) . '</span>';
  }
}

function _rdf_truncate_uri($uri, $title = NULL, $options = array()) {
  $title = $title ? $title : $uri;
  $function = isset($options['link']) ? $options['link'] : 'l';
  return $function(
    preg_replace('/ \.\.\.$/', '...', truncate_utf8($title, 32, TRUE, TRUE)),
    $uri, array('attributes' => array('title' => $uri)));
}
