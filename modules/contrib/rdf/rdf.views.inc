<?php
// $Id: rdf.views.inc,v 1.8 2009/03/25 04:20:41 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Views API hooks

/**
 * Implementation of hook_views_plugins().
 */
function rdf_views_plugins() {
  return array(
    // Style plugins
    'style' => array(
      'rdf' => array(
        'title' => t('RDF Feed'),
        'help' => t('Generates an RSS 1.0-compatible RDF feed from a view.'),
        'handler' => 'views_plugin_style_rdf',
        'path' => drupal_get_path('module', 'rdf') . '/contrib/rdf_views',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'feed',
        'help topic' => 'style-rss',
      ),
    ),

    // Row plugins
    'row' => array(
      'node_rdf' => array(
        'title' => t('Node (RDF)'),
        'help' => t('Outputs the node as an RDF 1.0-compatible RDF resource.'),
        'handler' => 'views_plugin_row_node_rdf',
        'path' => drupal_get_path('module', 'rdf') . '/contrib/rdf_views',
        'base' => array('node'), // only works with 'node' as base.
        'uses options' => TRUE,
        'type' => 'feed',
        'help topic' => 'style-node-rss',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_data()
 */
function rdf_views_data() {
  $data = array(
    'rdf_resources' => array(
      // Table info.
      'table' => array(
        'group' => t('RDF'),
        'base' => array(
          'field' => 'rid',
          'title' => t('RDF'),
          'help' => t('RDF meta-data.'),
          'weight' => 0,
        ),
      ),
      // Fields.
      'uri' => array(
        'title' => t('URI'),
        'help' => t('The URI of the RDF resource'),
        'field' => array('handler' => 'views_handler_field'),
        'filter' => array('handler' => 'views_handler_filter_boolean_operator'), 
      ),
    ),
  );
  
  foreach (rdf_db_get_repository_tables() as $name => $table) {
    $info = rdf_db_load_repository($name);
    $data[$table] = array(
      // Table info.
      'table' => array(
        'group' => t('@table', array('@table' => $info->title)),
        'join' => array(
          'rdf_resources' => array(
            'left_field' => 'rid',
            'field' => 'sid',
          ),
        ),
      ),
      // Fields.
      /* TODO: 'oid', 'tid', 'lang' */
      'sid' => array(
        'title' => t('Subject'),
        'help' => t("Subject of the RDF statement."),
        'field' => array('handler' => 'views_handler_field'),
      ),
      'uid' => array(
        'title' => t('User'),
        'help' => t("User who created statement."),
        'field' => array('handler' => 'views_handler_field'),
        'relationship' => array(
          'handler' => 'views_handler_relationship',
          'base' => 'users',
          'field' => 'uid',
          'label' => t('user'),
        ),
      ),
      'created' => array(
        'title' => t('Created'),
        'help' => t("Timestamp of the statement."),
        'field' => array('handler' => 'views_handler_field_date'),
        'filter' => array('handler' => 'views_handler_filter_date'),
      ),
      'data' => array(
        'title' => t('Object'),
        'help' => t('RDF Object literal'),
        'field' => array('handler' => 'views_handler_field_rdf_literal'),
        'relationship' => array(
          'handler' => 'views_handler_relationship',
          'base' => 'rdf_resources',
          'base field' => 'uri',
          'field' => 'data',
          'label' => t('@table Resources', array('@table' => $info->title)),
        ),
      ),
    );
    // Add predicate and named graph fields differently as we need to refer back
    // to rdf_resources to pull usable values.
    $data[$table .'_predicate']['table'] = array(
      'group' => t('@table', array('@table' => $info->title)),
      'join' => array(
        'rdf_resources' => array(
          'left_table' => $table,
          'left_field' => 'pid',
          'table' => 'rdf_resources',
          'field' => 'rid',
        ),
      ),
    );
     $data[$table .'_predicate']['uri'] = array(
        'title' => t('Predicate'),
        'help' => t("The RDF statement's predicate."),
        'field' => array('handler' => 'views_handler_field'),
        'filter' => array('handler' => 'views_handler_filter_rdf_predicate'),
    );
    $data[$table .'_context']['table'] = array(
      'group' => t('@table', array('@table' => $info->title)),
      'join' => array(
        'rdf_resources' => array(
          'left_table' => $table,
          'left_field' => 'gid',
          'table' => 'rdf_resources',
          'field' => 'rid',
        ),
      ),
    );
     $data[$table .'_context']['uri'] = array(
        'title' => t('Named graph'),
        'help' => t("The RDF statement's named graph."),
        'field' => array('handler' => 'views_handler_field'),
        'filter' => array('handler' => 'views_handler_filter_rdf_context'),
    );
  }
  return $data;
}

function rdf_views_handlers() {
  return array(
    'info' => array('path' => drupal_get_path('module', 'rdf') . '/contrib/rdf_views'),
    'handlers' => array(
      'views_handler_filter_rdf_predicate' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_filter_rdf_context' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_field_rdf_literal' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}

//////////////////////////////////////////////////////////////////////////////

function rdf_views_get_feeds() {
  $result = db_query("SELECT v.*, vd.* FROM {views_display} vd LEFT JOIN {views_view} v ON vd.vid = v.vid WHERE vd.display_plugin = 'feed'");
  $feeds  = array();
  while ($row = db_fetch_object($result)) {
    $row->display_options = unserialize($row->display_options);
    if (isset($row->display_options['style_plugin']) && $row->display_options['style_plugin'] == 'rdf') {
      $feed_path    = $row->display_options['path'];
      $feed_options = $row->display_options['style_options'];
      $item_plugin  = $row->display_options['row_plugin'];
      $item_options = $row->display_options['row_options'];  

      if ($defaults = db_result(db_query("SELECT vd.display_options FROM {views_display} vd WHERE vd.display_plugin = 'default' AND vd.vid = %d", $row->vid))) {
        $row->display_options = array_merge_recursive(unserialize($defaults), $row->display_options);    
      }

      $feeds['views_' . $row->name] = array(
        'type'        => RDF_RSS_URI,
        'path'        => $feed_path,
        'title'       => $row->display_options['title'],
        'module'      => 'views',
        'callback'    => NULL, // N/A
        'enabled'     => TRUE, // TODO: only if the view itself is enabled
      );
    }
  }
  return $feeds;
}

function rdf_views_get_feed_settings($feed_id, array $defaults = array()) {
  $feed_id   = is_object($feed_id) ? $feed_id->id : $feed_id;
  $view_name = substr($feed_id, strlen('views_'));
  $result    = db_query("SELECT vd.vid, vd.id, vd.display_options FROM {views_view} v LEFT JOIN {views_display} vd ON v.vid = vd.vid WHERE v.name = '%s' AND vd.display_plugin = 'feed'", $view_name);
  while ($row = db_fetch_object($result)) {
    $row->display_options = unserialize($row->display_options);
    if (isset($row->display_options['style_plugin']) && $row->display_options['style_plugin'] == 'rdf') {
      return array_merge((array)$row->display_options['style_options'], (array)$row->display_options['row_options']);
    }
  }
  return $defaults;
}

function rdf_views_set_feed_settings($feed_id, array $settings) {
  $feed_id   = is_object($feed_id) ? $feed_id->id : $feed_id;
  $view_name = substr($feed_id, strlen('views_'));
  $result    = db_query("SELECT vd.vid, vd.id, vd.display_options FROM {views_view} v LEFT JOIN {views_display} vd ON v.vid = vd.vid WHERE v.name = '%s' AND vd.display_plugin = 'feed'", $view_name);
  while ($row = db_fetch_object($result)) {
    $row->display_options = unserialize($row->display_options);
    if (isset($row->display_options['style_plugin']) && $row->display_options['style_plugin'] == 'rdf') {
      $item_options = array_intersect_key($settings, array('item_length' => '', 'date_timezone' => ''));
      $feed_options = array_diff_key($settings, $item_options);
      if (($row->display_options['style_options'] != $feed_options) || ($row->display_options['row_options'] != $item_options)) {
        $row->display_options['style_options'] = $feed_options;
        $row->display_options['row_options']   = $item_options;
        return db_query("UPDATE {views_display} vd SET vd.display_options = '%s' WHERE vd.vid = %d AND vd.id = '%s'", serialize($row->display_options), $row->vid, $row->id);
      }
      return TRUE;
    }
  }
  return FALSE;
}
