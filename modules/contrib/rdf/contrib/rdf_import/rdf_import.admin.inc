<?php
// $Id: rdf_import.admin.inc,v 1.1 2008/02/11 21:54:40 arto Exp $
/**
 * rdf_import.admin.inc - RDF Import admin page callbacks.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 * @subpackage rdf_import.module
 */

//////////////////////////////////////////////////////////////////////////////
// RDF data import

function rdf_import_admin_screen(&$form_state, $edit = array('repository' => 'local', 'url' => 'http://', 'format' => RDF_FORMAT)) {
  $edit = (object)$edit;
  $form = array();

  foreach (array('repository', 'url', 'format') as $key) {
    if (isset($_GET[$key])) {
      // This is idiotic, but due to drupal_urlencode() the URL has a
      // double-escaped slash character that we need to get rid of...
      // @see http://api.drupal.org/api/function/drupal_urlencode/6
      $edit->$key = ($key == 'url') ? str_replace('/%2F', '//', $_GET[$key]) : $_GET[$key];
    }
  }

  $form['import'] = array('#type' => 'fieldset', '#title' => t('Import RDF data from a URL'));
  $form['import']['repository'] = array('#type' => 'select', '#title' => t('Repository'), '#default_value' => $edit->repository, '#options' => rdf_get_repositories('names', array('persistent' => TRUE, 'mutable' => TRUE)), '#description' => t(''), '#disabled' => TRUE);
  $form['import']['url'] = array('#type' => 'textfield', '#title' => 'URL', '#default_value' => $edit->url, '#maxlength' => 255, '#required' => TRUE, '#description' => t(''));
  $form['import']['format'] = array('#type' => 'select', '#title' => t('Format'), '#default_value' => $edit->format, '#options' => rdf_get_formats('names') /*FIXME*/, '#description' => t(''));

  $form['import']['submit'] = array('#type' => 'submit', '#value' => t('Import'));
  return $form;
}

function rdf_import_admin_screen_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!rdf_is_valid_url($url)) {
    form_set_error('url', t('URL is not valid.'));
  }
}

function rdf_import_admin_screen_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  // Fetch the serialized representation from the given URL:
  if (!($text = file_get_contents($url))) {
    drupal_set_message(t('Error while fetching RDF data from %url.', array('%url' => $url)), 'error');
    $form_state['redirect'] = array('admin/content/rdf/import', drupal_query_string_encode(array('url' => $url)));
    return;
  }

  // Parse into triples using the specified RDF parser:
  if (($data = rdf_unserialize($text, array('format' => $format, 'uri' => $url))) === FALSE) {
    drupal_set_message(t('Error while parsing RDF data from %url.', array('%url' => $url)), 'error');
    $form_state['redirect'] = array('admin/content/rdf/import', drupal_query_string_encode(array('url' => $url)));
    return;
  }

  // Assert each triple in the specified repository, while also preserving
  // provenance. We keep track of any failures during assertions.
  $counter = 0;
  foreach ($data as $stmt) {
    if (call_user_func_array('rdf_insert', array_merge($stmt, array(array('graph' => $url, 'repository' => $repository))))) {
      $counter++;
    }
  }

  drupal_set_message(t('!count statements imported from %url.', array('!count' => $counter, '%url' => $url)));

  if (($errors = (count($data) - $counter))) {
    drupal_set_message(t('!count statements were not imported due to an error when asserting them.', array('!count' => $errors)), 'error');
    $form_state['redirect'] = array('admin/content/rdf/import', drupal_query_string_encode(compact('repository', 'url', 'format')));
  }
  else {
    $form_state['redirect'] = array('admin/content/rdf', drupal_query_string_encode(array('g' => $url)));
  }
}
