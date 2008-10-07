<?PHP

// $Id: biblio.import.export.inc,v 1.7.2.17 2008/05/21 02:03:49 rjerome Exp $
/**
 * @file
 * Functions that are used to import and export biblio data.
 *
 */
/*   biblio.import.export.inc
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
 * Return a form select box populated with all the users of the site.
 *
 * @param $my_uid
 *   The user id of the person accessing the form so the select box defaults
 *   to their userid
 * @return
 *   An array which will be used by the form builder to add a select box to a form
 */
function _biblio_admin_build_user_select($my_uid) {
  $sql = 'SELECT DISTINCT u.uid, u.name, u.status, u.mail FROM {users} u  WHERE u.uid != 0 ';
  $result = db_query($sql);
  while ($user = db_fetch_object($result)) {
    $users[$user->uid] = $user->name ." ($user->mail)";
  }
  asort($users);
  $select = array (
    '#type' => 'select',
    '#title' => t("Set user ID of entries in this file to"),
    '#options' => $users,
    '#default_value' => $my_uid
  );
  return $select;
}
/**
 * Return a form used to import files into biblio.
 *
 * @return
 *   An array which will be used by the form builder to build the import form
 */
function biblio_import_form() {
  global $user;
  if (biblio_access('import')) { // && !user_access('administer nodes')) {
    $form['#attributes']['enctype'] = 'multipart/form-data';
    $form["file_upload"] = array (
      '#type' => 'file',
      '#title' => t('Import file'),
      '#default_value' => '',
      '#size' => 60
    );
    $form['filetype'] = array (
      '#type' => 'radios',
      '#title' => t('File Type'),
      '#default_value' => '',
      '#options' => array (
        'tagged' => t('EndNote Tagged'),
        'ris'    => t('RIS'),
        'xml'    => t('EndNote 7 XML'),
        'xml8'   => t('EndNote 8+ XML'),
        'bib'    => t('BibTex')
      )
    );
    if ($user->uid) {
      $form ['userid'] = _biblio_admin_build_user_select($user->uid);
    }
    // Get the vocabularies  attached to the biblio node type ...
    $vocabularies = module_invoke('taxonomy', 'get_vocabularies', 'biblio');
    // ... and print a form to select the terms in each of them
    $form['import_taxonomy'] = array (
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Taxonomy Settings'),
      '#description' => t('Here you may select terms to be assigned to imported records (if any vocabularies are assigned to the biblio node type)')
    );
    if (count($vocabularies)) {
      $i = 0;
      foreach ($vocabularies as $vocabulary) {
        $form['import_taxonomy']['vocabulary'. $i] = module_invoke('taxonomy', 'form', $vocabulary->vid, 0);
        $form['import_taxonomy']['vocabulary'. $i++]['#description'] = t("Select taxonomy term to assigned to imported entries");
      }
    } else {
      $vocab_msg = t('There are currently no vocabularies assigned to the biblio node type, please go the the !url page to fix this', array ('!url' => l(t('admin/content/taxonomy'), 'admin/content/taxonomy')));
      $form['import_taxonomy']['vocabulary_message'] = array (
        '#value' => '<p><div>'. $vocab_msg .'</div></p>'
      );
    }
    if (variable_get('biblio_keyword_freetagging', 0)) {
      $freetag_vocab = module_invoke('taxonomy', 'get_vocabulary', variable_get('biblio_freetagging_vocab', 0));
      $msg = t('Keywords will be added to the <b>@name</b> vocabulary as specified in the Taxonomy section of the !url page.', array ('@name' => $freetag_vocab->name, '!url' => l(t('admin/settings/biblio'), 'admin/settings/biblio')));
    } else {
      $msg = t('Keywords will <b>NOT</b> be added to the vocabulary as specified in the Taxonomy section of the !url page.', array ('!url' => l(t('admin/settings/biblio'), 'admin/settings/biblio')));
    }
    $form['import_taxonomy']['freetagging_information'] = array (
      '#value' => '<p><div>'. $msg .'</div></p>'
    );
    $form['button'] = array ('#type' => 'submit', '#value' => t('Import'));
    return $form;
    //print theme('page', form($form, 'post', null, $attributes));
  } else {
    drupal_set_message("You are not authorized to access the biblio import page", 'error');
    print theme('page', '');
  }
}

/**
 * Implementation of hook_validate() for the biblio_import_form.
 */
