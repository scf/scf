<?php
// $Id: node_from_user.inc,v 1.1.2.1 2009/06/15 10:55:24 fago Exp $

/**
 * @file
 * Provides a CTools (Panels) relationship that gets a node context from the
 * user context based on nodes marked as content profiles.
 */

/**
 * Implementation of specially named hook_ctools_relationships().
 */
function content_profile_node_from_user_ctools_relationships() {
  $args['node_from_user'] = array(
    'title' => t("Profile Node"),
    'keyword' => 'content_profile',
    'description' => t('Adds a Content Profile from user context'),
    'required context' => new ctools_context_required(t('User'), 'user'),
    'context' => 'content_profile_node_from_user_ctools_context',
    'settings form' => 'content_profile_node_from_user_ctools_settings_form',
    'settings form validate' => 'content_profile_node_from_user_ctools_settings_form_validate',
  );
  
  return $args;
}

/**
 * Return a new context based on an existing context
 */
function content_profile_node_from_user_ctools_context($context = NULL, $conf) {
  // If unset it wants a generic, unfilled context, which is just NULL
  if (empty($context->data)) {
    return ctools_context_create_empty('node', NULL);
  }

  if (isset($context->data->uid)) {
    // Load the node for the requested type
    $uid = $context->data->uid;
    $content_profile_node = content_profile_load($conf['type'], $uid);

    // Send it to ctools
    return ctools_context_create('node', $content_profile_node);
  }
  else {
    return ctools_context_create_empty('node', NULL);
  }
}

/**
 * Settings form for the relationship
 */
function content_profile_node_from_user_ctools_settings_form($conf) {
  $options = content_profile_get_types('names');
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Relationship type'),
    '#options' => $options,
    '#default_value' => $conf['type']
  );

  return $form;
}