<?php
// $Id: ajax.inc,v 1.12 2008/04/25 23:42:42 merlinofchaos Exp $

/**
 * @file ajax.inc
 *
 * Handles the server side AJAX interactions of Views.
 *
 * @defgroup ajax Views ajax library
 * @{
 */

/**
 * Simple render function to make sure output is what we want.
 *
 * This function renders an object into JSON, and that object contains
 * commands to the ajax response parser on the other side. The actual
 * commands that can be sent are completely dependent upon the client
 * javascript parser, which can be anything, but this function assumes
 * that 'display', at least, will be displayed in some kind of ajax
 * spot or popup.
 */
function views_ajax_render($output = NULL, $title = NULL, $url = NULL, $js = NULL) {
  if (empty($output)) {
    $output->display = t('Server reports invalid input error.');
    $output->title = t('Error');
  }
  elseif (!is_object($output)) {
    $temp = new stdClass();
    $temp->display = $output;
    $temp->title = $title;
    $temp->url = $url;
    $output = $temp;
  }
  if (!empty($js)) {
    $output->js = $js;
  }

  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  print drupal_to_js($output);
  exit;
}

/**
 * Wrapper around drupal_build_form to handle some AJAX stuff automatically.
 * This makes some assumptions about the client.
 */
function views_ajax_form_wrapper($form_id, &$form_state) {
  // This won't override settings already in.
  $form_state += array(
    're_render' => FALSE,
    'no_redirect' => !empty($form_state['ajax']),
  );

  $output = drupal_build_form($form_id, $form_state);
  if (!empty($form_state['ajax']) && empty($form_state['executed'])) {
    // If the form didn't execute and we're using ajax, build up a
    // json command object to render.
    $object = new stdClass();
    $object->display = '';
    if ($messages = theme('status_messages')) {
      $object->display = '<div class="views-messages">' . $messages . '</div>';
    }
    $object->display .= $output;

    $object->title = empty($form_state['title']) ? '' : $form_state['title'];
    if (!empty($form_state['help_topic'])) {
      $module = !empty($form_state['help_module']) ? $form_state['help_module'] : 'views';
      $object->title = theme('advanced_help_topic', $module, $form_state['help_topic']) . $object->title;
    }
    $object->url = empty($form_state['url']) ? url($_GET['q'], array('absolute' => TRUE)) : $form_state['url'];
    $object->js = empty($form_state['js settings']) ? NULL : $form_state['js settings'];
    if (!empty($form_state['#section'])) {
      $object->hilite = '.' . views_ui_item_css($form_state['#section']);
    }

    $output = $object;
  }

  // These forms have the title built in, so set the title here:
  if (empty($form_state['ajax']) && !empty($form_state['title'])) {
    drupal_set_title($form_state['title']);
  }

  return $output;
}

/**
 * @}
 */