function biblio_import_form_validate($form, & $form_state) {
  $op = $form_state['values']['op'];
  $filetype = $form_state['values']['filetype'];
  if (!file_save_upload('file_upload')) {
    form_set_error('', t("You must provide a file name"));
  }
  if ($op == t('Import') && $filetype == '') {
    form_set_error('', t("You must select a file type"));
  }
}
/**
 * Implementation of hook_submit() for the biblio_import_form.
 */
function biblio_import_form_submit($form, & $form_state) {
  if ($form_state['values']['op'] == t('Import') && isset ($form_state['values']['filetype'])) {
    if ($import = file_save_upload('file_upload')) {
      drupal_set_message(t("@file was uploaded", array ('@file' => $import->filename)), 'status');
      if (($file_content = @ file_get_contents($import->filepath)) === false) {
        drupal_set_message(t("Could not read contents of @file ", array ('@file' => $import->filename)), 'error');
        return;
      } else {
        // Concatenate all the terms of the different vocabularies
        // in a single array to be sent to biblio_import
        $terms = array ();
        foreach (array_keys($form_state['values']) as $key) {
          if (preg_match('/(vocabulary[0-9]+)/', $key)) {
            if ($form_state['values'][$key] > 0)
              $terms[] = $form_state['values'][$key];
          }
        }
        // Added the $terms argument
        // the array of terms to be attached to the node(s)
        $userid = (isset ($form_state['values']['userid'])) ? $form_state['values']['userid'] : 1;
        $filetype = $form_state['values']['filetype'];
        $content = biblio_import($file_content, $filetype, $userid, $terms);
        file_delete($import->filepath);
      }
    } else {
      drupal_set_message(t("File was NOT successfully uploaded"), 'error');
    }
  }
}

function biblio_export_form() {
  $form['pot'] = array (
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('POT Export'),
    '#description' => t('Here you may export a ".pot" file which contains the titles and hints from the database which are not normally captured by traslation extrators)')
  );
  $form['pot']['button'] = array (
    '#type' => 'submit',
    '#value' => t('Export translation data')
  );

  return $form;
}
function biblio_export_form_submit($form, & $form_state) {
  if ($form_state['values']['op'] == t('Export translation data')) {
    biblio_dump_db_data_for_pot();
  }

}

/**
 * Import data from a file and return the node ids created.
 *
 * @param $userid
 *   The user id of that will be assigned to each node imported
 * @param $filename
 *   The name of the file containing the data to import
 * @param $type
 *   The format of the file to be imported (tagged, XML, RIS, bibTEX)
 * @param $terms
 *   the vocabulary that the imported nodes will be associated with
 * @return
 *   An array the node id's of the items imported
 */
function biblio_import(& $file_content, $type = 'tagged', $userid = 1, $terms = null, $save = true) {
  global $user, $node_array, $node_template;
  $node_template = array ();
  $node_array = array ();
  $parsed = 0;
  if (strlen($file_content)) {
    $options = variable_get('node_options_biblio', array ('status'));
    if (module_exists('i18n') && variable_get('i18n_node_biblio', 0)) {
      $node['language'] = module_invoke('i18n', 'default_language');
    }
    $account = user_load($userid);
    $node_template = array (
      'type' => 'biblio',
      'comment' => variable_get('comment_biblio', 0),
      'promote' => in_array('promote', $options),
      'moderate' => in_array('moderate', $options),
      'sticky' => in_array('sticky', $options),
      'format' => 0,
      'status' => in_array('status', $options),
      'uid' => user_access('administer nodes') ? $account->uid : $user->uid,
      'name' => user_access('administer nodes') ? $account->name : $user->name
    );
    switch ($type) {
      case 'tagged' : // EndNote Tagged
        require_once (drupal_get_path('module', 'biblio') .'/tagged_parser.inc');
        $parsed = _endnote_tagged_import($file_content, $node_template, $node_array);
        break;
      case 'xml' : // EndNote 7 XML
        $parsed = biblio_endnote_XML_import($file_content, $node_template, $node_array, 7);
        break;
      case 'xml8' : // EndNote 8+ XML
        $parsed = biblio_endnote_XML_import($file_content, $node_template, $node_array, 8);
        break;
      case 'bib' : // BibTex
        $parsed = biblio_bibtex_import($file_content, $node_template, $node_array);
        break;
      case 'csv' : // comma separated variable file
        $parsed = biblio_csv_import($file_content, $node_template, $node_array);
        break;
      case 'biblio_backup' : // a complete backup of all biblio information
        $parsed = biblio_restore($file_content, $node_template, $node_array);
        break;
      case 'ris' : // RIS
        require_once (drupal_get_path('module', 'biblio') .'/ris_parser.inc');
        $parsed = _ris_tagged_import($file_content, $node_template, $node_array);
        break;
    }
    if ($parsed > 0 && $save === true) {
      $node_ids = biblio_save_imported_nodes($node_array);
      if ($terms && count($node_ids)) {
        // set the vocabulary for the returned nodes
        foreach ($node_ids as $node_id) {
          module_invoke('taxonomy', 'node_save', $node_id, $terms);
        }
      }
      if (count($node_ids)) {
        db_query('UPDATE {node} SET uid = %d WHERE nid IN(%s)', $userid, implode(',', $node_ids));
        db_query('UPDATE {node_revisions} SET uid = %d WHERE nid IN(%s)', $userid, implode(',', $node_ids));
      }

      return $node_ids;
    }
    elseif ($parsed && !$save) {
      return $node_array;
    }

  } else {
    return false;
  }
}
/**
 * Export nodes in a given file format.
 *
 * @param $format
 *   The file format to export the nodes in (tagged, XML, bibTEX)
 * @param $nid
 *   If not NULL, then export only the given nodeid, else we will
 *   use the session variable which holds the most recent query. If neither
 *   $nid or the session variable are set, then nothing is exported
 * @param $version
 *   The version of EndNote XML to use.  There is one format for ver. 1-7 and
 *   a different format for versions 8 and greater.
 * @return
 *   none
 */
