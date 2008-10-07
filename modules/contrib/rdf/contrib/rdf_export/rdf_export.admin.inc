<?php
// $Id: rdf_export.admin.inc,v 1.1 2008/02/11 21:54:18 arto Exp $
/**
 * rdf_export.admin.inc - RDF Export admin page callbacks.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 * @subpackage rdf_export.module
 */

//////////////////////////////////////////////////////////////////////////////
// RDF data export

function rdf_export_admin_screen(&$form_state) {
  $form = array();
  $form['export'] = array('#type' => 'fieldset', '#title' => t('Export RDF data'));

  $repos = rdf_get_repositories('names');
  $form['export']['repos'] = array('#type' => 'checkboxes', '#title' => t('Repositories'), '#default_value' => array_keys($repos), '#options' => $repos);

  /*$namespaces = array_keys(rdf_get_namespaces('names'));
  $namespaces = array_combine($namespaces, $namespaces);
  unset($namespaces['_']); // always export bnodes
  $form['export']['vocabs'] = array('#type' => 'checkboxes', '#title' => t('Vocabularies'), '#default_value' => array_keys($namespaces), '#options' => $namespaces);*/

  $formats = rdf_get_formats('names', 'w');
  $form['export']['format'] = array('#type' => 'select', '#title' => t('RDF format'), '#options' => $formats, '#default_value' => RDF_FORMAT, '#description' => t('Select the output format.'));

  $form['export']['sort'] = array('#type' => 'checkbox', '#title' => t('Sort by subject'), '#default_value' => '1', );

  $form['export']['submit'] = array('#type' => 'submit', '#value' => t('Export'));
  return $form;
}

function rdf_export_admin_screen_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $repos = array_keys(array_filter($repos, 'is_string'));
  //$vocabs = array_keys(array_filter($vocabs, 'is_string'));

  if (empty($repos)) {
    form_set_error('repos', t('No repositories selected.'));
  }

  /*if (empty($vocabs)) {
    form_set_error('vocabs', t('No vocabularies selected.'));
  }*/
}

function rdf_export_admin_screen_submit($form, &$form_state) {
  require_once drupal_get_path('module', 'rdf_export') . '/rdf_export.pages.inc';

  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $repos = array_values(array_filter($repos, 'is_string'));
  //$vocabs = array_values(array_filter($vocabs, 'is_string'));

  // TODO: filter by vocabulary selections
  $data = rdf_query(NULL, NULL, NULL, array('repository' => $repos));
  rdf_export($data, 'export', $format, array('log' => TRUE));
}
