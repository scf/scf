<?php

// $Id: scf.admin.inc $

/**
 * @file
 * Admin page callbacks for the SCF module
 */


function scf_admin_settings_form () {
  $wt = -5;

  $form['scf_settings'] = array(
      '#type' => 'fieldset'
  );

  $form['scf_settings']['scf_usurp_front_page'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow SCF to control front page?'),
      '#default_value' => variable_get('scf_usurp_front_page', TRUE),
      '#required' => TRUE,
      '#weight' => $wt++
  );
  
  $form['scf_settings']['scf_author_rid'] = array(
      '#type' => 'select',
      '#title' => t('Role to use for author selection.'),
      '#options' => user_roles(TRUE),
      '#default_value' => variable_get('scf_author_rid', DRUPAL_AUTHENTICATED_RID),
      '#required' => TRUE,
      '#description' => t('A user with this role will appear in author selectors for assigning authorship to content items.'),
      '#weight' => $wt++
  );
  
  $form['scf_settings']['scf_pub_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Publication title'),
      '#default_value' => variable_get('scf_pub_title', t('Publications')),
      '#required' => TRUE,
      '#weight' => $wt++
  );
  
  /*
  $form['scf_settings']['scf_pub_description'] = array(
      '#type' => 'textarea',
      '#title' => t('Publication description'),
      '#weight' => $wt++
  );
  */


  foreach (module_implements('admin_settings_subform') as $mod) {
    $subform = module_invoke($mod, 'admin_settings_subform');
    if (!empty($subform)) {
      $form['scf_settings'] += $subform;
    }
  }

  return system_settings_form($form);
}