function biblio_export($format = "tagged", $nid = null, $version = 7) {
  require_once (drupal_get_path('module', 'biblio') .'/biblio.contributors.inc');
  $params = array ();
  if ($nid === null && isset ($_SESSION['last_biblio_query']) && !empty ($_SESSION['last_biblio_query'])) {
    $query = $_SESSION['last_biblio_query'];
    $params = $_SESSION['last_biblio_query_terms'];
  }
  elseif (!empty ($nid)) {
    $query = db_rewrite_sql("SELECT * FROM {node} n left join {biblio} b  on n.vid=b.vid WHERE n.nid=%d ");
    $params[] = $nid;
  } else {
    return;
  }
  $result = db_query($query, $params);
  while ($node = db_fetch_object($result)) {
    $node->biblio_contributors = biblio_load_contributors($node->vid);
    $results[] = $node;
  }

  if (count($results)) {
    switch ($format) {
      case "tagged" :
        drupal_set_header('Content-type: 	application/x-endnote-refer');
        drupal_set_header('Content-Disposition:  filename="Drupal-Biblio.enw"');
        print biblio_endnote_tagged_export($results);
        break;
      case "xml" :
        drupal_set_header('Content-type: application/xml; charset=utf-8');
        drupal_set_header('Content-Disposition: attachment; filename="Biblio-EndNote'. $version .'.xml"');
        print biblio_endnote_XML_export($results, $version);
        break;
      case "bibtex" :
        drupal_set_header('Content-type: 	application/text; charset=utf-8');
        drupal_set_header('Content-Disposition:  filename="Biblio-Bibtex.bib"');
        print biblio_bibtex_export($results);
        break;
      case "csv" :
        drupal_set_header('Content-Type: application/text; charset=utf-8');
        drupal_set_header('Content-Disposition: attachment; filename=Biblio-export.csv');
        print biblio_csv_export($results);
        break;
    }
  }

}
/**
 * Import bibtex data.
 *
 * @param $data
 *   the contents of a bibtex file passed as one big string
 * @param $node
 *   an array (populated in biblio_import() ), containing the boiler plate
 *   information common to all nodes
 * @return
 *   an array of node ids
 */
function biblio_bibtex_import(& $data, $node_template, & $node_array) {
  require_once (drupal_get_path('module', 'biblio') .'/bibtexParse/PARSEENTRIES.php');
  $bibtex = new PARSEENTRIES();
  $bibtex->loadBibtexString($data);
  $bibtex->extractEntries();
  if ($bibtex->count) {
    $bibtex->bib2node($node_array, $node_template);
  }
  return $bibtex->count;
}
/**
 * Export data in bibtex format.
 *
 * @param $result
 *   a database result set pointer
 * @return
 *   none
 */
