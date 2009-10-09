<?php
// $Id: sparql.admin.inc,v 1.3 2008/06/06 10:09:07 arto Exp $
/**
 * sparql.admin.inc - SPARQL API admin page callbacks.
 *
 * @author Arto Bendiken <http://bendiken.net/>
 * @copyright Copyright (c) 2007-2008 Arto Bendiken. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package sparql.module
 */

//////////////////////////////////////////////////////////////////////////////
// SPARQL settings form

function sparql_admin_settings() {
  $form = array();

  $form['server'] = array('#type' => 'fieldset', '#title' => t('SPARQL endpoint settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['server']['sparql_endpoint'] = array('#type' => 'radios', '#title' => t('SPARQL endpoint'), '#default_value' => (int)SPARQL_ENDPOINT, '#options' => array(t('Disabled'), t('Enabled')), '#description' => t(''));
  $form['server']['sparql_features'] = array('#type' => 'checkboxes', '#title' => t('Enabled features'), '#default_value' => array_filter(variable_get('sparql_features', array('select', 'ask', 'construct', 'describe')), 'is_string'), '#options' => array('select' => t('SELECT'), 'ask' => t('ASK'), 'construct' => t('CONSTRUCT'), 'describe' => t('DESCRIBE')), '#description' => t(''));

  return system_settings_form($form);
}
