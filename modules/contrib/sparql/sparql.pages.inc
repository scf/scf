<?php
// $Id: sparql.pages.inc,v 1.1 2008/04/28 13:59:49 arto Exp $
/**
 * sparql.pages.inc - SPARQL API menu callbacks.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package sparql.module
 */

//////////////////////////////////////////////////////////////////////////////
// SPARQL query screen

function sparql_endpoint() {
  if (!SPARQL_ENDPOINT) {
    die(drupal_not_found());
  }

  drupal_add_css(drupal_get_path('module', 'sparql') .'/sparql.css');

  $endpoint = isset($_GET['endpoint']) ? $_GET['endpoint'] : '';
  $graph = isset($_GET['default-graph-uri']) ? $_GET['default-graph-uri'] : '';
  $query = isset($_GET['query']) ? $_GET['query'] : '';
  $debug = isset($_GET['debug']) ? !empty($_GET['debug']) : '';

  $form = array();

  $form['query'] = array('#type' => 'fieldset', '#title' => t('Query'), '#prefix' => '<div class="container-inline">', '#suffix' => '</div>');
  $form['query']['query'] = array('#type' => 'textarea', '#title' => '', '#default_value' => $query, '#rows' => 8, '#prefix' => '<div class="sparql-query-text">', '#suffix' => '</div>');
  //$form['query']['debug'] = array('#type' => 'checkbox', '#title' => 'Debug', '#default_value' => $debug, '#prefix' => '<div class="sparql-query-debug">', '#suffix' => '</div>');
  $form['query']['submit'] = array('#type' => 'submit', '#value' => t('Query'), '#prefix' => '<div class="sparql-query-button">', '#suffix' => '</div>');

  $form['options'] = array('#type' => 'fieldset', '#title' => t('Options'), '#collapsible' => TRUE, '#collapsed' => TRUE, '#prefix' => '<div class="sparql-query-options">', '#suffix' => '</div>');
  $form['options']['endpoint'] = array('#type' => 'textfield', '#title' => 'Endpoint URL', '#default_value' => $endpoint, '#required' => TRUE, '#description' => t(''));
  $form['options']['default-graph-uri'] = array('#type' => 'textfield', '#title' => 'Default Graph URI', '#default_value' => $graph, '#description' => t(''));

  return $form;
}

function sparql_endpoint_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $graph = $form_state['values']['default-graph-uri'];
}

function sparql_endpoint_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $graph = $form_state['values']['default-graph-uri'];

  $query = array_filter(array('endpoint' => $endpoint, 'default-graph-uri' => $graph, 'query' => $query), 'strlen');
  $form_state['redirect'] = array('sparql', http_build_query($query, NULL, '&'));
}

//////////////////////////////////////////////////////////////////////////////

function sparql_render_query_debug($query) {
  $query = sparql_parse($query);
  $form['debug'] = array('#value' => highlight_string(var_export($query, TRUE), TRUE));
  return drupal_render($form['debug']);
}

function sparql_render_query_results($query, $options = NULL) {
  $output = '';

  if (empty($query)) {
    if (isset($_GET['query'])) {
      drupal_set_message(t('No query specified.'), 'error');
    }
  }
  else {
    timer_start('sparql_query');
    $result = sparql_query($query, $options, $errors);
    if (!empty($errors)) {
      foreach ($errors as $error) {
        drupal_set_message($error, 'error');
      }
    }
    else {
      list(, $time) = array_values(timer_stop('sparql_query'));

      $form['time'] = array('#value' => t('Query execution took @time.', array('@time' => format_plural(round($time / 1000.0, 2), '1 second', '@count seconds'))), '#prefix' => '<div class="sparql-query-time">', '#suffix' => '</div>');
      $output .= drupal_render($form['time']);
      $output .= theme('sparql_results', $result);
    }
  }

  return $output;
}

function theme_sparql_endpoint($form) {
  $debug = !empty($form['query']['debug']['#value']);
  $query = $form['query']['query']['#value'];
  foreach (array('endpoint', 'graph') as $var) {
    if (isset($form['options'][$var]['#value'])) {
      $$var = $form['options'][$var]['#value'];
    }
  }
  $options = compact('endpoint', 'graph');

  $output = drupal_render($form['query']);
  $output .= drupal_render($form['options']);
  $output .= !$debug ? sparql_render_query_results($query, $options) : sparql_render_query_debug($query, $options);
  $output .= drupal_render($form);
  return $output;
}

function theme_sparql_results($result) {
  switch (gettype($result)) {
    case 'boolean': // ASK
      $result = array(array('success' => $result ? t('yes') : t('no')));
      // fall through to SELECT

    case 'array':   // SELECT
      if (empty($result)) {
        // TODO
        $form['results'] = array('#value' => ' ' . t('No results were returned.'));
        return drupal_render($form['results']);
      }
      else {
        $vars = array_keys($result[0]);
        $rows = array_map('array_values', $result);
        return theme('table', $vars, $rows, array('class' => 'sparql-results'));
      }
      break;

    case 'object':  // CONSTRUCT/DESCRIBE
      return theme('rdf_triple_table', $result);
      break;
  }
}