function biblio_bibtex_export($results) {
  if (!is_array($results)) {
    $result_array[] = $results;
  } else {
    $result_array = $results;
  }
  $bibtex = '';
  foreach ($result_array as $pub) {
    $type = "article";
    $journal = $series = $booktitle = $school = $organization = null;
    switch ($pub->biblio_type) {
      case 102 :
        $type = "article";
        $journal = $pub->biblio_secondary_title;
        break;
      case 100 :
        $type = "book";
        $series = $pub->biblio_secondary_title;
        break;
      case 101 :
        $type = "inbook";
        $booktitle = $pub->biblio_secondary_title;
        break;
      case 100 :
        $type = "incollection";
        break;
      case 103 :
        $type = "inproceedings";
        $journal = $pub->biblio_secondary_title;
        $organization = $pub->biblio_publisher;
        $series = $pub->biblio_tertiary_title;
        break;
      case 129 :
        $type = "misc";
        break;
      case 108 :
        $type = "phdthesis";
        $school = $pub->biblio_publisher;
        $pub->biblio_publisher = null;
        break;
      case 104 :
        $type = "proceedings";
        break;
      case 109 :
        $type = "techreport";
        $organization = $pub->biblio_publisher;
        $pub->biblio_publisher = null;
        break;
      case 124 :
        $type = "unpublished";
        break;
    }
    $bibtex .= '@'. $type .' { ';
    $bibtex .= ($pub->biblio_citekey) ? $pub->biblio_citekey .",\n" : ",\n";
    if (!empty($pub->title))
      $bibtex .= "\ttitle = {". $pub->title ."},\n";
    if (!empty($journal))
      $bibtex .= "\tjournal = {". $journal ."},\n";
    if (!empty($booktitle))
      $bibtex .= "\tbooktitle = {". $booktitle ."},\n";
    if (!empty($series))
      $bibtex .= "\tseries = {". $series ."},\n";
    if (!empty($pub->biblio_volume))
      $bibtex .= "\tvolume = {". $pub->biblio_volume ."},\n";
    if (!empty($pub->biblio_number))
      $bibtex .= "\tnumber = {". $pub->biblio_number ."},\n";
    if (!empty($pub->biblio_year))
      $bibtex .= "\tyear = {". $pub->biblio_year ."},\n";
    if (!empty($pub->biblio_notes))
      $bibtex .= "\tnote = {". $pub->biblio_notes ."},\n";
    if (!empty($pub->biblio_date))
      $bibtex .= "\tmonth = {". $pub->biblio_date ."},\n";
    if (!empty($pub->biblio_pages))
      $bibtex .= "\tpages = {". $pub->biblio_pages ."},\n";
    if (!empty($pub->biblio_publisher))
      $bibtex .= "\tpublisher = {". $pub->biblio_publisher ."},\n";
    if (!empty($school))
      $bibtex .= "\tschool = {". $school ."},\n";
    if (!empty($organization))
      $bibtex .= "\torganization = {". $organization ."},\n";
    if (!empty($institution))
      $bibtex .= "\tinstitution = {". $institution ."},\n";
    if (!empty($pub->biblio_type_of_work))
      $bibtex .= "\ttype = {". $pub->biblio_type_of_work ."},\n";
    if (!empty($pub->biblio_edition))
      $bibtex .= "\tedition = {". $pub->biblio_edition ."},\n";
    if (!empty($pub->biblio_section))
      $bibtex .= "\tchapter = {". $pub->biblio_section ."},\n";
    if (!empty($pub->biblio_place_published))
      $bibtex .= "\taddress = {". $pub->biblio_place_published ."},\n";
    if (!empty($pub->biblio_abst_e))
      $bibtex .= "\tabstract = {". $pub->biblio_abst_e ."},\n";
    if (!empty($pub->biblio_keywords))
      $bibtex .= "\tkeywords = {". $pub->biblio_keywords ."},\n";
    if (!empty($pub->biblio_isbn))
      $bibtex .= "\tISBN = {". $pub->biblio_isbn ."},\n";
    if (!empty($pub->biblio_url))
      $bibtex .= "\tURL = {". $pub->biblio_url ."},\n";
    $a = $e = NULL;
    foreach ($pub->biblio_contributors as $auth) {
      switch ($auth['ctid']) {
        case 1 :
          $a .= (!empty ($a)) ? ' and '. trim($auth['name']) : trim($auth['name']);
          break;
        case 2 :
          $e .= (!empty ($e)) ? ' and '. trim($auth['name']) : trim($auth['name']);
          break;
      }
    }
    if (!empty ($a))
      $bibtex .= "\tauthor = {". $a ."}". (($e) ? ",\n" : "\n");
    if (!empty ($e))
      $bibtex .= "\teditor = {". $e ."}\n";
    $bibtex .= "}\n\n";
  }

  return $bibtex;
}
/**
 * Save node imported from a file.
 *
 * @param $node_array
 *   a 2 dimensional array containing all the node information
 * @return
 *   The node ids of the saved nodes
 */
