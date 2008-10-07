<?php
// $Id: pngfix.admin.inc,v 1.1.2.3 2008/02/06 17:02:25 robloach Exp $

/**
 * @file
 * Provides the administration page for PNG Fix.
 */
  
/**
 * Administration settings page
 */ 
function pngfix_admin() {
  $form = array();
  $form['pngfix_css_classes'] = array(
    '#type' => 'textfield',
    '#title' => t('CSS Classes'),
    '#default_value' => variable_get('pngfix_css_classes', ''),
    '#description' => t('Comma seperated list of CSS classes to apply the PNG Fix to (use <a href="@jqueryselector">jQuery Selectors</a>).  The CSS class must be for the element that surrounds the image, and not for the image itself.  In the case of <code>&lt;div class="pngfix"&gt;&lt;img ... /&gt;&lt;/div&gt;</code>, use .pngfix to apply the PNG Fix to the image.', array('@jqueryselector' => 'http://docs.jquery.com/Selectors')),
  );
  return system_settings_form($form);
} // function pngfix_admin
