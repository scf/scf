<?php
// $Id: rdf.feedapi.inc,v 1.3 2009/02/22 20:44:14 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Feed Element Mapper API hooks

/**
 * Implementation of hook_feedapi_mapper().
 */
function rdf_feedapi_mapper($op, $node, $field_name, $feed_element = array(), $sub_field = '') {
  if ($field_name == 'rdf') { // defined in rdf_nodeapi('prepare')
    switch ($op) {
      case 'describe':
        return t('Maps feed item properties into RDF repositories and into CCK fields as defined by RDF schema settings');

      case 'list':
        // The sub-fields define the actual mapping targets available in the
        // Feed Element Mapper drop-down boxes for each feed item property.
        // In our case, we expose all mutable RDF repositories as targets,
        // and additionally include a 'System' target that will perform a
        // reverse mapping through the RDF Schema API:
        $sub_fields = array('system' => t('System'));
        foreach (rdf_get_repositories() as $repo_name => $repo_info) {
          if (!empty($repo_info['enabled']) && !empty($repo_info['mutable'])) {
            $sub_fields[$repo_name] = $repo_info['title'];
          }
        }
        return $sub_fields;

      case 'map':
        // NOTE: $node->nid is unset at this point if the feed item node is being created.

        // We're unable to map individual feed item properties, as the
        // hook_feedapi_mapper() API misplaces the information on what the
        // feed item property's name is (i.e. it only passes in the value,
        // in $feed_element, not the name), without which we are powerless
        // to do any RDF-related mapping. However, we do support collections
        // of feed item properties, such as the 'rdf' collection exported by
        // the patched version (see drupal.org issue #XXXXXX) of the Common
        // Syndication Parser for RSS 1.0, as collections of properties are
        // indexed by the property name and hence allow us to obtain the
        // predicate in question. This is still a bit iffy, however, in the
        // sense that the property collections are actually indexed by
        // namespaced QNames/CURIEs, so namespace prefixes become quite (and
        // unduly) significant as we don't have any other means of
        // determining the full predicate URI. Provided that well-known
        // "standard" prefixes are used in the feed and on the site running
        // this code, all will be well.
        if (is_array($feed_element)) { // is a collection of properties
          $namespaces = rdf_get_namespaces();
          $data = array();

          // Add a statement indicating where the feed item node originates from:
          if (isset($feed_element['rdf_about'][0])) {
            $data['dc:origin'][] = rdf_uri($feed_element['rdf_about'][0]);
            unset($feed_element['rdf_about']);
          }

          // Process all RDF statements extracted from the feed item:
          foreach ($feed_element as $predicate => $values) {
            $predicate = implode(':', explode('_', $predicate, 2)); // first '_' => ':'
            $namespace = substr($predicate, 0, strpos($predicate, ':'));

            if (isset($namespaces[$namespace])) { // skip any unknown namespaces
              foreach (is_array($values) ? $values : array($values) as $value) {
                $data[$predicate][] = rdf_str_to_val($value); // attempt to cast untyped values
              }
            }
          }

          if ($sub_field == 'system') {
            // Attempt reverse CCK field mappings using the RDF Schema module:
            _rdf_feedapi_mapper_cck($node, $data);
          }
          else {                        
            // Store the extracted RDF statements in the RDF repository
            // selected as the target for the mapping (the repository name
            // is passed in $sub_field). We have to do this in an indirect
            // manner as (for new feed item nodes) the $node object is still
            // being constructed and therefore doesn't have a $node->nid,
            // and hence of course doesn't have a URI either. See
            // rdf_nodeapi('prepare') and rdf_nodeapi('insert') for details
            // on how the $node->rdf and $node->rdf_options mechanisms work.
            $node->rdf_options = array('repository' => $sub_field);
            $node->rdf = $data;
          }
        }
        return $node;
    }
  }
  return FALSE;
}

/**
 * @see rdf_feedapi_mapper()
 * @private
 */
function _rdf_feedapi_mapper_cck(&$node, $data) {
  // TODO: all this probably should go into the RDF Schema module proper.
  static $var_prefix = 'rdf_schema_property_content_';

  foreach ($data as $predicate => $values) {
    // Ensure the QName/CURIE property names can be mapped into valid
    // predicate URIs, and ruthlessly skip any that can't. See the note in
    // rdf_feedapi_mapper() regarding the undue importance we must place on
    // namespace prefixes for all this to work.
    if (($predicate = rdf_qname_to_uri($predicate)) && rdf_is_valid_uri($predicate)) {

      // Check if any RDF Schema-based CCK field mappings have been defined
      // for this particular predicate URI:
      $result = db_query('SELECT * FROM {variable} WHERE name LIKE "%s%%" AND value = "%s"', $var_prefix, serialize($predicate));
      while ($row = db_fetch_object($result)) {

        // For each defined CCK field mapping, graft the property values
        // onto the $node object in a CCK-compatible manner. The idea is
        // that CCK will take care of actually storing these values once
        // hook_nodeapi('insert') or hook_nodeapi('update') gets called.
        $field_name = substr($row->name, strlen($var_prefix));
        foreach ($values as $value) {
          $field = isset($node->$field_name) ? $node->$field_name : array();
          $field[] = array('value' => rdf_val_to_str($value));
          $node->$field_name = $field;
        }
      }
    }
  }

  return $node;
}