function biblio_save_imported_nodes(& $node_array) {
  $dup_count = 0;
  if (function_exists('node_save')) {
    $res = db_query("SELECT nid, biblio_md5 FROM {biblio} ");
    while ($md5 = db_fetch_object($res)) {
      $sum[$md5->biblio_md5] = $md5->nid;
    }
    foreach ($node_array as $imp_node) {
      $node = (object) $imp_node;
      if (variable_get('biblio_keyword_freetagging', 0) && variable_get('biblio_freetagging_vocab', 0)) {
        $vid = variable_get('biblio_freetagging_vocab', 0);
        $node->taxonomy['tags'][$vid] = $node->biblio_keywords;
      }
      $hash = biblio_hash($node);

      node_save($node);
      if ($node->nid) {
        $node_ids[] = $node->nid;
        if (isset ($sum[$hash])) { // if this is a potential duplcate, write the nids of the pre-existing and new nodes
          $dup_map = array (
            'vid' => $sum[$hash],
            'did' => $node->nid
          );
          drupal_write_record('biblio_duplicates', $dup_map);
          $dup_count++;
        }
      }
    }
  }
  if ($dup_count)
    drupal_set_message(t("Detected @dupcount duplicate node(s) when importing", array ('@dupcount' => $dup_count)), 'error');

  drupal_set_message(t("Succesfully imported @count entries.", array ('@count' => count($node_ids))), 'status');

  return $node_ids;
}
/**
 * Import EndNote XML data.
 *
 * @param $data
 *   the contents of an EndNote XML file passed as one big string
 * @param $node
 *   boiler plate information common to all nodes
 * @param $version
 *   the EndNote version of the XML file.  EndNote uses one format up to version
 *   7 then change to another format in version 8 and greater.
 * @return
 *   The node ids of the saved nodes
 */
function biblio_endnote_XML_import(& $data, $node_template, & $node_array, $version = 8) {
  //global $node_array, $node_template;
  $xml_parser = drupal_xml_parser_create($data);
  // use case-folding so we are sure to find the tag in
  xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, false);
  xml_parser_set_option($xml_parser, XML_OPTION_SKIP_WHITE, true);
  if ($version == 8) {
    global $user, $records, $rec_count, $node_array, $node_template, $node_id;
    require_once (drupal_get_path('module', 'biblio') .'/endnote8_parser.inc');
    xml_set_element_handler($xml_parser, "en8_startElement", "en8_endElement");
    xml_set_character_data_handler($xml_parser, "en8_characterData");
    xml_parse($xml_parser, $data, 1);
  }
  elseif ($version == 7) {
    require_once (drupal_get_path('module', 'biblio') .'/endnote7_parser.inc');
    xml_parse_into_struct($xml_parser, $data, $values);
    en7_parse($values, $node_array, $node_template);
  }
  xml_parser_free($xml_parser);
  return count($node_array);
}
/**
 * Export data in EndNote XML format.
 *
 * @param $result
 *   a database pointer to a result set
 * @param $version
 *   the EndNote version of the XML file.  EndNote uses one format up to version
 *   7 then change to another format in version 8 and greater.
 * @return
 *   none
 */
function biblio_endnote_XML_export($result, $version = 7) {
  if ($version == 8) {
    require_once (drupal_get_path('module', 'biblio') .'/endnote8_export.inc');
    $xml = _endnote8_XML_export($result);
  }
  elseif ($version == 7) {
    require_once (drupal_get_path('module', 'biblio') .'/endnote7_export.inc');
    $xml = _endnote7_XML_export($result);
  }
  return $xml;
}
/**
 * Export data in EndNote tagged format.
 *
 * @param $result
 *   a database pointer to a result set
* @return
 *   none
 */
