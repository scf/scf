<?php
// $Id: rdf.menu.inc,v 1.2 2009/03/25 22:55:20 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Core API hooks

/**
 * Implementation of hook_menu().
 */
function rdf_menu_build() {
  return array(
    // Export site information
    'rdf' => array(
      'title' => 'RDF',
      'type' => MENU_CALLBACK,
      'access callback' => 'user_access',
      'access arguments' => array('access RDF data'),
      'page callback' => 'rdf_export_site',
      'file' => 'rdf.pages.inc',
    ),

    'rdf/schema' => array(
      'title' => 'RDF schema',
      'type' => MENU_CALLBACK,
      'access arguments' => array('access RDF data'),
      'page callback' => 'rdf_export',
      'page arguments' => array('rdf_schema_data', 'schema'),
      'file' => 'rdf.pages.inc', // HACK
    ),
    'rdf/settings' => array(
      'type' => MENU_CALLBACK,
      'access callback' => 'rdf_menu_access_http',
      'access arguments' => array('export site settings', 'import site settings'),
      'page callback' => 'rdf_schema_export_settings',
      'file' => 'rdf.pages.inc', // HACK
    ),
    'rdf/modules' => array(
      'type' => MENU_CALLBACK,
      'access callback' => 'rdf_menu_access_http',
      'access arguments' => array('export enabled modules', 'import enabled modules'),
      'page callback' => 'rdf_schema_export_modules',
      'file' => 'rdf.pages.inc', // HACK
    ),

    // Export user information
    'user/%user/rdf' => array(
      'title' => 'RDF',
      'type' => MENU_CALLBACK,
      'access callback' => 'rdf_menu_access_user',
      'access arguments' => array(1),
      'page callback' => 'rdf_export_user',
      'page arguments' => array(1),
      'file' => 'rdf.pages.inc',
    ),

    // Export node information
    'node/%node/rdf' => array(
      'title' => 'RDF',
      'type' => MENU_CALLBACK,
      'access callback' => 'rdf_menu_access_node',
      'access arguments' => array('view', 1),
      'page callback' => 'rdf_export_node',
      'page arguments' => array(1),
      'file' => 'rdf.pages.inc',
    ),

    // Administer >> Content management >> RDF data
    'admin/content/rdf' => array(
      'title' => 'RDF data',
      'description' => 'Query, create, and delete RDF statements.',
      'access arguments' => array('administer RDF data'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_admin_data'),
      'file' => 'rdf.admin.inc',
    ),
    'admin/content/rdf/list' => array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
    ),
    'admin/content/rdf/add' => array(
      'title' => 'Add statement',
      'type' => MENU_LOCAL_TASK,
      'weight' => 1,
      'access arguments' => array('administer RDF data'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_db_admin_data_form'),
      'file' => 'rdf.admin.inc',
    ),
    'admin/content/rdf/import' => array(
      'title' => 'Import data',
      'type' => MENU_LOCAL_TASK,
      'weight' => 20,
      'access arguments' => array('import RDF data'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_import_admin_screen'),
      'file' => 'rdf.admin.inc',
    ),
    'admin/content/rdf/export' => array(
      'title' => 'Export data',
      'type' => MENU_LOCAL_TASK,
      'weight' => 30,
      'access arguments' => array('export RDF data'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_export_admin_screen'),
      'file' => 'rdf.admin.inc',
    ),

    // Administer >> Site configuration >> RDF settings
    'admin/settings/rdf' => array(
      'title' => 'RDF settings',
      'description' => 'Settings for the RDF API.',
      'access arguments' => array('administer site configuration'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_admin_settings'),
      'file' => 'rdf.admin.inc',
    ),

    // Administer >> Site configuration >> RDF settings >> Mappings
    'admin/settings/rdf/mappings' => array(
      'title' => 'Mappings',
      'access arguments' => array('administer site configuration'),
      'page callback' => 'rdf_admin_mappings',
      'file' => 'rdf.admin.inc',
      'weight' => 10,
    ),
    'admin/settings/rdf/mappings/list' => array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
    ),
    'rdf/autocomplete/resource' => array(
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer RDF data'),
      'page callback' => 'rdf_admin_autocomplete_resource',
      'file' => 'rdf.admin.inc',
    ),
    'rdf/autocomplete/class' => array(
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer RDF data'),
      'page callback' => 'rdf_admin_autocomplete_class',
      'file' => 'rdf.admin.inc',
    ),
    'rdf/autocomplete/property' => array(
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer RDF data'),
      'page callback' => 'rdf_admin_autocomplete_property',
      'file' => 'rdf.admin.inc',
    ),

    // Administer >> Site configuration >> RDF settings >> Feeds
    'admin/settings/rdf/feeds' => array(
      'title' => 'Feeds',
      'access arguments' => array('administer site configuration'),
      'page callback' => 'rdf_admin_feeds',
      'file' => 'rdf.admin.inc',
      'weight' => 20,
    ),
    'admin/settings/rdf/feeds/list' => array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
    ),
    'admin/settings/rdf/feeds/edit/%' => array(
      'title' => 'Configure RDF feed',
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer site configuration'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_admin_feed', 5),
      'file' => 'rdf.admin.inc',
    ),

    // Administer >> Site configuration >> RDF settings >> Namespaces
    'admin/settings/rdf/namespaces' => array(
      'title' => 'Namespaces',
      'access arguments' => array('administer site configuration'),
      'page callback' => 'rdf_admin_namespaces',
      'file' => 'rdf.admin.inc',
      'weight' => 30,
    ),
    'admin/settings/rdf/namespaces/list' => array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
    ),
    'admin/settings/rdf/namespaces/add' => array(
      'title' => 'Add namespace',
      'type' => MENU_LOCAL_TASK,
      'weight' => 1,
      'access arguments' => array('administer site configuration'),
      'page callback' => 'rdf_db_admin_ns_edit',
      'file' => 'rdf.admin.inc',
    ),
    'admin/settings/rdf/namespaces/edit' => array(
      'title' => 'Edit namespace',
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer site configuration'),
      'page callback' => 'rdf_db_admin_ns_edit',
      'file' => 'rdf.admin.inc',
    ),
    'admin/settings/rdf/namespaces/delete' => array(
      'title' => 'Delete namespace',
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer site configuration'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_db_admin_ns_delete'),
      'file' => 'rdf.admin.inc',
    ),

    // Administer >> Site configuration >> RDF settings >> Contexts
    'admin/settings/rdf/contexts' => array(
      'title' => 'Contexts',
      'access arguments' => array('administer site configuration'),
      'page callback' => 'rdf_admin_contexts',
      'file' => 'rdf.admin.inc',
      'weight' => 40,
    ),
    'admin/settings/rdf/contexts/list' => array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
    ),

    // Administer >> Site configuration >> RDF settings >> Repositories
    'admin/settings/rdf/repositories' => array(
      'title' => 'Repositories',
      'access arguments' => array('administer RDF repositories'),
      'page callback' => 'rdf_admin_repositories',
      'file' => 'rdf.admin.inc',
      'weight' => 50,
    ),
    'admin/settings/rdf/repositories/list' => array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
    ),
    'admin/settings/rdf/repositories/rdf/add' => array(
      'title' => 'Add local repository',
      'type' => MENU_LOCAL_TASK,
      'weight' => 1,
      'access arguments' => array('administer RDF repositories'),
      'page callback' => 'rdf_db_admin_db_edit',
      'file' => 'rdf.admin.inc',
    ),
    'admin/settings/rdf/repositories/rdf/edit' => array(
      'title' => 'Edit local repository',
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer RDF repositories'),
      'page callback' => 'rdf_db_admin_db_edit',
      'file' => 'rdf.admin.inc',
    ),
    'admin/settings/rdf/repositories/rdf/delete' => array(
      'title' => 'Delete local repository',
      'type' => MENU_CALLBACK,
      'access arguments' => array('administer RDF repositories'),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('rdf_db_admin_db_delete'),
      'file' => 'rdf.admin.inc',
    ),
  );
}
