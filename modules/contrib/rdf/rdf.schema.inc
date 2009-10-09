<?php
// $Id: rdf.schema.inc,v 1.2 2009/02/16 13:27:09 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// RDF Schema API

function rdf_schema_data() {
  return rdf_query(NULL, NULL, NULL, array('context' => RDF_SCHEMA_URI));
}

function rdf_schema_get_class_uri($class) {
  return RDF_SCHEMA_URI . $class;
}

function rdf_schema_get_property_uri($class, $property) {
  return RDF_SCHEMA_URI . $class . '#' . $property;
}

function rdf_schema_get_entities() {
  return array_keys(module_invoke_all('rdf_classes'));
}

function rdf_schema_get_classes() {
  $resources = array(RDF_SCHEMA_URI => new RDF_QueryCallback('rdf_schema_load_schema'));

  $url_options = array('absolute' => TRUE, 'alias' => TRUE);
  foreach (module_invoke_all('rdf_classes') as $class => $class_info) {
    if (empty($class_info['enabled']) || !module_exists($class_info['module']))
      continue; // skip disabled modules

    // Class URI
    $uri = rdf_schema_get_class_uri($class);
    $resources[$uri] = new RDF_QueryCallback('rdf_schema_load_schema_class', array($class, $class_info));

    // Property URIs
    if (!empty($class_info['table'])) {
      $sql_schema = drupal_get_schema($class_info['table']);
      $form_info = _rdf_schema_get_form($class);
      foreach ($sql_schema['fields'] as $field_name => $field_info) {

        $uri = rdf_schema_get_property_uri($class, $field_name);
        $resources[$uri] = new RDF_QueryCallback('rdf_schema_load_schema_property', array($class, $field_name, $field_info, @$form_info[$field_name]));
      }
    }
  }

  return $resources;
}

function rdf_schema_get_resources() {
  $resources = array(RDF_SITE_URI => new RDF_QueryCallback('rdf_schema_load_site'));

  $url_options = array('absolute' => TRUE, 'alias' => TRUE);
  foreach (module_invoke_all('rdf_classes') as $class => $class_info) {
    if (empty($class_info['enabled']) || !module_exists($class_info['module']))
      continue; // skip disabled modules

    $result = db_query($class_info['query']);
    while ($row = db_fetch_array($result)) {
      $uri = $class_info['uri'];
      foreach ($row as $field => $value) {
        $uri = str_replace('%' . $field, $value, $uri);
      }

      $uri_options = array();

      // URI fragments must be specially handled in order to use them with url()
      if (strpos($uri, '#') !== FALSE) {
        list($uri, $uri_options['fragment']) = explode('#', $uri, 2);
      }

      $menu_item = menu_get_item($uri);
      if (!empty($menu_item['access'])) {
        if (is_object($menu_item['page_arguments'][0])) {
          $item = $menu_item['page_arguments'][0];

          $uri = url($uri, array_merge($url_options, $uri_options));
          $resources[$uri] = new RDF_QueryCallback('rdf_schema_load_' . $class, array($item));
        }
      }
    }
  }

  return $resources;
}

//////////////////////////////////////////////////////////////////////////////
// RDF Schema meta-model

function rdf_schema_load_schema() {
  return array(
    rdf::type         => rdf_qname_to_uriref('owl:Ontology'),
    rdfs::seeAlso     => rdf_uri(url('rdf', array('absolute' => TRUE))),
    rdfs::label       => t('RDF schema for @site_name', array('@site_name' => variable_get('site_name', 'Drupal'))),
    dc::type          => rdf_qname_to_uriref('dcmitype:Dataset'),
    dc::format        => RDF_FORMAT,
    dc::publisher     => rdf_uri(RDF_SITE_URI),
    dc::creator       => rdf_uri(RDF_SCHEMA_MODULE_URI),
  );
}

function rdf_schema_load_schema_class($class, $class_info, $comment = NULL) {
  $comment = !empty($comment) ? $comment : module_invoke($class_info['module'], 'help', 'node/add/#' . $class, NULL);
  $comment = !empty($comment) ? $comment : module_invoke($class_info['module'], 'help', 'admin/help#' . $class_info['module'], NULL);
  return array(
    rdf::type         => rdf_qname_to_uriref('rdfs:Class'),
    rdfs::isDefinedBy => rdf_uri(RDF_SCHEMA_URI),
    rdfs::subClassOf  => rdf_qname_to_uriref('rdfs:Resource'),
    rdfs::label       => rdf_literal($class_info['title'], 'en'),
    rdfs::comment     => !empty($comment) ? rdf_literal(strip_tags($comment), 'en') : NULL,
  );
}