function biblio_endnote_tagged_export($results) {
  if (!is_array($results)) {
    $result_array[] = $results;
  } else {
    $result_array = $results;
  }
  $doc = "";
  foreach ($result_array as $pub) {
    switch ($pub->biblio_type) {
      case 100 :
        $doc .= "%0 Book\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break; // book section
      case 101 :
        $doc .= "%0 Book Section\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break; // book section
      case 102 :
        $doc .= "%0 Journal Article\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%J ". trim($pub->biblio_secondary_title) ."\r\n";
        break; // journal
      case 103 :
        $doc .= "%0 Conference Paper\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break;
      case 104 :
        $doc .= "%0 Conference Proceedings\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break; // conference proceedings
      case 105 :
        $doc .= "%0  Newspaper Article\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break;
      case 107 :
        $doc .= "%0  Web Page\r\n";
        break;
      case 108 :
        $doc .= "%0 Thesis\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break; // thesis
      case 109 :
        $doc .= "%0 Report\r\n";
        break; // report
      case 110 :
      case 111 :
        $doc .= "%0  Film or Broadcast\r\n";
        break;
      case 112 :
        $doc .= "%0  Artwork\r\n";
        break;
      case 114 :
        $doc .= "%0  Audiovisual Material\r\n";
        break;
      case 115 :
        $doc .= "%0  Hearing\r\n";
        break;
      case 116 :
        $doc .= "%0  Case\r\n";
        break;
      case 117 :
        $doc .= "%0  Bill\r\n";
        break;
      case 118 :
        $doc .= "%0  Statute\r\n";
        break;
      case 119 :
        $doc .= "%0  Patent\r\n";
        break;
      case 120 :
        $doc .= "%0  Personal Communication\r\n";
        break;
      case 121 :
        $doc .= "%0  Manuscript\r\n";
        break;
      case 122 :
        $doc .= "%0  Map\r\n";
        break;
      case 123 :
        $doc .= "%0  Chart or Table\r\n";
        break;
      case 124 :
        $doc .= "%0  Unpublished Work\r\n";
        break;
      case 125 :
        $doc .= "%0  Online Database\r\n";
        break;
      case 126 :
        $doc .= "%0  Government Document\r\n";
        break;
      case 127 :
        $doc .= "%0  Classical Work\r\n";
        break;
      case 128 :
        $doc .= "%0  Legal Rule or Regulation\r\n";
        break;
      case 119 :
        $doc .= "%0 Patent\r\n";
        if (!empty($pub->biblio_secondary_title))
          $doc .= "%B ". trim($pub->biblio_secondary_title) ."\r\n";
        break; // patent
      default :
        $doc .= "%0 Generic\r\n";
        break; // generic
    }
    if (isset($pub->biblio_year) && $pub->biblio_year < 9998)
      $doc .= "%D ". trim($pub->biblio_year) ."\r\n";
    if (!empty($pub->title))
      $doc .= "%T ". trim($pub->title) ."\r\n";
    foreach ($pub->biblio_contributors as $auth) {
      switch ($auth['ctid']) {
        case 1 :
          $doc .= "%A ". trim($auth['name']) ."\r\n";
          break;
        case 2 :
          $doc .= "%E ". trim($auth['name']) ."\r\n";
          break;
        case 3 :
          $doc .= "%Y ". trim($auth['name']) ."\r\n";
          break;
      }
    }

    if (!empty($pub->biblio_place_published))
      $doc .= "%C ". trim($pub->biblio_place_published) ."\r\n";
    if (!empty($pub->biblio_publisher))
      $doc .= "%I ". trim($pub->biblio_publisher) ."\r\n";
    if (trim($pub->biblio_keywords))
      $doc .= "%K ". trim($pub->biblio_keywords) ."\r\n";
    if (!empty($pub->biblio_call_number))
      $doc .= "%L ". trim($pub->biblio_call_number) ."\r\n";
    if (!empty($pub->biblio_accession_number))
      $doc .= "%M ". trim($pub->biblio_accession_number) ."\r\n";
    if (!empty($pub->biblio_issue))
      $doc .= "%N ". trim($pub->biblio_issue) ."\r\n";
    if (!empty($pub->biblio_pages))
      $doc .= "%P ". trim($pub->biblio_pages) ."\r\n";
    if (!empty($pub->biblio_tertiary_title))
      $doc .= "%S ". trim($pub->biblio_tertiary_title) ."\r\n";
    if (!empty($pub->biblio_url))
      $doc .= "%U ". trim($pub->biblio_url) ."\r\n";
    if (!empty($pub->biblio_volume))
      $doc .= "%V ". trim($pub->biblio_volume) ."\r\n";
    $abst = "";
    if (!empty($pub->biblio_abst_e))
      $abst .= trim($pub->biblio_abst_e);
    if (!empty($pub->biblio_abst_f))
      $abst .= trim($pub->biblio_abst_f);
    if ($abst) {
      $search = array("/\r/", "/\n/");
      $replace = " ";
      $abst = preg_replace($search, $replace, $abst);
      $doc .= "%X ". $abst ."\r\n";
    }
    if (!empty($pub->biblio_notes))
      $doc .= "%Z ". trim($pub->biblio_notes) ."\r\n";
    if (!empty($pub->biblio_edition))
      $doc .= "%7 ". trim($pub->biblio_edition) ."\r\n";
    if (!empty($pub->biblio_date))
      $doc .= "%8 ". trim($pub->biblio_date) ."\r\n";
    if (!empty($pub->biblio_type_of_work))
      $doc .= "%9 ". trim($pub->biblio_type_of_work) ."\r\n";
    if (!empty($pub->biblio_isbn))
      $doc .= "%@ ". trim($pub->biblio_isbn) ."\r\n";

    $doc .= "\r\n";
  }

  return $doc;
}
function biblio_csv_export_2($result, $bfields) {
  //  $query_biblio_fields = 'SELECT name, title FROM {biblio_fields}';
  //  $res_biblio_fields = db_query($query_biblio_fields);
  //  while ($rec = db_fetch_object($res_biblio_fields)){
  //    $bfields[$rec->name] = $rec->title;
  //  }
  $bfields = biblio_get_db_fields('all');
  $query_biblio_types = 'SELECT tid, name FROM {biblio_types}';
  $res_biblio_types = db_query($query_biblio_types);
  while ($rec = db_fetch_object($res_biblio_types)) {
    $btypes[$rec->tid] = $rec->name;
  }
  switch (variable_get('biblio_csv_field_sep', 'tab')) {
    case 'tab' :
      $filedsep = "\t";
      break;
    case 'comma' :
      $filedsep = ',';
      break;
  }
  switch (variable_get('biblio_csv_text_sep', 'dquote')) {
    case 'dquote' :
      $textsep = '"';
      break;
    case 'quote' :
      $textsep = '\'';
      break;
  }
  $label = (variable_get('biblio_csv_col_head', 'label') == 'label' ? 1 : 0); // or 'col_name'
  $linebreak = variable_get('biblio_linebreak_exp', 1);
  while ($rec = db_fetch_object($result)) {
    $node_id = $rec->nid;
    $node_array[$node_id]['type'] = $btypes[$rec->biblio_type]; // there is no "label" for "type"
    $col_array['type'] = 'Type';
    foreach (array_keys($bfields) as $fieldname) {
      if (!empty ($rec-> $fieldname) && !in_array($fieldname, array (
          'biblio_citekey',
          'biblio_coins'
        ))) {
        $col_array[$fieldname] = $bfields[$fieldname]; // mark field as in use
        $text = strtr($rec-> $fieldname, $textsep, "$textsep$textsep");
        if ($linebreak) {
          $text = strtr($text, ';', "\n");
        }
        $node_array[$node_id][$fieldname] = trim($text);
      }
    }
  } //end while
  if ($label) { // head line containing column names
    $csv = $textsep . join("$textsep$filedsep$textsep", array_values($col_array)) ."$textsep\n";
  } else { // original DB field names
    $csv = $textsep . join("$textsep$filedsep$textsep", array_keys($col_array)) ."$textsep\n";
  }
  // Enclosing text in "<text>" is neccessary to enshure
  // multi line fields (like author) are handled correctly.
  // Therefore existing " must be excaped before.
  $csv = '"'. join("\"\t\"", array_keys($col_array)) ."\"\n";
  foreach ($node_array as $line_array) {
    $csv_line = '';
    foreach (array_keys($col_array) as $col) {
      $csv_line .= "$filedsep$textsep". $line_array[$col] . $textsep;
    }
    $csv .= substr($csv_line, 1) ."\n"; // cut off leading fieldsep and append EOL
  }
  drupal_set_header('Content-Type: text/plain; charset=utf-8');
  drupal_set_header('Content-Disposition: attachment; filename=biblio_export.csv');
  return $csv;
}
//function _biblio_cck_join($biblio_fields = array()) {		// works not with php4
function _biblio_cck_join(& $biblio_fields) {
  $cck_join = '';
  $biblio_fields['nid'] = 'Node-ID'; // identify records for update operations
  $query_cck_fields = "SELECT field_name, label from {node_field_instance} where type_name='biblio' and not (widget_type='image')";
  $res_cck_fields = db_query($query_cck_fields);
  while ($rec = db_fetch_object($res_cck_fields)) {
    $cck_table = 'content_'. $rec->field_name;
    $cck_field = $rec->field_name .'_value';
    $biblio_fields[$cck_field] = $rec->label;
    $cck_join .= ' left join {'. $cck_table .'} on b.vid='. $cck_table .'.vid';
  }
  return $cck_join;
}

