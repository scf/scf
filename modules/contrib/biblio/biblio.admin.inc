<?php
// $Id: biblio.admin.inc,v 1.9.2.80 2009/09/30 15:48:25 rjerome Exp $
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
 * Default page callback for batches.
 */

function biblio_admin_ahah($form, $element, $value) {
  switch ($form) {

  }
}
/**
* Implementation of hook_settings().
*/
function biblio_admin_settings() {
  $version = '$Revision: 1.9.2.80 $ $Date: 2009/09/30 15:48:25 $';
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
  $form['biblio_base_title'] = array(
   '#type' => 'textfield',
    '#title' => t('Biblio page title'),
    '#size' => 20,
    '#default_value' => variable_get('biblio_base_title', 'Biblio'),
    '#description' => t('The page title shown on the base URL.')
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
  $form['authors'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Authors'),
    '#description' => ''
  );
  $form['authors']['biblio_auto_orphaned_author_delete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically delete orphaned authors'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_auto_orphaned_author_delete', 0),
    '#description' => t('Orphaned authors are those which are no longer linked to any entries as the result of a biblio update or delete.')
  );
  $form['bibtex'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('BibTex settings'),
    '#description' => ''
  );
  $form['bibtex']['biblio_hide_bibtex_braces'] = array(
    '#type' => 'checkbox',
    '#title' => t('Retain bibtex\'s {Protected} capitalization in the title string, but hide the braces on display'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_hide_bibtex_braces', 0),
    '#description' =>''
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
  $result = db_query("SELECT b.name, bftd.title FROM {biblio_fields} b
                      INNER JOIN {biblio_field_type} bt ON bt.fid=b.fid
                      INNER JOIN {biblio_field_type_data} bftd ON bftd.ftdid=bt.ftdid
                      WHERE bt.tid=0 ORDER by bftd.title ASC ");
  $schema = drupal_get_schema('biblio');
  $keys = array_keys($schema['fields']);
  $options = array();
  $options['nid'] =  t('Node ID');
  while ($row = db_fetch_array($result)) {
    $options[$row['name']] = $row['title'];
  }
  $form['citekey'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Citekey'),
    '#description' => t('You can alter citekey related settings here.')
  );
  $form['citekey']['biblio_display_citation_key'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show citation key in results'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_display_citation_key', 0),
    '#description' => t('This will output the citekey as the first item in the citation string')
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
  // Allow only users to modify PHP code which have PHP block visibility permissions
  if (user_access('use PHP for block visibility')) {
    $form['citekey']['biblio_citekey_phpcode'] = array(
     	'#type' => 'textarea',
   		'#title' => t('PHP code for citekey generation'),
    	'#default_value' => variable_get('biblio_citekey_phpcode', ''),
    	'#description' => t('Advanced usage only: PHP code that returns the citekey. Should not include &lt;?php ?&gt; delimiters.')
    );
  }

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
  $form['isi_wok'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('ISI Web of Knowledge'),
    '#description' => ''
  );
  $form['isi_wok']['biblio_fix_isi_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically replace "Go to ISI" links with the  URL below'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_fix_isi_links', 0),
    '#description' => t('This option automatically replaces any fake "Go to ISI" links with the supplied URL to ISI Web of Knowledge.  Note a subscription with ISI is required for these links to function.')
  );
  $form['isi_wok']['biblio_isi_url'] = array(
    '#type' => 'textfield',
    '#title' => t('ISI Web of Knowledge URL'),
    '#size' => 128,
    '#maxlength' => 512,
    '#default_value' => variable_get('biblio_isi_url', 'http://apps.isiknowledge.com/InboundService.do?Func=Frame&product=WOS&action=retrieve&SrcApp=EndNote&Init=Yes&SrcAuth=ResearchSoft&mode=FullRecord&UT='),
    '#description' => t('Enter the URL which will replace the "Go to ISI" fake links imported from EndNote')
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
  $form['keywords']['biblio_keyword_orphan_autoclean'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically remove orphaned keywords'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_keyword_orphan_autoclean', 1),
    '#description' => t('This option automatically deletes keywords which are no longer associated with any publications (primarily due to the due to the removal of a publication or editing a keyword).')
  );
  $taxo_mesg = '<div class="admin-dependencies">'. t('Depends on') .': '. t('Taxonomy') .' (<span class="admin-disabled">'. t('disabled') .'</span>)</div>';
  $form['keywords']['biblio_copy_taxo_terms_to_keywords'] = array(
    '#type' => 'checkbox',
    '#title' => t('Copy any selected taxonomy terms to the biblio keyword database'),
    '#return_value' => 1,
    '#default_value' =>  module_exists('taxonomy') ? variable_get('biblio_copy_taxo_terms_to_keywords', 0) : 0,
    '#disabled' => (!module_exists('taxonomy')),
    '#description' => module_exists('taxonomy') ? t('If this option is selected, the selected taxonomy terms will be copied to the '.variable_get('biblio_base_title', 'Biblio').' keyword database and be displayed as keywords (as well as taxonomy terms) for this entry.') : $taxo_mesg
   );
  $form['keywords']['biblio_keyword_freetagging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use keywords from biblio entries as taxonomy "free tags"'),
    '#return_value' => 1,
    '#default_value' => module_exists('taxonomy') ? variable_get('biblio_keyword_freetagging', 0) : 0,
    '#disabled' => (!module_exists('taxonomy')),
    '#description' => module_exists('taxonomy') ? t('This option allows user to add keywords (free tags) to describe their documents. These keywords will be registered as taxonomy.') : $taxo_mesg
  );
  $vocabularies = module_invoke('taxonomy', 'get_vocabularies');
  // ... and print a form to select the terms in each of them
  $taxo_options = array();
  $taxo_options[0]= '<'. t('none') .'>';
  if (count($vocabularies)) {
    foreach ($vocabularies as $voc) {
      $taxo_options[$voc->vid] =  $voc->name;
    }
    $form['keywords']['biblio_keyword_vocabulary'] = array(
      '#type' => 'select',
      '#title' => t('Vocabulary'),
      '#default_value' => variable_get('biblio_keyword_vocabulary', 0),
      '#options' => $taxo_options,
      '#description' => t('Select vocabulary (category) to use for free tags.'),
      '#multiple' => FALSE,
  //    '#disabled' => (!variable_get('biblio_keyword_freetagging', 0)),
  //    '#size' => $multiple ? min(9, count($taxo_options)) : 0,
      '#weight' => 15
    );
  }

  $form['links'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Links')
  );
  $form['links']['export'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Export Links')
  );
  $form['links']['export']['biblio_export_links'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Show export links'),
    '#default_value' => variable_get('biblio_export_links', array('tagged', 'xml', 'bibtex')),
    '#options' => array(
      'tagged'  => t('EndNote Tagged'),
      'xml'     => t('EndNote XML'),
      'bibtex'  => t('BibTex'),
    ),
    '#description' => t('You can select which export links to display here.')
  );
  $form['links']['google'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Google Scholar Link')
  );
  $form['links']['google']['biblio_google_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show Google Scholar links'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_google_link', 1),
    '#description' => t('Selecting this option will display a link to Google Scholar beside each entry.  Clicking this link searches Google Scholar using the title and first author of the current biblio entry.')
  );

  $form['links']['biblio_links_target_new_window'] = array(
    '#type' => 'checkbox',
    '#title' => t('Links open in new browser'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_links_target_new_window', 0),
    '#description' => t('This causes related URLs to open in a new browser window')
  );
  $form['links']['biblio_inlinemode_in_links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Carry "inline" mode through to all links'),
      '#return_value' => 1,
      '#default_value' => variable_get('biblio_inlinemode_in_links', 0),
      '#description' => t('This causes the "inline" mode to be applied to all links such as titles, authors and keywords')
  );

  $form['links']['biblio_link_title_url'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hyperlink titles using supplied URL if available'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_link_title_url', 0),
    '#description' => t('Selecting this links the titles to the supplied URL (if available) rather than the "node" view.')
  );
  $form['links']['biblio_author_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hyperlink author names'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_author_links', 1),
    '#description' => t('This creates a hyperlink on author names which, when clicked, will select entries which contain that author')
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
  $sid = "Biblio:". variable_get('site_name', 'Drupal');
  $form['openurl']['biblio_openurl_sid'] = array(
    '#type' => 'textfield',
    '#title' => t('OpenURL Site ID'),
    '#size' => 95,
    '#default_value' => variable_get('biblio_openurl_sid', $sid),
    '#description' => t('This sets your institution\'s site name, some link resolvers will require a specific Site ID in order to process your requests.')
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
  if (!module_exists('search')) {
    $search_text = '<div class="admin-dependencies">'. t('Depends on') .': '. t('Search') .' (<span class="admin-disabled">'. t('disabled') .'</span>)</div>';
    $search_disabled = TRUE;
    variable_set('biblio_footnotes_integration', 0);
  }
  else {
    $search_text = '<div class="admin-dependencies">'. t('Depends on') .': '. t('Search') .' (<span class="admin-enabled">'. t('enabled') .'</span>)</div>';
    $search_disabled = FALSE;
  }
  $form['search'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => $search_text,
    '#title' => t('Search'),
  );
  $form['search']['biblio_search'] = array(
    '#type' => 'checkbox',
    '#disabled' => $search_disabled,
    '#title' => t('Enable a search box on the biblio page.'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_search', 0),
    '#description' => t('Shows a search box on the biblio page that returns drupal search results in the biblio style.')
  );
  $form['search']['biblio_search_button_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Search button text'),
    '#disabled' => $search_disabled,
    '#size' => 95,
    '#default_value' => variable_get('biblio_search_button_text', t('Biblio search')),
    '#description' => t('This allows you to customize the text on the search button, it defaults to "Biblio search".')
  );
  $form['search']['biblio_index'] = array(
    '#type' => 'checkbox',
    '#disabled' => $search_disabled,
    '#title' => t('Re-/Index a biblio node when creating or updating.'),
    '#return_value' => 1,
    '#default_value' => variable_get('biblio_index', 0),
    '#description' => t('A biblio node must be indexed for the drupal search to know its content. You need to check this option if you want to search for a biblio node that you just created or updated. Otherwise you must wait for the cron job to reindex nodes.')
  );
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
  $form['sort']['biblio_sort_tabs_style'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show sort links as "tabs"'),
    '#default_value' => variable_get('biblio_sort_tabs_style', 0),
    '#return_value' => 1,
    '#description' => t('This changes the sort links from text links to tabs')
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
  $form['style']['biblio_no_year_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Text to display if no year of publication is available'),
    '#size' => 95,
    '#default_value' => variable_get('biblio_no_year_text', t('Submitted')),
    '#description' => t('The text that is displayed when no date of publication is given or it is deliberately set to <b>9999</b>, it defaults to "Submitted".')
  );
  $form['style']['biblio_inpress_year_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Text to display if year of publication is set to 9998'),
    '#size' => 95,
    '#default_value' => variable_get('biblio_inpress_year_text', t('In Press')),
    '#description' => t('The text that is displayed when the date of publication is deliberately set to <b>9998</b>, it defaults to "In Press".')
  );

  $form['style']['biblio_style'] = array(
    '#type' => 'radios',
    '#title' => t('Style'),
    '#default_value' => variable_get('biblio_style', 'cse'),
    '#options' => _biblio_admin_get_styles(),
    '#description' => t('Set the bibliographic style of the "list" view.')
  );

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

  $form = system_settings_form($form);
  $form['#submit'][] = 'biblio_admin_settings_form_submit';
  // our submit handler is added after the call to system settings form so that it gets
  // called after system_settings_form_submit, and thus the variables have been stored already
  // and the menu will be rebuilt correctly.
  return ($form);
}
function biblio_admin_settings_form_submit($form, $form_state) {
  if ($form_state['values']['biblio_keyword_freetagging'] && $form_state['values']['biblio_keyword_vocabulary']) {
    if ($vocabulary = taxonomy_vocabulary_load(variable_get('biblio_keyword_vocabulary', 0))) {
      $vocabulary = (array) $vocabulary;
      $vocabulary['nodes']['biblio'] = 1;
      taxonomy_save_vocabulary($vocabulary);
    }
  }

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
    $msg .= l("admin/settings/biblio/fields", "admin/settings/biblio/fields");
    $msg .= t(' page.').'</p>';
  }
  else {
    $msg .= t('Checking the "Common" box will add the field to all the different publication types. Checking "Required" will force the user to supply a value for the field, checking "Autocomplete" will enable AJAX type auto complete look up for the field when the user is entering data and the weight value changes the order which it is rendered on the form with smaller values floating to the top of the form. ');
  }
  $msg .= t('Finally, for each author field you can choose a set of author roles. Assigning different roles to authors within the same field, e.g. primary and secondary authors within the authors field, allows to theme them differently.');
  $msg .= '</div>';
  $form['#redirect'] = 'admin/settings/biblio/type';
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
  // get author types
  $result = db_query("SELECT * FROM {biblio_contributor_type_data} b");
  $contrib_options = array();
  while($contrib_type = db_fetch_object($result)) {
      $contrib_options[$contrib_type->auth_type] = $contrib_type->title;
      $contrib_hints[$contrib_type->auth_type] = $contrib_type->hint;
  }

  // first get all of the field info
  if ($tid) {
    $result = db_query("SELECT bf.*, bftd.*, bft.common, bft.visible, bft.autocomplete, bft.weight, bft.required, bt.name AS type_name
                        FROM {biblio_fields} AS bf
                        INNER JOIN {biblio_field_type} AS bft ON bft.fid=bf.fid
                        INNER JOIN {biblio_field_type_data} AS bftd ON bftd.ftdid=bft.ftdid
                        INNER JOIN {biblio_types} AS bt ON bt.tid=bft.tid
                        WHERE bft.tid=$tid");
  }
  else {
    $result = db_query("SELECT bf.*, bftd.*, bft.common, bft.visible, bft.autocomplete, bft.weight, bft.required
                        FROM {biblio_fields} AS bf
                        INNER JOIN {biblio_field_type} AS bft ON bft.fid=bf.fid
                        INNER JOIN {biblio_field_type_data} AS bftd ON bftd.ftdid=bft.ftdid
                        WHERE bft.tid=$tid");
  }
  while ($row = db_fetch_array($result)) {
    $fields[$row['fid']] = $row;
  }
  $form['configured_flds'] = array('#tree' => 1);
  if ($tid) { // show an existing type
    $typename = $fields[1]['type_name'];
    $form['type_name'] = array(
      '#type' => 'value',
      '#title' => 'tid',
      '#value' => $typename

    );
    $form['top_message'] = array(
      '#value' => t('Field settings related to @type publications',array('@type' => $typename))
    );
    $form['type_id'] = array(
      '#type' => 'value',
      '#title' => 'tid',
      '#value' => $tid
    );
  } else {
    $form['top_message'] = array(
      '#value' => t('Field settings common to all publication types')
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
      if ($fld['visible'] )
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
    $disabled = $tid ? ($fld['common'] ? 1 : 0) : 0;
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
      '#disabled' => $disabled
    );
    $form['configured_flds'][$key]['hint'] = array(
      '#type' => 'textfield',
      '#default_value' => $fld['hint'],
      '#size' => 20,
      '#weight' => $fld['weight'],
      '#disabled' => $disabled
    );
    $form['configured_flds'][$key]['weight'] = array(
      '#type' => 'textfield',
      '#default_value' => $fld['weight'],
      '#size' => 2,
      '#weight' => $fld['weight'],
      //'#disabled' => $disabled
    );
    $form['configured_flds'][$key]['checkboxes'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $def_values[$fld['name']],
      '#weight' => $fld['weight'],
      //'#disabled' => $disabled
    );
    $form['configured_flds'][$key]['ftdid'] = array(
      '#type' => '#value',
      '#value' => $fld['ftdid']
    );
    if ($fld['type'] == 'contrib_widget') {
      $cur = _biblio_get_auth_types($fld['fid'], $tid ? $tid : $fld['fid']);
      $form['configured_flds'][$key]['auth_type'] = array(
        '#type' => 'select',
  			'#options' => $contrib_options,
        '#size' => min(count($contrib_options), 4),
        '#multiple' => TRUE,
      	'#default_value' => _biblio_get_auth_types($fld['fid'], $disabled ? 0 : $tid),
      	'#disabled' => $disabled
      );
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  return $form;
}

function biblio_admin_types_edit_form_submit($form, & $form_state) {
  $tid = $form_state['values']['type_id'];
  if (empty($tid)) $tid = 0;
  // save author types
  $link['biblio_type'] = $tid;
  db_query('DELETE FROM {biblio_contributor_type} WHERE biblio_type=%d', $link['biblio_type']);
  for($i=1; $i<=5; $i++) {
    $link['auth_category'] = $i;
    $auth_types = (array)$form_state['values']['configured_flds'][$i]['auth_type'];
    // if default values have not changed, do not store to the database explicitly
    if ($link['biblio_type'] &&
        (array_values($auth_types) == array_values((array)$form['configured_flds'][$i]['auth_type']['#default_value']))) continue;
    foreach ($auth_types as $type) {
      $link['auth_type'] = $type;
      drupal_write_record('biblio_contributor_type', $link);
    }
  }
  // save other field data
  foreach ($form_state['values']['configured_flds'] as $fid => $values) {
    $update = FALSE;
    $val = array();
    $link = array();
    $link['fid'] = $fid;
    $link['tid'] = $tid;
    foreach ($values as $name => $value) {
      if (!$form['configured_flds'][$fid][$name]['#disabled']) {
        $default_value = $form['configured_flds'][$fid][$name]['#default_value'];
        if ($name == 'checkboxes') {
          $link['visible']      = (!empty($value['visible'])) ? 1 : 0;
          $link['required']     = (!empty($value['required'])) ? 1 : 0;
          if ($tid == 0) {
            $link['common']       = (!empty($value['common'])) ? 1 : 0;
            $link['autocomplete'] = (!empty($value['autocomplete'])) ? 1 : 0;
          }
        }
        elseif ($name == weight) {
          $link['weight'] = $value;
        } else {
          $val[$name] = $value;
          if (!$update)
          $update = ($default_value != $val[$name]) ? $form['configured_flds'][$fid]['ftdid']['#value'] : FALSE;
        }
      }
    }
    if ($update == $fid && $link['tid']) { // we just changed a default value, so create a new entry in biblio_field_type_data
      $new_ftdid = variable_get('biblio_last_ftdid', 100);
      variable_set('biblio_last_ftdid', $new_ftdid + 1);
      $val['ftdid'] = $new_ftdid;
      $link['ftdid'] = $new_ftdid;
      $link['cust_tdid'] = $new_ftdid;
      drupal_write_record('biblio_field_type_data', $val);

    } elseif ($update >= 100 && $link['tid']) { // we are updating an existing entry
      $val['ftdid'] = $form['configured_flds'][$fid]['ftdid']['#value'];
      drupal_write_record('biblio_field_type_data', $val, 'ftdid');

    } elseif ($update == $fid) { // changing the defaults
      $val['ftdid'] = $fid;
      drupal_write_record('biblio_field_type_data', $val, 'ftdid');
    }

    drupal_write_record('biblio_field_type', $link, array('fid','tid'));

    if($tid == 0) {
      if ($link['common']) {
        $query = "UPDATE {biblio_field_type} SET ftdid = %d, common = %d, visible = %d WHERE fid = %d";
        db_query($query, array($fid, 1, 1, $fid));
      }
      else { // not common, so change pointer back to customizations if available
        $query = "UPDATE {biblio_field_type} SET ftdid = cust_tdid, common = %d WHERE fid = %d";
        db_query($query, array(0, $fid));
      }
      // set the autocomplete bit on this field for all the types
      $query = "UPDATE {biblio_field_type} SET autocomplete = %d WHERE fid = %d";
      db_query($query, array($link['autocomplete'], $fid));
    }
  }
  drupal_set_message(t("The changes have been saved."));

  // Clear the cached pages and menus:
  menu_rebuild();
}

function biblio_admin_type_mapper_page() {
  $output .= '<h3>'.t('Choose a file format to modify').'</h3><p />';
  $output .= l('BibTex', 'admin/settings/biblio/fields/typemap/bibtex').'<br>';
  $output .= l('RIS', 'admin/settings/biblio/fields/typemap/ris').'<br>';
  $output .= l('EndNote Tagged', 'admin/settings/biblio/fields/typemap/tagged').'<br>';
  $output .= l('EndNote 7 XML', 'admin/settings/biblio/fields/typemap/endnote7').'<br>';
  $output .= l('EndNote X3 XML', 'admin/settings/biblio/fields/typemap/endnote8').'<br>';;
  $output .= l('CrossRef XML', 'admin/settings/biblio/fields/typemap/unixref').'<br>';
  return $output;
}

function biblio_admin_type_mapper_form($form_state, $format = 'bibtex', $op = 'map') {
  module_load_include('inc', 'biblio', 'biblio.type.mapper');
  $names = biblio_get_type_names($format);
  $map = biblio_get_type_map($format);
  ksort($names);
  $form['fileformat'] = array(
    '#type' => 'hidden',
    '#value' => $format,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to Defaults'),
  );
  $form['#redirect'] = 'admin/settings/biblio/fields/typemap';

  if ($op == 'map') {
    $result = db_query('SELECT t.* FROM {biblio_types} as t WHERE t.tid > 0');
    while ($type = db_fetch_object($result)) {
      $biblio_type_options[$type->tid] =  $type->name;
    }
    asort($biblio_type_options);
    $biblio_type_select = array(
    '#type' => 'select',
    '#options' => $biblio_type_options,
    );
    foreach ($names as $key => $value) {
      $biblio_type_select['#default_value'] = $map[$key];
      $form['type'][$key] = array(
      'format' => array('#value' => "<b>$key</b> (<i>$value</i>)"),
      'biblio' => $biblio_type_select,
      '#tree' => 1,
      );

    }
  }
  elseif($op == 'add') {
    $form['type_name'] = array(
      '#type' => 'textfield',
      '#title' => 'Publication Type',
      '#required' => TRUE,
      '#description' => t('This is the name of the type identifier, exactly as it appears in the file'),
    );
    $form['type_desc'] = array(
      '#type' => 'textfield',
      '#title' => 'Description'
    );
    $form['submit']['#value'] = t('Add');
    unset($form['reset']);
    $form['#redirect'] = 'admin/settings/biblio/fields/typemap/'.$format;
  }

  return $form;
}

function theme_biblio_admin_type_mapper_form($form) {
  switch ($form['fileformat']['#value']) {
    case 'bibtex':
      $title = 'BibTex';
      break;
    case 'ris':
      $title = 'RIS';
      break;
    case 'tagged':
      $title = 'EndNote Tagged';
      break;
    case 'endnote7':
      $title = 'EndNote 7 and earlier XML';
      break;
    case 'endnote8':
      $title = 'EndNote 8 and later XML';
      break;
  }
  if ($form['submit']['#value'] == t('Save')) {
  drupal_set_title($title.' '.t('Type Mapping'));
  foreach(element_children($form['type']) as $key ){
      $rows[] = array(
        drupal_render($form['type'][$key]['format']),
        drupal_render($form['type'][$key])
      );
  }
  $header = array(
    $title .' '.t('Publication Types'),
    t('Biblio Publication Type')
  );
  $output .= theme('table', $header, $rows);
  $output .= l('['.t('Add New'). " $title ".t('Publication Type').']','admin/settings/biblio/fields/typemap/'.$form['fileformat']['#value'].'/add').' ';
  $output .= l('['.t('Add New Biblio Publication Type').']','admin/settings/biblio/fields/type/new').'<br />';
  }
  elseif($form['submit']['#value'] == t('Add')) {
   drupal_set_title(t('Add new publication type to '). $title.' '.t('file type'));
   $output .= drupal_render($form['type_name']);
   $output .= drupal_render($form['type_desc']);
  }
  $output .= drupal_render($form['submit']);
  $output .= drupal_render($form['reset']);
  $output .= drupal_render($form);

  return $output;
}

function biblio_admin_type_mapper_form_submit($form, $form_state) {
  if ($form_state['values']['op'] == t('Save')) {
    foreach($form_state['values'] as $key => $value) {
      if (is_array($value)) {
        $map[$key]=$value['biblio'];
      }
    }
    variable_set('biblio_'.$form_state['values']['fileformat'] .'_type_map', $map);
  }
  elseif ($form_state['values']['op'] == t('Reset to Defaults')) {
    variable_del('biblio_'. $form_state['values']['fileformat'] .'_type_map');
    variable_del('biblio_'. $form_state['values']['fileformat'] .'_type_names');
  }
  elseif ($form_state['values']['op'] == t('Add')) {
    $names = biblio_get_type_names($form_state['values']['fileformat']);
    $names[$form_state['values']['type_name']] = $form_state['values']['type_desc'];
    variable_set('biblio_'.$form_state['values']['fileformat'] .'_type_names', $names);
  }
  //drupal_goto('admin/settings/biblio/fields/typemap');
}

function biblio_admin_types_form() {
  $result = db_query('SELECT t.* FROM {biblio_types} as t WHERE t.tid > 0');
  //$rows[] = array('',t('Fields Common To All Types'),l('edit', 'admin/settings/biblio/types/edit'),'');
  while ($row = db_fetch_object($result)) {
    if ($row->tid < 999) {
      $rows[] = array(
        $row->tid,
        $row->name,
        ($row->visible) ? l(t('edit'), 'admin/settings/biblio/fields/type/edit/'. $row->tid) : '', ($row->visible) ? l(t('hide'), 'admin/settings/biblio/fields/type/hide/'. $row->tid) : l(t('show'), 'admin/settings/biblio/fields/type/show/'. $row->tid)
      );
    }
    else {
      $rows[] = array(
        $row->tid,
        $row->name,
        l(t('edit'), 'admin/settings/biblio/fields/type/edit/'. $row->tid), l(t('delete'), 'admin/settings/biblio/fields/type/delete/'. $row->tid)
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
  $output .= '<p>[ '. l(t('Add New Type'), 'admin/settings/biblio/fields/type/new') .' ]';
  $output .= ' [ '. l(t('Reset all types to defaults'), 'admin/settings/biblio/fields/type/reset') .' ]';
  $output .= theme('table', $header, $rows);
  $output .= '<p>[ '. l(t('Add New Type'), 'admin/settings/biblio/fields/type/new') .' ]';
  $output .= ' [ '. l(t('Reset all types to defaults'), 'admin/settings/biblio/fields/type/reset') .' ]';
  return $output;
}
function biblio_admin_types_add_form() {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Type Name'),
    '#size' => 20,
    '#weight' => 1,
    '#required' => TRUE,
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
      'tid'       => $values['tid'],
      'fid'       => $f,
      'ftdid'     => $f,
      'cust_tdid' => $f,
      'weight'    => $f,
      'visible'    => 1,
    );
    drupal_write_record('biblio_field_type', $ft_vals);
  }
  // Fill contributor types. Use the first 4 defaults.
  for ($type = 1; $type <= 4; $type++) {
    $ct_vals = array (
      'auth_category' => $type,
      'biblio_type' => $values['tid'],
      'auth_type' => $type,
      );
    drupal_write_record('biblio_contributor_type', $ct_vals);
  }

  drupal_goto('admin/settings/biblio/fields/type');
}
function biblio_admin_types_hide() {
  $args = func_get_args();
  if ($args[0] > 0 && is_numeric($args[0])) {
    $result = db_query('UPDATE {biblio_types} SET visible = 0 WHERE tid = %d ', $args[0]);
  }
  drupal_goto('admin/settings/biblio/fields/type');
}
function biblio_admin_types_show() {
  $args = func_get_args();
  if ($args[0] > 0 && is_numeric($args[0])) {
    $result = db_query('UPDATE {biblio_types} SET visible = 1 WHERE tid = %d ', $args[0]);
  }
  drupal_goto('admin/settings/biblio/fields/type');
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
    )) . $existing_msg, $_GET['destination'] ? $_GET['destination'] : 'admin/settings/biblio/fields/type', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
    return $output;
  }
  else {
    drupal_goto('admin/settings/biblio/fields/type');
  }
}
function biblio_admin_types_delete_form_submit($form, & $form_state) {
  db_query('DELETE FROM {biblio_types} WHERE tid = %d', $form_state['values']['tid']);
  db_query('DELETE FROM {biblio_field_type} WHERE tid = %d', $form_state['values']['tid']);
  drupal_goto('admin/settings/biblio/fields/type');
}
function biblio_admin_types_reset_form() {
  $form['reset'] = array(
    '#type' => 'value',
    '#value' => 'reset'
  );
  $output = confirm_form($form, t('Are you sure you want to reset ALL the field definitions to the defaults? '), $_GET['destination'] ? $_GET['destination'] : 'admin/settings/biblio/fields/type', t('By doing this, you will loose all customizations you have made to the field titles, <b><u>this action cannot be undone</u></b>!'), t('Reset!'), t('Cancel'));
  return $output;
}
function biblio_admin_types_reset_form_submit($form, & $form_state) {
  module_load_include('install', 'biblio');
  biblio_reset_types();
  drupal_goto("admin/settings/biblio/fields/type");
}
/*
 * This functin is used by both the admin/settings/biblio page and user profile page
 *   - if $user is set, then it is being called from the user profile page
 */
function _biblio_get_user_profile_form(& $form, $profile_user = FALSE) {
  global $user;
  $allow_edit = variable_get('biblio_show_user_profile_form', '1');
  $allow_edit = $allow_edit || user_access('administer biblio');

  $form['biblio_profile'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' =>  TRUE,
    '#title' => ($profile_user) ? t('My publications') : t('Profile pages')
  );
  if(!$profile_user) {
    $form['biblio_profile']['biblio_show_user_profile_form'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to override these settings on their "My account" page') ,
      '#return_value' => 1,
      '#description' =>  t('If this is selected, a form similar to this section will be available to the user when they edit their own account information.  This will allow them to override the global preferences set here.') ,
      '#default_value' => variable_get('biblio_show_user_profile_form', '1')
    );
  }
  $form['biblio_profile']['biblio_show_profile'] = array(
    '#type' => 'checkbox',
    '#title' => ($profile_user) ? t('Show my publications on my profile page') : t('Show publications on users profile pages'),
    '#return_value' => 1,
    '#disabled' => !$allow_edit,
    '#description' => ($profile_user) ? t('Selecting this will create a listing of your publications on your profile page') : t('This sets the site wide default, users may change this in their profile')
  );
  if ($profile_user) {
    $form['biblio_profile']['biblio_show_profile']['#default_value'] = (isset($profile_user->biblio_show_profile)) ? $profile_user->biblio_show_profile : variable_get('biblio_show_profile', '0');
  }
  else {
    $form['biblio_profile']['biblio_show_profile']['#default_value'] = variable_get('biblio_show_profile', '0');
  }
  $form['biblio_profile']['biblio_my_pubs_menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show "My publications" item in the navigation menu'),
    '#disabled' => !$allow_edit,
    '#return_value' => 1,
    '#description' => ''
  );
  if ($profile_user) {
    $form['biblio_profile']['biblio_my_pubs_menu']['#default_value'] = ((isset($profile_user->biblio_my_pubs_menu)) ? $profile_user->biblio_my_pubs_menu : variable_get('biblio_my_pubs_menu', '0'));
  }
  else {
    $form['biblio_profile']['biblio_my_pubs_menu']['#default_value'] = variable_get('biblio_my_pubs_menu', '0');
  }
  if ($profile_user) {
    $db_result = db_query("SELECT cd.lastname, cd.firstname, cd.initials, cd.cid FROM {biblio_contributor_data} cd
            ORDER by cd.lastname ASC ");
    $options = array();
    $options[0] = t('(None)');

    while ($row = db_fetch_object($db_result)) {
        $options[$row->cid] =  "$row->lastname, $row->firstname $row->initials ";
    }
    $form['biblio_profile']['biblio_contributor_id'] = array(
    '#type' => 'select',
    '#title' => t('Link My Profile with this Author from the Biblio database'),
    '#default_value' => $profile_user->biblio_contributor_id ,
    '#disabled' => !$allow_edit,
    '#options' => $options,
    '#description' => t('This will link your profile to the selected author from the biblio database, then all publications containing this author to be displayed on your "Publications" tab.'),
    );
    $options = array();
    $options['system'] = t('System default');
    $options = array_merge($options, _biblio_admin_get_styles() );
    $form['biblio_profile']['biblio_user_style'] = array(
    '#type' => 'radios',
    '#title' => t('Style'),
    '#default_value' => (isset($profile_user->biblio_user_style) ? $profile_user->biblio_user_style : 'system'),
    '#options' => $options,
    '#description' => t('Set the bibliographic style of the "list" view.')
  );

  }
}
function _biblio_get_user_doi_form(&$form, $user) {
  $form['biblio_doi'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' =>  TRUE,
    '#title' => t('CrossRef Login Information')
  );
  $link_attrs = array('attributes' => array('target' => '_blank'),
                      'absolue' => TRUE);
  $form['biblio_doi']['biblio_crossref_pid'] = array(
    '#type' => 'textfield',
    '#title' => t('CrossRef OpenURL Account ID'),
    '#default_value' => $user->biblio_crossref_pid,
    '#return_value' => 1,
    '#description' => t('Enter your CrossRef OpenURL account ID which you can obtain here ') .l('OpenURL Account Request Form', 'http://www.crossref.org/requestaccount/', $link_attrs)
  );
  return $form;
}
function _biblio_get_user_openurl_form(&$form, $user) {
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
    '#default_value' => !empty($user->biblio_baseopenurl) ? $user->biblio_baseopenurl :  '',
    '#description' => t('This sets your base <a href="http://en.wikipedia.org/wiki/OpenURL">OpenURL</a> gateway, which is used to generate OpenURL links. To implement a "Universal" OpenURL system, try using OCLC\'s <a href="http://www.oclc.org/productworks/urlresolver.htm">OpenURL Resolver Registry</a> gateway: <a href="http://worldcatlibraries.org/registry/gateway">http://worldcatlibraries.org/registry/gateway</a>')
  );
  $sid = "Biblio:". variable_get('site_name', 'Drupal');
  $form['openurl']['biblio_openurl_sid'] = array(
    '#type' => 'textfield',
    '#title' => t('OpenURL Site ID'),
    '#size' => 95,
    '#default_value' => !empty($user->biblio_openurl_sid) ? $user->biblio_openurl_sid : '',
    '#description' => t('This sets your site name, some link resolvers will require a specific Site ID in order to process your requests.')
  );
  return $form;
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
    while (($files[] = readdir($dh)) !== FALSE) {}
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
  ksort($styles);
  return $styles;
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
      return ('SELECT lastname, firstname, initials, COUNT(lastname) as cnt  FROM {biblio_contributor_data} GROUP BY lastname, firstname, initials HAVING COUNT(*)>1 ORDER BY lastname ASC, firstname ASC, initials ASC ');
      break;
    case "author_by_name" :
      return ("SELECT lastname, cid FROM {biblio_contributor_data} where lastname LIKE '%s'");
      break;
    case 'author_pub_count' :
//      return ('SELECT bd.lastname, b.cid,COUNT(*) AS cnt FROM {biblio_contributor} b, {biblio_contributor_data} bd WHERE bd.cid=b.cid GROUP BY b.cid HAVING cnt > 0 ORDER BY lastname ASC');
      return ('SELECT bd.lastname, b.cid,COUNT(*) AS cnt FROM {biblio_contributor} b, {biblio_contributor_data} bd WHERE bd.cid=b.cid GROUP BY b.cid, bd.lastname HAVING COUNT(*) > 0 ORDER BY lastname ASC');
      break;
  }
}


function biblio_admin_author_types_form($form_state, $op = NULL, $id = NULL) {
  switch ($op) {
    case 'edit':
      $type = db_fetch_object(db_query("SELECT * FROM {biblio_contributor_type_data} WHERE auth_type=%d",$id));
      $form['auth_type'] = array(
        '#type' => 'hidden',
        '#value' => $type->auth_type);
        //fall through and use the same form used for a new entry
    case 'new':
      $form['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Type Name'),
        '#size' => 20,
        '#weight' => 1,
        '#default_value' =>$type->title,
        '#required' => TRUE,
        '#maxlength' => 64
      );
      $form['hint'] = array(
        '#type' => 'textfield',
        '#title' => t('Description'),
    		'#size' => 60,
    		'#weight' => 2,
    		'#default_value' => $type->hint,
    		'#maxlength' => 255
      );
      $form['type_button'] = array(
    		'#type' => 'submit',
    		'#value' => ($op == 'new')? t('Create New Type') : t('Save'),
    		'#weight' => 3,
      	'#submit' => array('biblio_admin_author_types_form_submit')
      );
      $form['#redirect'] = 'admin/settings/biblio/author/type';
      return $form;
      break;
    case 'hide':
      break;
    default:
  }
}

function theme_biblio_admin_author_types_form($form) {
  // We need this complex query to realize author_types which are not in use (cid is NULL)
  $db_result = db_query("SELECT ctd.*, cid FROM {biblio_contributor_type_data} ctd
                         LEFT JOIN {biblio_contributor} c ON ctd.auth_type = c.auth_type
                         GROUP BY ctd.auth_type");
  while ($row = db_fetch_object($db_result)) {
    $ops = l(t('edit'), 'admin/settings/biblio/author/type/'.$row->auth_type.'/edit' );
    if (!isset($row->cid) && $row->auth_type >= 10) { // allow delete only if type not in use
      $ops .= '&nbsp;&nbsp;';
      $ops .= l(t('delete'), 'admin/settings/biblio/author/type/'.$row->auth_type.'/delete/');
    }
    $rows[] = array(
      $row->auth_type,
      $row->title,
      $row->hint,
      $ops,
    );
  }
  $header = array(
    t('Type Id'),
    t('Contributor Type'),
    t('Description'),
    array('data' => t('Operations'),'colspan' => '2')
  );
  $output .= '<p>[ '. l(t('Add New Type'), 'admin/settings/biblio/author/type/new') .' ]';
  $output .= theme('table', $header, $rows);
  $output .= '<p>[ '. l(t('Add New Type'), 'admin/settings/biblio/author/type/new') .' ]';
 // $output .= ' [ '. l(t('Reset all types to defaults'), 'admin/settings/biblio/authors/reset') .' ]';

  return $output;
}

function biblio_admin_author_types_form_submit($form, $form_state) {

  $record->title = $form_state['values']['title'];
  $record->hint = $form_state['values']['hint'];

  switch($form['#id']) {
    case 'biblio-admin-author-types-form-new':
      $record->title = $form_state['values']['title'];
      $record->hint = $form_state['values']['hint'];
      drupal_write_record('biblio_contributor_type_data',$record);
      break;
    case 'biblio-admin-author-types-form-edit':
      $record->auth_type = $form_state['values']['auth_type'];
      drupal_write_record('biblio_contributor_type_data',$record,'auth_type');
      break;

  }
}
function biblio_admin_author_type_delete_confirm(&$form_state, $type_id) {
  $base = variable_get('biblio_base', 'biblio');
  $type_data = db_fetch_object(db_query('SELECT * FROM {biblio_contributor_type_data} bctd WHERE bctd.auth_type = %d ', $type_id));
  $form['auth_type'] = array(
    '#type' => 'value',
    '#value' => $type_data->auth_type,
  );

  return confirm_form($form,
    t('Are you sure you want to delete the author type: %title ?', array('%title' => $type_data->title)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/biblio/author/type',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );

}
function biblio_admin_author_type_delete_confirm_submit($form, &$form_state) {

  db_query("DELETE FROM {biblio_contributor_type_data} WHERE auth_type=%d", $form_state['values']['auth_type']);
  db_query("DELETE FROM {biblio_contributor_type} WHERE auth_type=%d", $form_state['values']['auth_type']);

  drupal_goto('admin/settings/biblio/author/type');
}

function biblio_admin_author_edit_form($form_state, $author_id) {
  $author = db_fetch_object(db_query('SELECT * FROM {biblio_contributor_data} b WHERE b.cid = %d ', $author_id));
  if (!$author) {
    drupal_not_found();
    return;
  }

  $base = variable_get('biblio_base', 'biblio');
  $menu = menu_get_active_title();
  $path = (strpos($_GET['q'],'settings'))? 'admin/settings/biblio/author' : $base.'/authors';
  $form['#redirect'] = $path;

  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $author_id
  );
  $users = db_query('SELECT uid,name FROM {users} WHERE uid>0 ORDER BY name');
  $options[0] = t('(None)');
  while($user=db_fetch_array($users)) {
    $options[$user['uid']] = $user['name'];
  }
  $form['drupal_uid'] = array(
    '#type' => 'select',
    '#title' => t('Drupal User ID'),
		'#options' => $options,
    '#default_value' => $author->drupal_uid,
    '#weight' => 12,
    '#required' => FALSE,
    );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Complete Name'),
    '#default_value' => $author->name,
    '#size' => 100,
    '#weight' => 1,
    '#required' => TRUE,
    '#maxlength' => 128
  );
  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Prefix'),
    '#default_value' => $author->prefix,
    '#size' => 20,
    '#weight' => 2,
    '#required' => FALSE,
    '#maxlength' => 128
  );
  $form['firstname'] = array(
    '#type' => 'textfield',
    '#title' => t('First Name'),
    '#default_value' => $author->firstname,
    '#size' => 20,
    '#weight' => 3,
    '#required' => FALSE,
    '#maxlength' => 128
  );
  $form['initials'] = array(
    '#type' => 'textfield',
    '#title' => t('Initials'),
    '#default_value' => $author->initials,
    '#size' => 20,
    '#weight' => 4,
    '#required' => FALSE,
    '#maxlength' => 10
  );
  $form['lastname'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Name'),
    '#default_value' => $author->lastname,
    '#size' => 20,
    '#weight' => 5,
    '#required' => TRUE,
    '#maxlength' => 128
  );
  $form['suffix'] = array(
    '#type' => 'textfield',
    '#title' => t('Suffix'),
    '#default_value' => $author->suffix,
    '#size' => 20,
    '#weight' => 6,
    '#required' => FALSE,
    '#maxlength' => 128
  );
  $form['affiliation'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliation'),
    '#default_value' => $author->affiliation,
    '#size' => 100,
    '#weight' => 7,
    '#required' => FALSE,
    '#maxlength' => 256
  );
  $authors = db_query("SELECT * FROM {biblio_contributor_data} cd WHERE cid != %d ORDER BY lastname", $author_id);
  $base = variable_get('biblio_base', 'biblio');
  while($other_author=db_fetch_array($authors)) {
    if ((stripos($other_author['name'],$author->lastname) !== FALSE) && $author->aka != $other_author['cid']) {
      $merge_options[$other_author['cid']] = l($other_author['name'],$base.'/authors/'.$other_author['cid'].'/edit/');
      if ($other_author['affiliation']) $merge_options[$other_author['cid']] .= ' (' . $other_author['affiliation'] . ')';
    }
    if ($author->aka == $other_author['aka']) {
      $linked[$other_author['cid']] = l($other_author['lastname']. ($other_author['firstname']?', '.$other_author['firstname']:($other_author['initials']?', '.$other_author['initials']:'')),$base.'/authors/'.$other_author['cid'].'/edit/');
      if ($other_author['affiliation']) $linked[$other_author['cid']] .= ' (' . $other_author['affiliation'] . ')';
      $linked_default_values[] = $other_author['cid'];
    }
    else {
      $link_options[$other_author['cid']] = $other_author['lastname']. ($other_author['firstname']?', '.$other_author['firstname']:($other_author['initials']?', '.$other_author['initials']:''));
      if ($other_author['affiliation']) $link_options[$other_author['cid']] .= ' (' . $other_author['affiliation'] . ')';
    }
  }
  if (count($merge_options)) {
    $form['merge'] = array(
      '#type' => 'fieldset',
      '#title' => t('Author Merge'),
      '#description' => t('Select other author names which will be merged.  Merging removes all the selected authors, then replaces any references to these authors with the author being edited above.  You should only do this if you are sure the other authors represent the same author as the one being edited. <B>THIS CANNOT BE UNDONE!</B>'),
      '#weight' => 5,
      '#collapsible' => TRUE,
      '#collapsed' => !count($merge_options),
    );

    $form['merge']['merge_authors'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Similar authors'),
	    '#options' => $merge_options,
      '#weight' => 12,
      '#required' => FALSE,
    );
    $form['merge']['merge'] = array(
      '#type' => 'submit',
      '#value' => t('Merge')
    );
  }

    $form['link'] = array(
      '#type' => 'fieldset',
      '#title' => t('Author Link'),
      '#description' => t('Select other author names which will be linked.  Linking relates all the selected authors to the author being edited above, and and keeps each instance in the database.  You should only do this if you are sure the other authors represent the same author as the one being edited. '),
      '#weight' => 5,
      '#collapsible' => TRUE,
      '#collapsed' => !count($linked),
    );
    if (count($linked)) {
      $form['link']['linked_authors'] = array(
        '#type' => 'checkboxes',
        '#title' => t('This author has already been linked to these authors'),
        '#default_value' => $linked_default_values,
        '#options' => $linked,
        '#weight' => 12,
        '#required' => FALSE,
      );
    }
    $form['link']['link_authors'] = array(
      '#type' => 'select',
      '#title' => t('Other authors which could be linked to this author'),
      //'#default_value' => $link_default_values,
      '#options' => $link_options,
      '#weight' => 12,
      '#required' => FALSE,
      '#multiple' => TRUE,
    );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  return $form;
}
function biblio_admin_author_edit_form_validate($form, &$form_state) {
  foreach($form_state['values'] as $key => $value) {
    if (is_string($value)) $form_state['values'][$key] = trim($value);
  }
}
function biblio_admin_author_edit_form_submit($form, &$form_state) {
  $op = $form_state['values']['op'];
  if ($form_state['values']['drupal_uid'] == 0 ) {
    $uid = $form['drupal_uid']['#default_value'];
    $cid = 0;
  }
  else {
    $uid = $form_state['values']['drupal_uid'];
    $cid = $form_state['values']['cid'] ;
  }

  db_query("UPDATE {biblio_contributor_data} SET drupal_uid = 0 WHERE drupal_uid = %d", $uid);
  $data = unserialize(db_result(db_query('SELECT data FROM {users} WHERE uid = %d', $uid)));
  $data['biblio_contributor_id'] = $cid;
  $v[] = serialize($data);
  db_query("UPDATE {users} SET data = '%s' WHERE uid = %d", array_merge($v, array($uid)));

  drupal_write_record('biblio_contributor_data', $form_state['values'], 'cid');
  // merge authors
  if (isset($form_state['values']['merge_authors']) && $op == t('Merge')) {
    $merge_authors = array_filter($form_state['values']['merge_authors']);
    if (count($merge_authors) ) {
      $authors = implode(',', $merge_authors);
      db_query("UPDATE {biblio_contributor} SET cid={$form_state['values']['cid']} WHERE cid IN ($authors)");
      db_query("DELETE FROM {biblio_contributor_data} WHERE cid IN ($authors)");
    }
  }
  if (isset($form_state['values']['linked_authors']) ) {
    foreach ($form_state['values']['linked_authors'] as $key => $value) {
      if ($value == 0) {
        db_query("UPDATE {biblio_contributor_data} SET aka=$key WHERE cid=$key");
      }
    }
  }
  if (isset($form_state['values']['link_authors']) ) {
    //db_query("UPDATE {biblio_contributor_data} SET aka=$form_state['values']['cid'] WHERE cid IN ($authors)");
    foreach ($form_state['values']['link_authors'] as $key => $value) {
        db_query("UPDATE {biblio_contributor_data} SET aka={$form_state['values']['cid']} WHERE cid=$key");
    }
  }
}


function biblio_admin_orphans_form($form_state) {
  $base = variable_get('biblio_base', 'biblio');
  $result = pager_query('SELECT * FROM {biblio_contributor_data} WHERE cid NOT IN (SELECT cid FROM {biblio_contributor} GROUP BY cid)', 50);
  $authors = array();
  while ($author = db_fetch_object($result)) {
    $authors[$author->cid] = '';
    $form['name'][$author->cid] = array('#value' => l($author->name, $base.'/authors/'.$author->cid.'/edit'));
    $form['affiliation'][$author->cid] = array('#value' => check_plain($author->affiliation));
  }
  $form['authors'] = array('#type' => 'checkboxes', '#options' => $authors);
  $form['pager']   = array('#value' => theme('pager', NULL, 50, 0));
  $form['#theme'] = 'biblio_admin_orphans_form';
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#disabled' => (!count($authors)),
    '#submit' => array('biblio_admin_orphans_form_submit'),
  );
  return $form;
}
function biblio_admin_orphans_form_validate($form, &$form_state) {
  $check_count = array_filter($form_state['values']['authors']);
  if (count($check_count) == 0) {
    form_set_error('', t('No items selected.'));
  }
}

