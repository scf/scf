<?php
// $Id: rdf_db.admin.inc,v 1.3 2008/02/10 23:01:15 arto Exp $
/**
 * rdf_db.admin.inc - RDF DB admin page callbacks.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package rdf.module
 * @subpackage rdf_db.module
 */

//////////////////////////////////////////////////////////////////////////////
// RDF DB data management

function rdf_db_admin_data_form(&$form_state, $edit = array('subject' => '', 'predicate' => '', 'object' => '', 'repository' => 'local')) {
  $edit = (object)$edit;
  $form = array();

  $form['subject'] = array('#type' => 'textfield', '#title' => t('Subject'), '#default_value' => $edit->subject, '#maxlength' => 255, '#required' => TRUE, '#description' => t('The subject of the statement: the entity or resource that the statement is about, i.e. the who or what. This should be given as either a <a href="http://en.wikipedia.org/wiki/URI" title="Uniform Resource Identifier">URI</a> or a <a href="http://en.wikipedia.org/wiki/CURIE" title="Compact URI">CURIE</a>.'));
  $form['predicate'] = array('#type' => 'textfield', '#title' => t('Predicate'), '#default_value' => $edit->predicate, '#maxlength' => 255, '#required' => TRUE, '#description' => t('The predicate of the statement: the relation between subject and object, i.e. the property name or verb. This should be given as either a <a href="http://en.wikipedia.org/wiki/URI" title="Uniform Resource Identifier">URI</a> or a <a href="http://en.wikipedia.org/wiki/CURIE" title="Compact URI">CURIE</a>.'));
  $form['object'] = array('#type' => 'textfield', '#title' => t('Object'), '#default_value' => $edit->object, '#maxlength' => 255, '#required' => TRUE, '#description' => t('The object of the statement: the entity or resource that relates to the subject as described by the predicate; i.e. the property value. This should be given as either a <a href="http://en.wikipedia.org/wiki/URI" title="Uniform Resource Identifier">URI</a>, a <a href="http://en.wikipedia.org/wiki/CURIE" title="Compact URI">CURIE</a>, or a literal.'));

  //$form['repository'] = array('#type' => 'select', '#title' => t('Repository'), '#default_value' => $edit->repository, '#options' => array(), '#description' => t('')); // TODO

  $form['submit'] = array('#type' => 'submit', '#value' => t('Add statement'));
  return $form;
}

function rdf_db_admin_data_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!rdf_is_valid_uri($subject) && !rdf_is_valid_curie($subject)) {
    form_set_error('subject', t('Subject value is not a valid URI or CURIE.'));
  }

  if (!rdf_is_valid_uri($predicate) && !rdf_is_valid_curie($predicate)) {
    form_set_error('predicate', t('Predicate value is not a valid URI or CURIE.'));
  }

  // TODO: relax this once we have a specific input box for literals.
  if (!rdf_is_valid_uri($object) && !rdf_is_valid_curie($object)) {
    //form_set_error('object', t('Object value is not a valid URI or CURIE.'));
  }
}

function rdf_db_admin_data_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  // TODO: properly sanitize/expand/convert subject, predicate and object.
  $subject   = rdf_uriref($subject);
  $predicate = rdf_uriref($predicate);
  $object    = rdf_is_valid_uri($object) ? rdf_uriref($object) : $object;

  if (rdf_insert($subject, $predicate, $object)) {
    drupal_set_message(t('The statement has been added.'));
  }
  else {
    drupal_set_message(t('The statement was not added due to an error reported by the repository.'), 'error');
  }

  $form_state['redirect'] = array('admin/content/rdf', drupal_query_string_encode(array('s' => (string)$subject)));
}

//////////////////////////////////////////////////////////////////////////////
// RDF DB namespace management

function rdf_db_admin_ns_edit($prefix = '') {
  if (empty($prefix)) {
    return drupal_get_form('rdf_db_admin_ns_form');
  }
  else {
    return drupal_get_form('rdf_db_admin_ns_form', rdf_db_load_namespace($prefix));
  }
}

function rdf_db_admin_ns_form(&$form_state, $edit = array('prefix' => '', 'uri' => '')) {
  $edit = (object)$edit;
  $form = array();

  $form['prefix'] = array('#type' => 'textfield', '#title' => t('Prefix'), '#default_value' => $edit->prefix, '#maxlength' => 64, '#required' => TRUE, '#description' => t('The short abbreviation to use in the place of the base URI. Keep it short, and use only lowercase, alphanumeric letters.'));
  $form['uri'] = array('#type' => 'textfield', '#title' => t('Base URI'), '#default_value' => $edit->uri, '#maxlength' => 255, '#required' => TRUE, '#description' => t('The absolute base URI of the RDF vocabulary. Make sure that the URI terminates with a hash or slash character.'));

  $form['key'] = array('#type' => 'hidden', '#value' => $edit->prefix);
  $form['submit'] = array('#type' => 'submit', '#value' => empty($edit->prefix) ? t('Create new namespace') : t('Update namespace'));

  return $form;
}

