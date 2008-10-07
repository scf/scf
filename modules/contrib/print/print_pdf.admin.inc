<?php
/* $Id: print_pdf.admin.inc,v 1.1.2.5 2008/05/23 11:50:57 jcnventura Exp $ */

/**
 * @file
 * Contains the administrative functions of the PDF version module.
 *
 * This file is included by the PDF version module, and includes the
 * settings form.
 */

/**
 * Menu callback for the PDF version module settings form.
 *
 * @ingroup forms
 * @see _print_pdf_tools()
 */
function print_pdf_settings() {
  $print_pdf_settings = variable_get('print_pdf_settings', print_pdf_settings_default());

  $pdf_tools = _print_pdf_tools();

  if ($pdf_tools != -1) {
    $form['print_pdf_settings'] = array(
        '#type' => 'fieldset',
        '#tree' => TRUE,
      );

    $form['print_pdf_settings']['pdf_tool'] = array(
      '#type' => 'radios',
      '#title' => t('PDF generation tool'),
      '#options' => $pdf_tools,
      '#default_value' => $print_pdf_settings['pdf_tool'],
      '#description' => t('This option selects the PDF generation tool being used by this module to create the PDF version.'),
      '#required' => TRUE,
    );

    $form['print_pdf_settings']['show_link'] = array(
      '#type' => 'radios',
      '#title' => t('PDF version link'),
      '#default_value' => $print_pdf_settings['show_link'],
      '#options' => array(t("None (Disabled)"), t("Text only"), t("Icon only"), t("Icon and Text")),
      '#description' => t("Enable or disable the PDF version link for each node. Even if the link is disabled, you can still view the PDF version of a node by going to printpdf/nid where nid is the numeric id of the node."),
    );

    $form['print_pdf_settings']['link_class'] = array(
      '#type' => 'textfield',
      '#title' => t('Link class'),
      '#default_value' => $print_pdf_settings['link_class'],
      '#size' => 60,
      '#maxlength' => 250,
      '#description' => t('This can be used by themers to change the link style or by jQuery modules to open in a new window (e.g. greybox or thickbox). Multiple classes can be specified, separated by spaces.'),
    );

    $form['print_pdf_settings']['show_sys_link'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show link in system (non-content) pages'),
      '#default_value' => $print_pdf_settings['show_sys_link'],
      '#description' => t('Setting this option will add a PDF version page link on pages created by Drupal or the enabled modules.'),
    );

    $form['print_pdf_settings']['book_link'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show link in book nodes'),
      '#default_value' => $print_pdf_settings['book_link'],
      '#description' => t('Activate this to add a PDF version link in book nodes. Requires the (core) book module.'),
    );

    $form['print_pdf_settings']['content_disposition'] = array(
      '#type' => 'radios',
      '#title' => t('Open PDF in'),
      '#options' => array(t('Same browser window'), t('New browser window'), t('Save dialog')),
      '#default_value' => $print_pdf_settings['content_disposition'],
      '#description' => t("Select the desired method for opening the PDF in the user's browser."),
    );

    $form['print_pdf_settings']['paper_size'] = array(
      '#type' => 'select',
      '#title' => t('Paper size'),
      '#options' => array('4A0' => '4A0', '2A0' => '2A0', 'A0' => 'A0',
                          'A1' => 'A1', 'A2' => 'A2', 'A3' => 'A3', 'A4' => 'A4',
                          'A5' => 'A5', 'A6' => 'A6', 'A7' => 'A7', 'A8' => 'A8',
                          'A9' => 'A9', 'A10' => 'A10', 'B0' => 'B0', 'B1' => 'B1',
                          'B2' => 'B2', 'B3' => 'B3', 'B4' => 'B4', 'B5' => 'B5',
                          'B6' => 'B6', 'B7' => 'B7', 'B8' => 'B8', 'B9' => 'B9',
                          'B10' => 'B10', 'C0' => 'C0', 'C1' => 'C1', 'C2' => 'C2',
                          'C3' => 'C3', 'C4' => 'C4', 'C5' => 'C5', 'C6' => 'C6',
                          'C7' => 'C7', 'C8' => 'C8', 'C9' => 'C9', 'C10' => 'C10',
                          'RA0' => 'RA0', 'RA1' => 'RA1', 'RA2' => 'RA2',
                          'RA3' => 'RA3', 'RA4' => 'RA4', 'SRA0' => 'SRA0',
                          'SRA1' => 'SRA1', 'SRA2' => 'SRA2', 'SRA3' => 'SRA3',
                          'SRA4' => 'SRA4', 'LETTER' => 'Letter', 'LEGAL' => 'Legal',
                          'EXECUTIVE' => 'Executive', 'FOLIO'=> 'Folio',
                         ),
      '#default_value' => $print_pdf_settings['paper_size'],
      '#description' => t("Choose the paper size of the generated PDF."),
    );

    $form['print_pdf_settings']['page_orientation'] = array(
      '#type' => 'select',
      '#title' => t('Page orientation'),
      '#options' => array('portrait' => t('Portrait'), 'landscape' => t('Landscape')),
      '#default_value' => $print_pdf_settings['page_orientation'],
      '#description' => t("Choose the page orientation of the generated PDF."),
    );
  }
  else {
    $form['print_pdf_settings'] = array(
      '#type' => 'markup',
      '#value' => '<p>'. t("No PDF generation tool found! Please dowload a supported PHP PDF generation tool. Check this module's INSTALL.txt for more details.") .'</p>',
    );
  }

  return system_settings_form($form);
}

/**
 * Auxiliary function to locate suitable PDF generation tools
 *
 * @return
 *   array of filenames with the include-able PHP file of the located tools
 */
function _print_pdf_tools() {
  $tools = array_keys(file_scan_directory(drupal_get_path('module', 'print'), '^dompdf_config.inc.php$'));
  $tools = array_merge($tools, array_keys(file_scan_directory(drupal_get_path('module', 'print'), '^tcpdf.php$')));

  $num_tools = count($tools);

  if ($num_tools == 0) {
    $print_pdf_settings = variable_get('print_pdf_settings', print_pdf_settings_default());
    $print_pdf_settings['show_link'] = 0;
    variable_set('print_pdf_settings', $print_pdf_settings);
    return -1;
  }
  else {
    // Instead of array_combine(), use this to maintain PHP4 compatibility
    $tools2 = array();
    foreach ($tools as $key => $val) {
      $tools2[$val] = $tools[$key];
    }

    return $tools2;
  }
}