function biblio_admin_orphans_form_submit($form, &$form_state) {
  // Filter out unchecked authors
  $authors = array_filter($form_state['values']['authors']);
  update_sql('DELETE FROM {biblio_contributor_data} WHERE cid IN ('.implode(',',$authors).')');
  drupal_set_message(t('The orphaned authors ('.implode(',',$authors).') have been deleted.'));

}
function biblio_admin_keyword_orphans_form($form_state) {
  $base = variable_get('biblio_base', 'biblio');
  $result = pager_query('SELECT * FROM {biblio_keyword_data} WHERE kid NOT IN (SELECT kid FROM {biblio_keyword} GROUP BY kid)', 50);
  $keywords = array();
  while ($keyword = db_fetch_object($result)) {
    $keywords[$keyword->kid] = '';
    $form['keyword'][$keyword->kid] = array('#value' => l($keyword->word, $base.'/keyword/'.$keyword->kid.'/edit'));
  }
  $form['keywords'] = array('#type' => 'checkboxes', '#options' => $keywords);

  $form['pager']   = array('#value' => theme('pager', NULL, 50, 0));
  $form['#theme'] = 'biblio_admin_keyword_orphans_form';
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#disabled' => (!count($keywords)),
    '#submit' => array('biblio_admin_keyword_orphans_form_submit'),
  );
  return $form;
}
function biblio_admin_keyword_orphans_form_validate($form, &$form_state) {
  $check_count = array_filter($form_state['values']['keywords']);
  if (count($check_count) == 0) {
    form_set_error('', t('No items selected.'));
  }
}