function rdf_schema_load_schema_property($class, $field_name, $field_info, $form_info) {
  $title = isset($form_info['#title']) ? $form_info['#title'] : @$field_info['description'];
  return array(
    rdf::type         => rdf_qname_to_uriref('rdf:Property'),
    rdfs::isDefinedBy => rdf_uri(RDF_SCHEMA_URI),
    rdfs::domain      => rdf_uri(rdf_schema_get_class_uri($class)),
    rdfs::range       => rdf_qname_to_uriref('rdfs:Literal'), // TODO: should we map these to xsd:* types?
    rdfs::label       => !empty($title) ? rdf_literal($title, 'en') : NULL,
    rdfs::comment     => !empty($form_info['#description']) ? rdf_literal($form_info['#description'], 'en') : NULL,
  );
}

//////////////////////////////////////////////////////////////////////////////
// RDF Schema mappings

function rdf_schema_load_site() {
  return array(
    rdf::type         => rdf_uri(rdf_qname_to_uri('drupal:site')),
  );
}

function rdf_schema_load_user($user) {
  $user = is_object($user) ? $user : user_load((int)$user);
  return array(
    rdf::type         => rdf_uri(rdf_qname_to_uri('drupal:user')),
    'user:uid'        => (int)$user->uid,
    'user:name'       => $user->name,
    'user:mail'       => rdf_mailto($user->mail),
    'user:created'    => rdf_datetime($user->created),
    'user:access'     => rdf_datetime($user->access),
    'user:login'      => rdf_datetime($user->login),
    'user:status'     => (bool)$user->status,
  );
}

function rdf_schema_load_node($node) {
  $node = is_object($node) ? $node : node_load((int)$node);
  return array(
    rdf::type         => rdf_uri(rdf_qname_to_uri(variable_get('rdf_schema_class_' . $node->type, 'drupal:node'))),
    'node:nid'        => (int)$node->nid,
    'node:vid'        => (int)$node->vid,
    'node:uid'        => (int)$node->uid,
    'node:type'       => (int)$node->type,
    'node:title'      => $node->title,
    'node:body'       => $node->body,
    'node:created'    => rdf_datetime($node->created),
    'node:changed'    => rdf_datetime($node->changed),
    'node:status'     => (bool)$node->status,
    'node:comment'    => (bool)$node->comment,
    'node:promote'    => (bool)$node->promote,
    'node:moderate'   => (bool)$node->moderate,
    'node:sticky'     => (bool)$node->sticky,
    'node:translate'  => (bool)$node->translate,
  );
}

//////////////////////////////////////////////////////////////////////////////
// Forms API helper functions

function _rdf_schema_get_form($entity) {
  $form_builder = '_rdf_schema_get_' . $entity . '_form';
  $form = function_exists($form_builder) ? $form_builder() : array();
  return _rdf_schema_cleanup_form($form);
}

function _rdf_schema_get_user_form() {
  $form = @user_edit_form($form_state, NULL, NULL, TRUE);
  return $form/*['account']*/;
}

function _rdf_schema_get_vocabulary_form() {
  module_load_include('inc', 'taxonomy', 'taxonomy.admin');

  $form = @taxonomy_form_vocabulary($form_state);
  $form['vid']['#title'] = t('ID');
  return $form;
}

function _rdf_schema_get_term_form() {
  module_load_include('inc', 'taxonomy', 'taxonomy.admin');

  // TODO: pass a fake $vocabulary object to get all fields?
  $form = @taxonomy_form_term($form_state);
  $form['tid']['#title'] = t('ID');
  return $form;
}

function _rdf_schema_get_node_form($type = 'page') {
  module_load_include('inc', 'node', 'node.pages');

  $user = user_load(1);
  $node = array('uid' => $user->uid, 'name' => $user->name, 'type' => $type, 'language' => '');

  // TODO: created, type, language, changed
  $form = @node_form($form_state, $node);
  $form['nid']['#title'] = t('ID');
  $form['vid']['#title'] = t('Version ID');
  $form['uid']['#title'] = t('User ID');
  $form['tnid']['#title'] = t('Translation ID');
  return $form;
}

function _rdf_schema_get_comment_form() { // FIXME
  $form = @comment_form($form_state, array());
  $form['cid']['#title'] = t('ID');
  $form['pid']['#title'] = t('Parent ID');
  $form['nid']['#title'] = t('Node ID');
  $form['nid']['#title'] = t('User ID');
  return $form;
}

function _rdf_schema_cleanup_form(&$form) {
  unset($form['buttons']);
  foreach ($form as $key => &$element) {
    if (strpos($key, '#') === 0 || !is_array($element)) {
      unset($form[$key]);
      continue;
    }

    if ((isset($element['#type']) && $element['#type'] == 'fieldset') ||
        isset($element['#after_build'])) {
      foreach ($element as $field => $field_info) {
        if (is_string($field) && strpos($field, '#') !== 0) {
          $form[$field] = $field_info;
        }
      }
      unset($form[$key]);
    }
  }
  return $form;
}
