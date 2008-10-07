<?php
// $Id: poll.views.inc,v 1.2 2008/03/12 04:32:07 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers for poll.module
 */

/**
 * @defgroup views_poll_module poll.module handlers
 *
 * Includes only the core 'poll' table for now.
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function poll_views_data() {
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['poll']['table']['group']  = t('Poll');

  // For other base tables, explain how we join
  $data['poll']['table']['join'] = array(
    'node' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('poll', 'node', 'nid', 'nid'),
     ),
  );

  // ----------------------------------------------------------------
  // Fields

  // title
  $data['poll']['active'] = array(
    'title' => t('Active'), // The item it appears as on the UI,
    'help' => t('Whether the poll is open for voting.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
     ),
     'filter' => array(
       'handler' => 'views_handler_filter_boolean_operator',
       'label' => t('Active'),
       ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}

/**
 * @}
 */