function rdf_db_admin_ns_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!preg_match('/^[a-z_]+[a-z\d\._]*$/', $prefix)) {
    form_set_error('prefix', t('Prefix %prefix contains illegal characters.', array('%prefix' => $prefix)));
  }

  if (!rdf_is_valid_uri($uri)) {
    form_set_error('uri', t('Base URI %uri contains illegal characters.', array('%uri' => $uri)));
  }

  if (!preg_match('@[#/]$@', $uri)) {
    form_set_error('uri', t('Base URI %uri must terminate with "#" or "/".', array('%uri' => $uri)));
  }
}

function rdf_db_admin_ns_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (empty($key)) {
    db_query("INSERT INTO {rdf_namespaces} (prefix, uri) VALUES ('%s', '%s')", $prefix, $uri);
    drupal_set_message(t('The namespace has been created.'));
  }
  else {
    db_query("UPDATE {rdf_namespaces} SET prefix = '%s', uri = '%s' WHERE prefix = '%s'", $prefix, $uri, $key);
    drupal_set_message(t('The namespace has been updated.'));
  }

  $form_state['redirect'] = 'admin/settings/rdf/namespaces';
}

function rdf_db_admin_ns_delete($form_state, $prefix) {
  if (user_access('administer RDF namespaces') && rdf_db_load_namespace($prefix)) {
    $form['prefix'] = array('#type' => 'value', '#value' => $prefix);
    $output = confirm_form($form,
      t('Are you sure you want to delete the RDF namespace %title?', array('%title' => $prefix)),
      isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/rdf/namespaces');
  }
  return $output;
}

function rdf_db_admin_ns_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    db_query("DELETE FROM {rdf_namespaces} WHERE prefix = '%s'", $form_state['values']['prefix']);
    drupal_set_message(t('The namespace has been deleted.'));

    $form_state['redirect'] = 'admin/settings/rdf/namespaces';
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF DB repository management

function rdf_db_admin_db_edit($name = '') {
  if (empty($name)) {
    return drupal_get_form('rdf_db_admin_db_form');
  }
  else {
    return drupal_get_form('rdf_db_admin_db_form', rdf_db_load_repository($name));
  }
}

function rdf_db_admin_db_form(&$form_state, $edit = array('name' => '', 'title' => '', 'description' => '')) {
  $edit = (object)$edit;
  $form = array();

  $form['identity'] = array('#type' => 'fieldset', '#title' => t('Identification'));

  $form['identity']['title'] = array('#type' => 'textfield', '#title' => t('Name'), '#default_value' => $edit->title, '#maxlength' => 64, '#required' => TRUE, '#description' => t('The human-readable name of this repository. It is recommended that this name begins with a capital letter and consists only of letters, numbers, and spaces.'));
  $form['identity']['name'] = array('#type' => 'textfield', '#title' => t('ID'), '#default_value' => $edit->name, '#maxlength' => 32, '#required' => TRUE, '#description' => t('The machine-readable name of this repository. This text will be used for constructing the unique URI identifying this repository. This name may consist of only of lowercase letters, numbers, and underscores. Hyphens are not allowed. Underscores will be converted into hyphens when constructing the URI for the repository. This name must be unique to this repository.'));
  $form['identity']['description'] = array('#title' => t('Description'), '#type' => 'textarea', '#default_value' => $edit->description, '#rows' => 2, '#description' => t('A brief description of this repository.'));

  $form['key'] = array('#type' => 'hidden', '#value' => $edit->name);
  $form['submit'] = array('#type' => 'submit', '#value' => empty($edit->name) ? t('Create new repository') : t('Update repository'));

  return $form;
}

function rdf_db_admin_db_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!preg_match('/^[a-z]+[a-z\d_]*$/', $name)) {
    form_set_error('name', t('The machine-readable name can only consist of lowercase letters, underscores, and numbers.', array('%name' => $name)));
  }

  if (array_search($name, rdf_db_get_repository_names()) !== FALSE) { // FIXME
    form_set_error('name', t('The machine-readable name %name is already used by another repository.', array('%name' => $name)));
  }
}

function rdf_db_admin_db_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (empty($key)) {
    rdf_db_create_repository($name, array('dc:title' => $title, 'dc:description' => $description));
    drupal_set_message(t('The repository has been created.'));
  }
  else {
    if ($key != $name) {
      rdf_db_rename_repository($key, $name);
      rdf_db_delete_repository_metadata($key);
    }
    rdf_db_update_repository_metadata($name, array('dc:title' => $title, 'dc:description' => $description));
    drupal_set_message(t('The repository has been updated.'));
  }

  $form_state['redirect'] = 'admin/settings/rdf/repositories';
}

function rdf_db_admin_db_delete($form_state, $name) {
  if (user_access('administer RDF repositories')) {
    $form['name'] = array('#type' => 'value', '#value' => $name);
    $output = confirm_form($form,
      t('Are you sure you want to delete the RDF repository %title?', array('%title' => $name)),
      isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/rdf/repositories',
      t('This action will destroy all data contained in the repository and cannot be undone.'));
  }
  return $output;
}

function rdf_db_admin_db_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    rdf_db_delete_repository($form_state['values']['name']);
    drupal_set_message(t('The repository has been deleted.'));

    $form_state['redirect'] = 'admin/settings/rdf/repositories';
  }
}
