<?php
// $Id: biblio.admin.inc,v 1.9.2.14 2008/05/22 21:23:36 rjerome Exp $
/**
 *   biblio.admin.inc
 *
 *   Copyright (C) 2006-2008  Ron Jerome
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**
* Implementation of hook_settings().
*/
function biblio_admin_settings() {
  $version = '$Revision: 1.9.2.14 $ $Date: 2008/05/22 21:23:36 $';
  $version = str_replace('$', '', $version);
  $form['biblio_rev'] = array(
    '#value' => $version,

  );
  $form['biblio_base'] = array(
    '#type' => 'textfield',
    '#title' => t('Base URL'),
    '#size' => 20,
    '#default_value' => variable_get('biblio_base', 'biblio'),
    '#description' => t('This sets the base URL used to access the biblio module (e.g. /biblio ).')
  );
  $form['biblio_rowsperpage'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of results per page'),
    '#default_value' => variable_get('biblio_rowsperpage', 25),
    '#size' => 6,
    '#maxlength' => 6,
    '#description' => t('This sets the number of results that will be displayed per page.')
  );
  $form['biblio_view_only_own'] = array(
    '#type' => 'checkbox',
    '#title' => t('Restrict users such that they can only view their own biblio entries'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_view_only_own', 0),
    '#description' => t('This option restricts the users capability to view biblio entries.  They will only be able to see the entries which they have created and own.')
  );
  $form['block'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Block settings'),
    '#description' => ''
  );
  $form['block']['biblio_block_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title for the "New Publications" block'),
    '#default_value' => variable_get('biblio_block_title', 'New Publications'),
    '#size' => 40,
    '#maxlength' => 40,
    '#description' => ''
  );
  $form['block']['biblio_rowsperblock'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of results in the "New Publications" block'),
    '#default_value' => variable_get('biblio_rowsperblock', 4),
    '#size' => 2,
    '#maxlength' => 2,
    '#description' => t('This sets the number of results that will be displayed in the "New Publications" block.')
  );
  $form['block']['biblio_block_order'] = array(
    '#type' => 'radios',
    '#title' => t('Order by'),
    '#default_value' => variable_get('biblio_block_order', 'n.created'),
    '#options' => array(
      'n.created' => t('Date Created'),
      'b.biblio_year' => t('Year Published')
     )
  );
  $result = db_query("SELECT b.name, btd.title FROM {biblio_fields} b, {biblio_field_type} bt, {biblio_field_type_data} btd
                      WHERE bt.fid=b.fid AND btd.ftdid=bt.ftdid
                      AND bt.tid=0
                      ORDER by btd.title ASC ");
  $schema = drupal_get_schema('biblio');
  $keys = array_keys($schema['fields']);
  $choice = new stdClass();
  $choice->option = array(
    'nid' => t('Node ID')
  );
  $options[0] = $choice;
  while ($row = db_fetch_array($result)) {
    $fields[$row['name']] = $row;
    $choice = new stdClass();
    $choice->option = array(
      $row['name'] => $row['title']
    );
    $options[] = $choice;
  }
  $form['citekey'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Citekey'),
    '#description' => t('You can alter citekey related settings here.')
  );
  $form['citekey']['biblio_auto_citekey'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto generate citekeys if not given'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_auto_citekey', 1),
    '#description' => t('This option will cause "citekey" entries to be automatically generated if a value is not provided.')
  );
  $form['citekey']['biblio_citekey_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Citekey prefix'),
    '#default_value' => variable_get('biblio_citekey_prefix', ''),
    '#size' => 10,
    '#maxlength' => 10,
    '#description' => t('This text will be combined with the field choosen below to form the auto generated citekey.')
  );
  $form['citekey']['biblio_citekey_field1'] = array(
    '#type' => 'select',
    '#title' => t('Primary Citekey  field'),
    '#default_value' => variable_get('biblio_citekey_field1', 'nid'),
    '#options' => $options,
    '#description' => t('Select the field to be used when generating citekeys.')
  );
  $form['citekey']['biblio_citekey_field2'] = array(
    '#type' => 'select',
    '#title' => t('Secondary Citekey field'),
    '#default_value' => variable_get('biblio_citekey_field2', 'nid'),
    '#options' => $options,
    '#description' => t('If the field above has no value this field will be used.')
  );
  $form['footnotes'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Footnotes'),
    '#description' => t('You can integrate with the !url module here.', array('!url' => l('footnotes', url("http://www.drupal.org/project/footnotes", array('query' => NULL, 'fragment' => NULL, 'absolute' => TRUE)))))
  );
  if (!module_exists('footnotes')) {
    $additional_text = '<div class="admin-dependencies">'. t('Depends on') .': '. t('Footnotes') .' (<span class="admin-disabled">'. t('disabled') .'</span>)</div>';
    $disabled = TRUE;
    variable_set('biblio_footnotes_integration', 0);
  }
  else {
    $additional_text = '<div class="admin-dependencies">'. t('Depends on') .': '. t('Footnotes') .' (<span class="admin-enabled">'. t('enabled') .'</span>)</div>';
    $disabled = FALSE;
  }
  $form['footnotes']['biblio_footnotes_integration'] = array(
    '#type' => 'checkbox',
    '#title' => t('Integration with the footnotes module') . $additional_text,
    '#disabled' => $disabled,
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_footnotes_integration', 0),
    '#description' => t('This will convert &lt;bib&gt; tags into &lt;fn&gt; tags.  This will cause intermingled &lt;bib&gt; and &lt;fn&gt; tags to be sequentially numbered.  For this to work, you must put the &lt;bib&gt; filter ahead of the &lt;fn&gt; filter in the filter chain.  If this option is not set, &lt;bib&gt; and &lt;fn&gt; tags will be handled separately.')
  );
  $form['keywords'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Keywords'),
    '#description' => ''
  );
  $form['keywords']['biblio_keyword_sep'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword separator'),
    '#size' => 2,
    '#default_value' => variable_get('biblio_keyword_sep', ','),
    '#description' => t('Enter the character which will be used to separate multiple keywords in the keyword field')
  );
  $form['links'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Links')
  );
  $form['links']['biblio_links_target_new_window'] = array(
    '#type' => 'checkbox',
    '#title' => t('Links open in new browser'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_links_target_new_window', 0),
    '#description' => t('This causes related URLs to open in a new browser window')
  );
  /*  $form['links']['biblio_inlinemode_in_links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Carry "inline" mode through to all links'),
      '#return_value' => 1,
      '#default_value' => variable_get('biblio_inlinemode_in_links', 0),
      '#description' => t('This causes the "inline" mode to be applied to all links such as titles, authors and keywords') );
  */
  $form['links']['biblio_link_title_url'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hyperlink titles using supplied URL if available'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_link_title_url', 0),
    '#description' => t('Selecting this links the titles to the supplied URL (if available) rather than the "node" view.')
  );
  $form['openurl'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('OpenURL'),
    '#description' => t('You can set an <a href="http://en.wikipedia.org/wiki/OpenURL">openurl</a> link here')
  );
  $form['openurl']['biblio_baseopenurl'] = array(
    '#type' => 'textfield',
    '#title' => t('OpenURL Base URL'),
    '#size' => 95,
    '#default_value' => variable_get('biblio_baseopenurl', ''),
    '#description' => t('This sets your institution\'s base <a href="http://en.wikipedia.org/wiki/OpenURL">OpenURL</a> gateway, which is used to generate OpenURL links. To implement a "Universal" OpenURL system, try using OCLC\'s <a href="http://www.oclc.org/productworks/urlresolver.htm">OpenURL Resolver Registry</a> gateway: <a href="http://worldcatlibraries.org/registry/gateway">http://worldcatlibraries.org/registry/gateway</a>')
  );
  $form['openurl']['biblio_openurlimage'] = array(
    '#type' => 'textfield',
    '#title' => t('OpenURL Image'),
    '#size' => 95,
    '#default_value' => variable_get('biblio_openurlimage', ''),
    '#description' => t('Enter a path to your image here, this image will be used as button which when clicked will find the entry via the OpenURL link')
  );
  // Add profile page settings... this is done in a fucntion so it can be reused elsewhere
  _biblio_get_user_profile_form($form);
  //
  $form['sort'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Sorting'),
    '#description' => t('You can set the default sorting and ordering for the /biblio page here.')
  );
  $form['sort']['biblio_sort'] = array(
    '#type' => 'radios',
    '#title' => t('Sort by'),
    '#default_value' => variable_get('biblio_sort', 'year'),
    '#options' => array(
      'author'  => t('Author'),
      'keyword' => t('Keyword'),
      'title'   => t('Title'),
      'type'    => t('Type'),
      'year'    => t('Year')
    )
  );
  $form['sort']['biblio_sort_tabs'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Show sort links'),
    '#default_value' => variable_get('biblio_sort_tabs', array('author', 'title', 'type', 'year')),
    '#options' => array(
      'author'  => t('Author'),
      'keyword' => t('Keyword'),
      'title'   => t('Title'),
      'type'    => t('Type'),
      'year'    => t('Year')
    ),
    '#description' => t('You turn the sorting links at the top of the /biblio page here.')
  );
  $form['sort']['biblio_order'] = array(
    '#type' => 'radios',
    '#title' => t('Order'),
    '#default_value' => variable_get('biblio_order', 'DESC'),
    '#options' => array(
      'DESC' => t('Descending'),
      'ASC' => t('Ascending')
    )
  );
  $form['style'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Styling'),
    '#description' => t('You can set the default style for the /biblio page here.')
  );
