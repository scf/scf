<?php
// $Id: rdf_export.pages.inc,v 1.1 2008/02/11 21:54:18 arto Exp $
/**
 * rdf_export.pages.inc - RDF Export page callbacks.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 * @subpackage rdf_export.module
 */

//////////////////////////////////////////////////////////////////////////////
// Menu callbacks

function rdf_export_site() {
  $data = rdf_query(url(NULL, array('absolute' => TRUE)));
  rdf_export($data, 'site');
}

function rdf_export_entity($type, $id) {
  $data = rdf_query(url($type . '/' . $id, array('absolute' => TRUE)));
  rdf_export($data, implode('-', array($type, $id)));
}

function rdf_export($data, $filename = 'export', $format = RDF_FORMAT, $options = array()) {
  require_once drupal_get_path('module', 'rdf_export') . '/rdf_export.inc';

  $formats = rdf_get_formats();
  $format = isset($formats[@$_GET['format']]) ? $_GET['format'] : $format;
  $format = $formats[$format];
  $filename = implode('.', array($filename, $format->file_ext));

  if (!empty($options['log'])) {
    watchdog('rdf', 'Exported RDF data file: %filename.', array('%filename' => $filename));
  }

  $output = rdf_serialize(is_string($data) ? $data() : $data, array('format' => $format->name));

  //$format->mime_type = 'text/plain'; // FIXME
  header('Content-Disposition: inline; filename=' . $filename);
  header('Content-Type: ' . $format->mime_type . '; charset=' . $format->encoding);
  header('Content-Length: ' . strlen($output));
  die($output);
}
