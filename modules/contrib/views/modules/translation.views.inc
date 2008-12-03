<?php
// $Id: translation.views.inc,v 1.2 2008/10/01 17:20:58 merlinofchaos Exp $

/**
 * @file
 *
 * Provide views data and handlers for translation.module
 */

/**
 * @defgroup views_translation_module taxonomy.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data_alter().
 *
 * Add translation information to the node table.
 */
function translation_views_data_alter(&$data) {

  // Joins
  $data['node']['table']['join']['node'] = array(
	  'left_field' => 'tnid',
	  'field' => 'tnid',
  );

  // Language field
  $data['node']['language'] = array(
    'group' => t('Node translation'),
    'title' => t('Language'),
    'help' => t('The language the content is in.'),
    'field' => array(
      'handler' => 'views_handler_field_node_language',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_node_language',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_language',
    ),
  );

  // The translation ID (nid of the "source" translation)
  $data['node']['tnid'] = array(
    'group' => t('Node translation'),
    'title' => t('Translation set node ID'),
    'help' => t('The ID of the translation set the content belongs to.'),
    'field' => array(
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_node_tnid',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_tnid',
      'parent' => 'views_handler_argument_numeric', // make sure parent is included
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'tnid',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'relationship' => array(
      'title' => t('Source translation'),
      'help' => t('The source that this content was translated from.'),
      'base' => 'node',
      'base field' => 'nid',
      'handler' => 'views_handler_relationship_translation',
      'label' => t('Source translation'),
    ),
  );

  // The source translation.
  $data['node']['translation'] = array(
    'group' => t('Node translation'),
    'title' => t('Translations'),
    'help' => t('Versions of content in different languages.'),
    'relationship' => array(
      'title' => t('Translations'),
      'help' => t('Versions of content in different languages.'),
      'base' => 'node',
      'base field' => 'tnid',
      'relationship table' => 'node',
      'relationship field' => 'nid',
      'handler' => 'views_handler_relationship_translation',
      'label' => t('Translations'),
    ),
  );

  // Translation status
  $data['node']['translate'] = array(
    'group' => t('Node translation'),
    'title' => t('Translation status'),
    'help' => t('The translation status of the node--whether or not the translation needs to be updated.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

}

/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function translation_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/translation',
    ),
    'handlers' => array(
      // field handlers
      'views_handler_field_node_language' => array(
        'parent' => 'views_handler_field_node',
      ),
      'views_handler_field_node_tnid' => array(
        'parent' => 'views_handler_field_node',
      ),
      'views_handler_field_node_translation_link' => array(
        'parent' => 'views_handler_field',
      ),
      // argument handlers
      'views_handler_argument_node_language' => array(
        'parent' => 'views_handler_argument',
      ),
      'views_handler_argument_node_tnid' => array(
        'parent' => 'views_handler_argument',
      ),

      // filters
      'views_handler_filter_node_language' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),

      // relationships
      'views_handler_relationship_translation' => array(
        'parent' => 'views_handler_relationship',
      ),

    ),
  );
}

/**
 * @}
 */
