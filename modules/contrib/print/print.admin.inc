<?php
// $Id: print.admin.inc,v 1.1.2.7 2008/05/23 11:50:57 jcnventura Exp $

/**
 * @file
 * Contains the administrative functions of the PF module.
 *
 * This file is included by the core PF module, and includes the
 * settings form.
 */

/**
 * Menu callback for the Printer-friendly pages module settings form.
 *
 * @ingroup forms
 */
function print_main_settings() {
  $print_settings = variable_get('print_settings', print_settings_default());
  
  $form['print_settings'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );

  $form['print_settings']['logo_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Logo URL'),
    '#default_value' => $print_settings['logo_url'],
    '#size' => 60,
    '#maxlength' => 250,
    '#description' => t('An alternative logo to display on the printer-friendly version. If left empty, the current theme\'s logo is used.'),
  );

  $form['print_settings']['css'] = array(
    '#type' => 'textfield',
    '#title' => t('Stylesheet URL'),
    '#default_value' => $print_settings['css'],
    '#size' => 60,
    '#maxlength' => 64,
    '#description' => t('The URL to your custom print cascading stylesheet, if any. When none is specified, the default module CSS file is used.'),
  );

  $form['print_settings']['urls'] = array(
    '#type' => 'checkbox',
    '#title' => t('Printer-friendly URLs list'),
    '#default_value' => $print_settings['urls'],
    '#description' => t('If set, a list of the destination URLs for the page links will be displayed at the bottom of the page.'),
  );

  $form['print_settings']['comments'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include comments in printer-friendly version'),
    '#default_value' => $print_settings['comments'],
    '#description' => t('When this option is active, user comments are also included in the printer-friendly version. Requires the comment module.'),
  );

  $form['print_settings']['newwindow'] = array(
    '#type' => 'radios',
    '#title' => t('New window method'),
    '#options' => array(t("Use HTML target (does not validate as XHTML Strict)"), t("Use Javascript (requires browser support)")),
    '#default_value' => $print_settings['newwindow'],
    '#description' => t('Choose the method used to open pages in a new window/tab.'),
  );

  $print_sourceurl_settings = variable_get('print_sourceurl_settings', print_sourceurl_settings_default());

  $form['print_sourceurl_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Source URL'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
  );

  $form['print_sourceurl_settings']['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display source URL'),
    '#default_value' => $print_sourceurl_settings['enabled'],
    '#description' => t('When this option is selected, the URL for the original page will be displayed at the bottom of the printer-friendly version.'),
  );

  $form['print_sourceurl_settings']['date'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add current time/date to the source URL'),
    '#default_value' => $print_sourceurl_settings['date'],
    '#description' => t('Display the current date and time in the Source URL line.'),
  );

  $form['print_sourceurl_settings']['forcenode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Force use of node ID in source URL'),
    '#default_value' => $print_sourceurl_settings['forcenode'],
    '#description' => t('Drupal will attempt to use the page\'s defined alias in case there is one. To force the use of the fixed URL, activate this option.'),
  );

  return system_settings_form($form);
}

function print_html_settings() {
  $print_html_settings = variable_get('print_html_settings', print_html_settings_default());

  $form['print_html_settings'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );

  $form['print_html_settings']['show_link'] = array(
    '#type' => 'radios',
    '#title' => t('Printer-friendly page link'),
    '#default_value' => $print_html_settings['show_link'],
    '#options' => array(t("None (Disabled)"), t("Text only"), t("Icon only"), t("Icon and Text")),
    '#description' => t("Enable or disable the printer-friendly page link for each node. Even if the link is disabled, you can still view the print version of a node by going to print/nid where nid is the numeric id of the node."),
  );

  $form['print_html_settings']['link_class'] = array(
    '#type' => 'textfield',
    '#title' => t('Link class'),
    '#default_value' => $print_html_settings['link_class'],
    '#size' => 60,
    '#maxlength' => 250,
    '#description' => t('This can be used by themers to change the link style or by jQuery modules to open in a new window (e.g. greybox or thickbox). Multiple classes can be specified, separated by spaces.'),
  );

  $form['print_html_settings']['show_sys_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show link in system (non-content) pages'),
    '#default_value' => $print_html_settings['show_sys_link'],
    '#description' => t('Setting this option will add a printer-friendly version page link on pages created by Drupal or the enabled modules.'),
  );

  $form['print_html_settings']['book_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Take control of the book module printer-friendly link'),
    '#default_value' => $print_html_settings['book_link'],
    '#description' => t('Activate this to have the printer-friendly link in book nodes handled by this module. Requires the (core) book module.'),
  );

  $form['print_html_settings']['new_window'] = array(
    '#type' => 'checkbox',
    '#title' => t('Open the printer-friendly version in a new window'),
    '#default_value' => $print_html_settings['new_window'],
    '#description' => t('Setting this option will make the printer-friendly version open in a new window/tab.'),
  );

  $form['print_html_settings']['sendtoprinter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send to printer'),
    '#default_value' => $print_html_settings['sendtoprinter'],
    '#description' => t('Automatically calls the browser\'s print function when the printer-friendly version is displayed.'),
  );

  $print_robot_settings = variable_get('print_robot_settings', print_robot_settings_default());

  $form['print_robot_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Robots META tags'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
  );

  $form['print_robot_settings']['noindex'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add noindex'),
    '#default_value' => $print_robot_settings['noindex'],
    '#description' => t('Instruct robots to not index printer-friendly pages. Recommended for good search engine karma.'),
  );

  $form['print_robot_settings']['nofollow'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add nofollow'),
    '#default_value' => $print_robot_settings['nofollow'],
    '#description' => t('Instruct robots to not follow outgoing links on printer-friendly pages.'),
  );

  $form['print_robot_settings']['noarchive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add noarchive'),
    '#default_value' => $print_robot_settings['noarchive'],
    '#description' => t('Non-standard tag to instruct search engines to not show a "Cached" link for your printer-friendly pages. Recognized by Googlebot.'),
  );

  $form['print_robot_settings']['nocache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add nocache'),
    '#default_value' => $print_robot_settings['nocache'],
    '#description' => t('Non-standard tag to instruct search engines to not show a "Cached" link for your printer-friendly pages'),
  );

  return system_settings_form($form);
}