//  $form['style']['biblio_normalize'] = array(
//    '#type' => 'checkbox',
//    '#title' => t('Normalize author names when displaying biblio records'),
//    '#return_value' => 1,
//    '#default_value' => variable_get('biblio_normalize', 0),
//    '#description' => t('Tries (doesn\'t always work) to reformat author names so that they are displayed in the format "Lastname, Initials" e.g. Smith, J.S. (Note: This setting does not modify the entry in the database, it only reformats it\'s presentation. This option can be turned off at any time to diplay the oringal format.)')
//  );
  $form['style']['biblio_node_layout'] = array(
    '#type' => 'radios',
    '#title' => t('Node Layout'),
    '#default_value' => variable_get('biblio_node_layout', 'tabular'),
    '#options' => array(
      'orig'    => t('Original'),
      'ft'      => t('Only Fulltext if available'),
      'tabular' => t('Tabular')
    ),
    '#description' => t('This alters the layout of the "node" (full) view.')
  );
  $form['style']['biblio_annotations'] = array(
    '#type' => 'select',
    '#title' => t('Annotations'),
    '#default_value' => variable_get('biblio_annotations', 'none'),
    '#options' => array(
      'none'           => t('none'),
      'biblio_notes'   => t('notes'),
      'biblio_custom1' => t('custom1'),
      'biblio_custom2' => t('custom2'),
      'biblio_custom3' => t('custom3'),
      'biblio_custom4' => t('custom4'),
      'biblio_custom5' => t('custom5'),
      'biblio_custom6' => t('custom6'),
      'biblio_custom7' => t('custom7')
    ),
    '#description' => t('Select a field from which an annotation will be displayed below biblo entry in "short" listings'),
    '#multiple' => FALSE,
    '#size' => 0
  );
  $form['style']['biblio_style'] = array(
    '#type' => 'radios',
    '#title' => t('Style'),
    '#default_value' => variable_get('biblio_style', 'cse'),
    '#options' => _biblio_admin_get_styles(),
    '#description' => t('This alters the layout of the "list" (short) view.')
  );
  $form['syndication'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Syndication'),
    '#description' => t('You can set the RSS defaults here.')
  );
  $form['syndication']['biblio_rss'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow RSS feeds of new biblio entries'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_rss', 0),
    '#description' => t('This will create an rss feed of the 10 most recent biblio entries. It will be available at /biblio/rss.xml')
  );
  $form['syndication']['biblio_rss_number_of_entries'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of items in the RSS feed.'),
    '#default_value' => variable_get('biblio_rss_number_of_entries', 10),
    '#size' => 6,
    '#maxlength' => 6,
    '#description' => t('Limits the number of items in the /biblio/rss.xml feed to this number.')
  );
  $form['taxo'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Taxonomy'),
    '#description' => t('You can set the Taxonomy defaults here.')
  );
  $form['taxo']['biblio_keyword_freetagging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use keywords from biblio entries as taxonomy "free tags"'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_keyword_freetagging', 0),
    '#description' => t('This option allows user to add keywords (free tags) to describe their documents. These keywords will be registered as taxonomy.')
  );
  $vocabularies = module_invoke('taxonomy', 'get_vocabularies');
  // ... and print a form to select the terms in each of them
  $choice = new stdClass();
  $choice->option = array(0 => '<'. t('none') .'>');
  $taxo_options[0] = $choice;
  if (count($vocabularies)) {
    foreach ($vocabularies as $voc) {
      $choice = new stdClass();
      $choice->option = array(
        $voc->vid => $voc->name
      );
      $taxo_options[] = $choice;
    }
    $form['taxo']['biblio_freetagging_vocab'] = array(
      '#type' => 'select',
      '#title' => t('Vocabulary'),
      '#default_value' => variable_get('biblio_freetagging_vocab', 0),
      '#options' => $taxo_options,
      '#description' => t('Select vocabulary (category) to use for free tags.'),
      '#multiple' => FALSE,
      '#disabled' => (!variable_get('biblio_keyword_freetagging', 0)),
      '#size' => $multiple ? min(9, count($taxo_options)) : 0,
      '#weight' => 15
    );
  }
  $form = system_settings_form($form);
  $form['#submit'][] = 'biblio_admin_settings_form_submit';
  // our submit handler is added after the call to system settings form so that it gets
  // called after system_settings_form_submit, and thus the variables have been stored already
  // and the menu will be rebuilt correctly.
  return ($form);
}
function biblio_admin_settings_form_submit() {
  menu_rebuild();
}
function biblio_admin_types_edit_form() {
  $tid = 0;
  $arg_list = func_get_args();
  $tid = is_numeric($arg_list[1]) ? $arg_list[1] : 0;
  $msg = '<div>'. t('On this page you can set type specific "Titles" and "Hints" which will display on the input form. ');
  if ($tid) {
    $msg .= t('Checking the "Visible" box will add the field to the input form, checking "Required" will force the user to supply a value for this field and the weight value changes the order which it is rendered on the form with smaller values floating to the top of the form. ');
    $msg .= '<p>'. t('Fields which are grayed out on this page have been set to "common" on the ');
    $msg .= l("admin/settings/biblio/defaults", "admin/settings/biblio/defaults");
    $msg .= t(' page.');
  }
  else {
    $msg .= t('Checking the "Common" box will add the field to all the differnet publication types. Checking "Required" will force the user to supply a value for the field, checking "Autocomplete" will enable AJAX type auto complete look up for the field when the user is entering data and the weight value changes the order which it is rendered on the form with smaller values floating to the top of the form. ');
  }
  $msg .= '</div>';
  $form['help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Help'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['help']['message'] = array(
    '#type' => 'markup',
    '#value' => $msg
  );
  // first get all of the field info
  if ($tid) {
    $result = db_query("SELECT b.*, btd.*, btt.name AS type_name FROM {biblio_fields} AS b, {biblio_field_type} AS bt, {biblio_field_type_data} AS btd, {biblio_types} as btt
                        WHERE bt.fid=b.fid AND btd.ftdid=bt.ftdid AND btt.tid=bt.tid
                        AND bt.tid=$tid");
  }
  else {
    $result = db_query("SELECT b.*, btd.* AS type_name FROM {biblio_fields} AS b, {biblio_field_type} AS bt, {biblio_field_type_data} AS btd
                        WHERE bt.fid=b.fid AND btd.ftdid=bt.ftdid
                        AND bt.tid=$tid");
  }
  while ($row = db_fetch_array($result)) {
    $fields[$row['fid']] = $row;
  }
  $form['configured_flds'] = array('#tree' => 1);
  if ($tid) { // show an existing type
    $form['type_name'] = array(
      '#type' => 'value',
      '#title' => 'tid',
      '#value' => $fields[1]['type_name']
    );
    $form['type_id'] = array(
      '#type' => 'value',
      '#title' => 'tid',
      '#value' => $tid
    );
  }
  uasort($fields, "biblio_form_sort"); // resort the fields since the weight may have changed
  $vis_comm = $tid ? 'visible' : 'common';
  $options["$vis_comm"] = '';
  $options['required'] = '';
  if ($tid == 0)
    $options['autocomplete'] = '';
  foreach ($fields as $key => $fld) {
    $def_values[$fld['name']] = array();
    if ($tid) {
      if ($fld['visible'])
        array_push($def_values[$fld['name']], 'visible');
      if ($fld['required'])
        array_push($def_values[$fld['name']], 'required');
    }
    else {
      if ($fld['common'])
        array_push($def_values[$fld['name']], 'common');
      if ($fld['required'])
        array_push($def_values[$fld['name']], 'required');
      if ($fld['autocomplete'])
        array_push($def_values[$fld['name']], 'autocomplete');
    }
    $form['configured_flds'][$key]['name'] = array(
      '#type' => 'markup',
      '#value' => $fld['name'],
      '#weight' => $fld['weight']
    );
    $form['configured_flds'][$key]['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $fld['title'],
      '#size' => 15,
      '#weight' => $fld['weight'],
      '#disabled' => $tid ? $fld['common'] ? 1 : 0 : 0
    );
    $form['configured_flds'][$key]['hint'] = array(
      '#type' => 'textfield',
      '#default_value' => $fld['hint'],
      '#size' => 10,
      '#weight' => $fld['weight'],
      '#disabled' => $tid ? $fld['common'] ? 1 : 0 : 0
    );
    $form['configured_flds'][$key]['weight'] = array(
      '#type' => 'textfield',
      '#default_value' => $fld['weight'],
      '#size' => 2,
      '#weight' => $fld['weight'],
      '#disabled' => $tid ? $fld['common'] ? 1 : 0 : 0
    );
    $form['configured_flds'][$key]['checkboxes'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $def_values[$fld['name']],
      '#weight' => $fld['weight'],
      '#disabled' => $tid ? $fld['common'] ? 1 : 0 : 0
    );
    $form['configured_flds'][$key]['ftdid'] = array(
      '#type' => '#value',
      '#value' => $fld['ftdid']
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  return $form;
}
function biblio_admin_types_edit_form_submit($form, & $form_state) {
  foreach ($form_state['values']['configured_flds'] as $fid => $values) {
    $update = false;
    $val = array();
    $link = array();
    $link['tid'] = $form['type_id']['#value'];
    $link['fid'] = $fid;
    foreach ($values as $name => $value) {
      if (!$form['configured_flds'][$fid][$name]['#disabled']) {
        $default_value = $form['configured_flds'][$fid][$name]['#default_value'];
        if ($name == 'checkboxes') {
          $def = in_array('visible', $default_value);
          $val['visible'] = !empty($value['visible']);
          if (!$update)
            $update = ($def != $val['visible']) ? $form['configured_flds'][$fid]['ftdid']['#value'] : false;
          $def = in_array('required', $default_value);
          $val['required'] = !empty($value['required']);
          if (!$update)
            $update = ($def != $val['required']) ? $form['configured_flds'][$fid]['ftdid']['#value'] : false;
          $def = in_array('common', $default_value);
          $val['common'] = !empty($value['common']);
          if (!$update)
            $update = ($def != $val['common']) ? $form['configured_flds'][$fid]['ftdid']['#value'] : false;
          $def = in_array('autocomplete', $default_value);
          $val['autocomplete'] = !empty($value['autocomplete']);
          if (!$update)
            $update = ($def != $val['autocomplete']) ? $form['configured_flds'][$fid]['ftdid']['#value'] : false;
        }
        else {
          $val[$name] = $value;
          if (!$update)
            $update = ($default_value != $val[$name]) ? $form['configured_flds'][$fid]['ftdid']['#value'] : false;
        }
      }
    }
    if ($update == $fid && $link['tid']) { // we just changed a default value, so create a new entry in biblio_field_type_data
      $new_ftdid = variable_get('biblio_last_ftdid', 101);
      variable_set('biblio_last_ftdid', $new_ftdid +1);
      $val['ftdid'] = $new_ftdid;
      $link['ftdid'] = $new_ftdid;
      $link['cust_tdid'] = $new_ftdid;
      drupal_write_record('biblio_field_type_data', $val);
      drupal_write_record('biblio_field_type', $link, array('tid', 'fid'));
    }
    else
      if ($update > 100 && $link['tid']) { // we are updating an existing entry
        $val['ftdid'] = $form['configured_flds'][$fid]['ftdid']['#value'];
        drupal_write_record('biblio_field_type_data', $val, 'ftdid');
      }
      else
        if ($update == $fid) { // changing the defaults
          $val['fid'] = $fid;
          drupal_write_record('biblio_fields', $val, 'fid');
          if ($val['common']) {
            $query = "UPDATE {biblio_field_type} SET ftdid = %d WHERE fid = %d";
            db_query($query, array($fid, $fid));
          }
          else { // not common, so change pointer back to customizations if available
            $query = "UPDATE {biblio_field_type} SET ftdid = cust_tdid WHERE fid = %d";
            db_query($query, array($fid));
          }
        }
  }
  drupal_set_message(t("The changes have been saved."));
  // Clear the cached pages and menus:
  menu_rebuild();
  drupal_goto('admin/settings/biblio/types');
}
function biblio_admin_types_form() {
  $result = db_query('SELECT t.* FROM {biblio_types as t WHERE t.tid > 0}');
  //$rows[] = array('',t('Fields Common To All Types'),l('edit', 'admin/settings/biblio/types/edit'),'');
  while ($row = db_fetch_object($result)) {
    if ($row->tid < 999) {
      $rows[] = array(
        $row->tid,
        $row->name,
        ($row->visible) ? l(t('edit'), 'admin/settings/biblio/types/edit/'. $row->tid) : '', ($row->visible) ? l(t('hide'), 'admin/settings/biblio/types/hide/'. $row->tid) : l(t('show'), 'admin/settings/biblio/types/show/'. $row->tid)
      );
    }
    else {
      $rows[] = array(
        $row->tid,
        $row->name,
        l(t('edit'), 'admin/settings/biblio/types/edit/'. $row->tid), l(t('delete'), 'admin/settings/biblio/types/delete/'. $row->tid)
      );
    }
  }
  $header = array(
    t('Type Id'),
    t('Type Name'), array(
      'data' => t('Operations'),
      'colspan' => '2'
    )
  );
  $output = theme('table', $header, $rows);
  $output .= '<p>[ '. l(t('Add New Type'), 'admin/settings/biblio/types/add') .' ]';
  $output .= ' [ '. l(t('Reset all types to defaults'), 'admin/settings/biblio/types/reset') .' ]';
  return $output;
}
function biblio_admin_types_add_form() {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Type Name'),
    '#size' => 20,
    '#weight' => 1,
    '#required' => true,
    '#maxlength' => 64
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#size' => 60,
    '#weight' => 2,
    '#maxlength' => 255
  );
  $form['type_button'] = array(
    '#type' => 'submit',
    '#value' => t('Create New Type'),
    '#weight' => 3
  );
  return $form;
}
function biblio_admin_types_add_form_submit($form, & $form_state) {
  $values = $form_state['values'];
  $values['tid'] = variable_get('biblio_max_user_tid', '999') + 1;
  drupal_write_record('biblio_types', $values);
  variable_set('biblio_max_user_tid', $values['tid']);
  for ($f = 1; $f <= 51; $f++) {
    $ft_vals = array(
      'tid' => $values['tid'],
      'fid' => $f,
      'ftdid' => $f,
      'cust_tdid' => $f
    );
    drupal_write_record('biblio_field_type', $ft_vals);
  }
  drupal_goto('admin/settings/biblio/types');
}
function biblio_admin_types_hide() {
  $args = func_get_args();
  if ($args[0] > 0 && is_numeric($args[0])) {
    $result = db_query('UPDATE {biblio_types} SET visible = 0 WHERE tid = %d ', $args[0]);
  }
  drupal_goto('admin/settings/biblio/types');
}
function biblio_admin_types_show() {
  $args = func_get_args();
  if ($args[0] > 0 && is_numeric($args[0])) {
    $result = db_query('UPDATE {biblio_types} SET visible = 1 WHERE tid = %d ', $args[0]);
  }
  drupal_goto('admin/settings/biblio/types');
}
function biblio_admin_types_delete_form() {
  $args = func_get_args();
  if ($args[1] > 0 && is_numeric($args[1])) {
    $result = db_query('SELECT t.* FROM {biblio_types} as t WHERE t.tid = %d ', $args[1]);
    $row = db_fetch_array($result);
    $num_rows = db_result(db_query('SELECT COUNT(*) FROM {biblio} as b WHERE b.biblio_type = %d', $row['tid']));
    if ($num_rows) {
      $existing_msg = t('There are @count biblio entries of this type, you should change the type of these entries before proceeding otherwise they will be deleted', array(
        '@count' => $num_rows
      ));
    }
    $form['tid'] = array(
      '#type' => 'value',
      '#value' => $row['tid']
    );
    $output = confirm_form($form, t('Are you sure you want to delete the custom biblio type:  %title ? ', array(
      '%title' => $row['name']
    )) . $existing_msg, $_GET['destination'] ? $_GET['destination'] : 'admin/settings/biblio/types', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
    return $output;
  }
  else {
    drupal_goto('admin/settings/biblio/types');
  }
}
function biblio_admin_types_delete_form_submit($form, & $form_state) {
  db_query('DELETE FROM {biblio_types} WHERE tid = %d', $form_state['values']['tid']);
  db_query('DELETE FROM {biblio_field_type} WHERE tid = %d', $form_state['values']['tid']);
  drupal_goto('admin/settings/biblio/types');
}
function biblio_admin_types_reset_form() {
  $form['reset'] = array(
    '#type' => 'value',
    '#value' => 'reset'
  );
  $output = confirm_form($form, t('Are you sure you want to reset ALL the field definitions to the defaults? '), $_GET['destination'] ? $_GET['destination'] : 'admin/settings/biblio/types', t('This action will reset any and all field customizations to the defaults, and cannot be undone!'), t('Reset'), t('Cancel'));
  return $output;
}
function biblio_admin_types_reset_form_submit($form, & $form_state) {
  require_once (drupal_get_path('module', 'biblio') ."/biblio.install");
  biblio_reset_types();
  drupal_goto("admin/settings/biblio/types");
}
/*
 * This functin is used by both the admin/settings/biblio page and user profile page
 *   - if $user is set, then it is being called from the user profile page
 */
function _biblio_get_user_profile_form(& $form, $user = false) {
  $form['profile'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => ($user) ? FALSE : TRUE,
    '#title' => ($user) ? t('My publications') : t('Profile pages')
  );
  if(!$user){
    $form['profile']['biblio_show_user_profile_form'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to override these settings on their "My account" page') ,
      '#return_value' => 1,
      '#description' =>  t('If this is selected, a form similar to this section will be available to the user when they edit their own account information.  This will allow them to override the global preferences set here.') ,
      '#default_value' => variable_get('biblio_show_user_profile_form', '1')
    );
  }
  $form['profile']['biblio_show_profile'] = array(
    '#type' => 'checkbox',
    '#title' => ($user) ? t('Show my publications on my profile page') : t('Show publications on users profile pages'),
    '#return_value' => 1,
    '#description' => ($user) ? t('Selecting this will create a listing of your publications on your profile page') : t('This sets the site wide default, users may change this in their profile')
  );
  if ($user) {
    $form['profile']['biblio_show_profile']['#default_value'] = (isset($user->biblio_show_profile)) ? $user->biblio_show_profile : variable_get('biblio_show_profile', '0');
  }
  else {
    $form['profile']['biblio_show_profile']['#default_value'] = variable_get('biblio_show_profile', '0');
  }
  $form['profile']['biblio_my_pubs_menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show "My publications" item in the navigation menu'),
    '#return_value' => 1,
    '#description' => ''
  );
  if ($user) {
    $form['profile']['biblio_my_pubs_menu']['#default_value'] = ((isset($user->biblio_my_pubs_menu)) ? $user->biblio_my_pubs_menu : variable_get('biblio_my_pubs_menu', '0'));
  }
  else {
    $form['profile']['biblio_my_pubs_menu']['#default_value'] = variable_get('biblio_my_pubs_menu', '0');
  }
  if ($user) {
    $form['profile']['biblio_lastname'] = array(
      '#type' => 'textfield',
      '#title' => t('My last name'),
      '#default_value' => $user->biblio_lastname,
      '#description' => t('This may be ueed to determine which publications are mine')
    );
  }
  $form['profile']['biblio_profile_uid'] = array(
    '#type' => 'radios',
    '#title' => t('Select publications by'),
    '#default_value' => (isset($user->biblio_profile_uid)) ? $user->biblio_profile_uid : variable_get('biblio_profile_uid', 'lastname'),
    '#options' => array(
      'uid' => t('User ID'),
      'lastname' => ($user) ? t('My last name') : t('Last name'),
      'uid_lastname' => t('Both (Last name & User ID)')
    ),
    '#description' => t('This defines the criteria used to select which publications will be listed on the users profile page')
  );
}
/*  This function parses the module directory for 'style' files, loads them and
 *  calls the info fuction to get some basic information like the short and long
 *  names of the style
*/
function _biblio_admin_get_styles() {
  $styles = array();
  $dir = drupal_get_path('module', 'biblio');
  $files = array();
  if ($dh = opendir($dir)) {
    while (($files[] = readdir($dh)) !== false) {}
    closedir($dh);
    foreach ($files as $file) {
      if (strstr($file, 'biblio_style_')) { //find the style files
        include_once $dir ."/". $file;
        if (function_exists(basename($file, ".inc") .'_info')) {
          $styles = array_merge($styles, call_user_func(basename($file, ".inc") .'_info')); //build and array of the short and long names
        }
      }
    }
  }
  return ($styles);
}
function biblio_form_sort($a, $b) {
  $a_weight = (is_array($a) && isset($a['weight'])) ? $a['weight'] : 0;
  $b_weight = (is_array($b) && isset($b['weight'])) ? $b['weight'] : 0;
  if ($a_weight == $b_weight) {
    return 0;
  }
  return ($a_weight < $b_weight) ? -1 : 1;
}
function biblio_admin_get_query($name) {
  switch ($name) {
    case "author_dup" :
      return ('SELECT lastname, COUNT(*) AS cnt FROM {biblio_contributors_data} GROUP BY lastname HAVING cnt>1');
      break;
    case "author_by_name" :
      return ('SELECT lastname, cid FROM {biblio_contributors_data} where lastname LIKE "%s"');
      break;
  }
}