function biblio_admin_keyword_orphans_form_submit($form, &$form_state) {

  $keywords = array_filter($form_state['values']['keywords']);
  update_sql('DELETE FROM {biblio_keyword_data} WHERE kid IN ('.implode(',',$keywords).')');
  $count = count($keywords);
  drupal_set_message(t('%count orphaned keywords have been deleted.', array('%count' => $count)));

}

function biblio_admin_keyword_edit_form($form_state, $keyword_id) {
  $base = variable_get('biblio_base', 'biblio');

  $keyword = db_fetch_object(db_query('SELECT * FROM {biblio_keyword_data} bkd WHERE bkd.kid = %d ', $keyword_id));

  $base = variable_get('biblio_base', 'biblio');
  $path = (strpos($_GET['q'],'settings'))? 'admin/settings/biblio/keywords' : $base.'/keywords';
  $form['#redirect'] = $path;

  $form['kid'] = array(
    '#type' => 'value',
    '#value' => $keyword_id
  );
  $form['word'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword'),
    '#default_value' => $keyword->word,
    '#size' => 100,
    '#weight' => -10,
    '#required' => TRUE,
    '#maxlength' => 255
  );

  $keywords = db_query("SELECT * FROM {biblio_keyword_data} kd WHERE word LIKE '%%%s%%' AND kid!=%d", drupal_substr($keyword->word,1,5), $keyword_id);

  unset($options);
  $base = variable_get('biblio_base', 'biblio');
  while($kw=db_fetch_object($keywords)) {
    $options[$kw->kid] = l($kw->word,$base.'/keywords/'.$kw->kid.'/edit/');
  }
  if ($options) {
    $form['merge'] = array(
      '#type' => 'fieldset',
      '#title' => t('Keyword Merge'),
      '#description' => t('Select other keywords which will be merged with the one above.  You should only do this if you are sure the other keywords represent the same keyword as the one being edited.'),
      '#weight' => 5,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['merge']['merge_keywords'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Similar keywords'),
	    '#options' => $options,
      '#weight' => 12,
      '#required' => FALSE,
      '#multiple' => TRUE,
    '#size' => min(count($options), 6),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  $form['delete'] = array(
    '#value' => l(t('Delete'), $base.'/keyword/'.$keyword_id.'/delete')
  );

  return $form;
}

function biblio_admin_keyword_delete_confirm(&$form_state, $keyword_id) {
  $base = variable_get('biblio_base', 'biblio');
  $keyword = db_fetch_object(db_query('SELECT * FROM {biblio_keyword_data} bkd WHERE bkd.kid = %d ', $keyword_id));
  $form['kid'] = array(
    '#type' => 'value',
    '#value' => $keyword_id,
  );

  return confirm_form($form,
    t('Are you sure you want to delete the keyword: %title from ALL publications?', array('%title' => $keyword->word)),
    isset($_GET['destination']) ? $_GET['destination'] : $base.'/keywords',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );

}

function biblio_admin_keyword_delete_confirm_submit($form, &$form_state) {
  $base = variable_get('biblio_base', 'biblio');
  module_load_include('inc','biblio','biblio.keywords');
  biblio_delete_keyword($form_state['values']['kid']);
  drupal_goto($base.'/keywords');
}

function biblio_admin_keyword_edit_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Save')) {
    drupal_write_record('biblio_keyword_data', $form_state['values'], 'kid');

    if (count($form_state['values']['merge_keywords'])) {
      $kids = implode(',', $form_state['values']['merge_keywords']);
      update_sql("UPDATE {biblio_keyword} SET kid={$form_state['values']['kid']} WHERE kid IN ($kids)");
      update_sql("DELETE FROM {biblio_keyword_data} WHERE kid IN ($kids)");
    }
  }
}