function biblio_backup() {

  $csv_function = (!function_exists('fputcsv')) ? 'biblio_fputcsv' : 'fputcsv';
  $count_sql = "SELECT COUNT(*)
                FROM {biblio} b, {node} n, {node_revisions} nr
                WHERE b.vid = n.vid and nr.vid = n.vid;";
  $field_type_sql = "SELECT *  FROM {biblio_field_type} ";
  $field_type_data_sql = "SELECT *  FROM {biblio_field_type_data} ";
  $field_fields_sql = "SELECT *  FROM {biblio_fields} ";
  $types_sql = "SELECT *  FROM {biblio_types} ";
  $sql = "SELECT b.*,
          n.type, n.language, n.title, n.uid, n.status, n.created,
          n.changed, n.comment, n.promote, n.moderate, n.sticky,
          n.tnid, n.translate,
          nr.title, nr.body, nr.teaser, nr.log, nr.timestamp, nr.format
          FROM {biblio} b, {node} n, {node_revisions} nr
          WHERE b.vid = n.vid and nr.vid = n.vid;";

  $biblio_count = db_result(db_query($count_sql));
  if ($biblio_count) {
    drupal_set_header('Content-Type: text/plain; charset=utf-8');
    drupal_set_header('Content-Disposition: attachment; filename=Biblio-export.csv');
    $biblio_nodes = db_query($sql);
    while ($node = db_fetch_array($biblio_nodes)) {
      $results[] = $node;
    }
    print biblio_csv_export($results);
    unset($results);
    $result = db_query($field_type_data_sql, 'biblio_field_type_data.csv');
    while ($data = db_fetch_array($result)) {
      $results[] = $data;
    }
    print biblio_csv_export($results);
    unset($results);
    $result = db_query($field_fields_sql, 'biblio_fields.csv');
    while ($data = db_fetch_array($result)) {
      $results[] = $data;
    }
    print biblio_csv_export($results);
    unset($results);
    $result = db_query($types_sql, 'biblio_types.csv');
    while ($data = db_fetch_array($result)) {
      $results[] = $data;
    }
    print biblio_csv_export($results);
    unset($results);
    $result = db_query($field_type_sql, 'biblio_field_type.csv');
    while ($data = db_fetch_array($result)) {
      $results[] = $data;
    }
    print biblio_csv_export($results);
  }
}

function biblio_restore(& $csv_content, $mode = 'create') {

}

function biblio_csv_export($results) {
  $csv = '';
  if (!is_array($results)) {
    $result_array[] = (array) $results;
  } else {
    $result_array = $results;
  }
  $fieldnames = null;
  foreach ((array)$result_array as $rec) {
    if (empty($fieldnames)) {
      $fieldnames = array_keys($rec);
      $csv .= biblio_strcsv($fieldnames);
    }
    $csv .= biblio_strcsv($rec);
  }
  return($csv);
}

function biblio_strcsv($fields = array(), $delimiter = ',', $enclosure = '"') {
  $str = '';
  $escape_char = '\\';
  foreach ($fields as $value) {
    if (strpos($value, $delimiter) !== false || strpos($value, $enclosure) !== false || strpos($value, "\n") !== false || strpos($value, "\r") !== false || strpos($value, "\t") !== false || strpos($value, ' ') !== false) {
      $str2 = $enclosure;
      $escaped = 0;
      $len = strlen($value);
      for ($i = 0; $i < $len; $i++) {
        if ($value[$i] == $escape_char) {
          $escaped = 1;
        } else
          if (!$escaped && $value[$i] == $enclosure) {
            $str2 .= $enclosure;
          } else {
            $escaped = 0;
          }
        $str2 .= $value[$i];
      }
      $str2 .= $enclosure;
      $str .= $str2 . $delimiter;
    } else {
      $str .= $value . $delimiter;
    }
  }
  $str = substr($str, 0, -1);
  $str .= "\n";
  return $str;
}