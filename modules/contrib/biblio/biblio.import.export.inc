<?PHP

// $Id: biblio.import.export.inc,v 1.7.2.68 2009/09/29 01:34:42 rjerome Exp $
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
    '#default_value' => $my_uid,
    '#disabled' => (user_access('administer biblio')) ?  FALSE : TRUE
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
    $form['biblio_import_file'] = array (
      '#type' => 'file',
      '#title' => t('Import file'),
      '#default_value' => '',
      '#size' => 60
    );
    $form['filetype'] = array (
      '#type' => 'select',
      '#title' => t('File Type'),
      '#default_value' => 0,
      '#options' => array (
        0     => t('Select type'),
        'bib'    => t('BibTex'),
        'tagged' => t('EndNote Tagged'),
        'xml'    => t('EndNote 7 XML (and previous versions)'),
        'xml8'   => t('EndNote 8 XML (and newer versions)'),
        'marc'   => t('MARC'),
        'ris'    => t('RIS'),
        )
    );
    $form['batch_process'] = array (
      '#type' => 'checkbox',
      '#title' => t('Batch Process'),
      '#default_value' => 1,
      '#description' => t('You should use batch processing if your import file contains more than about 20 records, or if you are experiencing script timeouts during import'),
    );
    $form ['userid'] = _biblio_admin_build_user_select($user->uid);
    // Get the vocabularies  attached to the biblio node type ...
    $vocabularies = module_invoke('taxonomy', 'get_vocabularies', 'biblio');
    // ... and print a form to select the terms in each of them
    $form['import_taxonomy'] = array (
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Taxonomy Settings'),
      '#description' => t('Typically you don\'t have to do anything here, however if you wish, you may select terms to be assigned to imported records. This effectively adds a keyword to all entries being imported.'));
    if (count($vocabularies)) {
      if (variable_get('biblio_keyword_freetagging', 0)) {
        $freetag_vocab = $vocabularies[variable_get('biblio_keyword_vocabulary', 0)];
        unset($vocabularies[variable_get('biblio_keyword_vocabulary', 0)]);
        $msg = t('<b>NOTE:</b> Keyword "free tagging" is turned on, consequently all incomming keywords will be added to the <b>@name</b> vocabulary as specified in the "Keyword" section of the !url page.', array ('@name' => $freetag_vocab->name, '!url' => l(t('admin/settings/biblio'), 'admin/settings/biblio')));
      } else {
        $msg = t('<b>NOTE:</b> Keyword "free tagging" is turned off, consequently keywords will <b>NOT</b> be added to the vocabulary as specified in the Taxonomy section of the !url page.', array ('!url' => l(t('admin/settings/biblio'), 'admin/settings/biblio')));
      }
      $i = 0;
      foreach ($vocabularies as $vocabulary) {
        $form['import_taxonomy']['vocabulary'. $i] = module_invoke('taxonomy', 'form', $vocabulary->vid, 0);
        $form['import_taxonomy']['vocabulary'. $i]['#weight'] = $vocabulary->weight;
        $form['import_taxonomy']['vocabulary'. $i++]['#description'] = t("Select taxonomy term to be assigned to imported entries");
      }
      $form['import_taxonomy']['copy_to_biblio'] = array(
        '#type' => 'checkbox',
        '#title' => t('Copy these terms to the biblio keyword database'),
        '#return_value' => 1,
        '#default_value' =>  variable_get('biblio_copy_taxo_terms_to_keywords', 0),
        '#description' => t('If this option is selected, the selected taxonomy terms will be copied to the '.variable_get('biblio_base_title', 'Biblio').' keyword database and be displayed as keywords (as well as taxonomy terms) for this entry.')
      );

    } else {
      if (module_exists('taxonomy')){
        $vocab_msg = t('There are currently no vocabularies assigned to the biblio node type, please go the the !url page to fix this', array ('!url' => l(t('admin/content/taxonomy'), 'admin/content/taxonomy')));
      }else{
        $vocab_msg = '<div class="admin-dependencies">'. t('Depends on') .': '. t('Taxonomy') .' (<span class="admin-disabled">'. t('disabled') .'</span>)</div>';

      }
      $form['import_taxonomy']['vocabulary_message'] = array (
        '#value' => '<p><div>'. $vocab_msg .'</div></p>'
      );
    }
    $form['import_taxonomy']['freetagging_information'] = array (
      '#value' => '<p><div>'. $msg .'</div></p>'
    );
    $form['button'] = array ('#type' => 'submit', '#value' => t('Import'));
    return $form;
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
  if ($error = $_FILES['files']['error']['biblio_import_file']) {
    switch ($error){
      case 1: form_set_error('', t("The uploaded file exceeds the upload_max_filesize directive in php.ini."));
      break;
      case 2: form_set_error('', t("The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form."));
      break;
      case 3: form_set_error('', t("The uploaded file was only partially uploaded."));
      break;
      case 4: form_set_error('', t("No file was uploaded."));
      break;
      case 6: form_set_error('', t("Missing a temporary folder."));
      break;
      case 7: form_set_error('', t("Failed to write file to disk."));
      break;
      case 8: form_set_error('', t("File upload stopped by extension."));
    }
  }

  if ($op == t('Import') && $filetype === 0) {
    form_set_error('', t("You must select a file type"));
  }
}
/**
 * Implementation of hook_submit() for the biblio_import_form.
 */
function biblio_import_form_submit($form, & $form_state) {
  global  $batch_proc;
  global  $session_id;
  if ($form_state['values']['op'] == t('Import') && isset ($form_state['values']['filetype'])) {
    if ($import_file = file_save_upload('biblio_import_file')) {
      if($form_state['values']['batch_process'] == 1) $batch_proc = 1;  // we will use batch import for larger files.

      drupal_set_message(t("@file was uploaded", array ('@file' => $import_file->filename)), 'status');
      // Concatenate all the terms of the different vocabularies
      // in a single array to be sent to biblio_import
      $terms = array ();
      foreach (array_keys($form_state['values']) as $key) {
        if (preg_match('/(vocabulary[0-9]+)/', $key)) {
          if (!empty($form_state['values'][$key])){
            if (is_array($form_state['values'][$key])) {
              $terms[] = $form_state['values'][$key];
            }
            else {
              $terms[] = array($form_state['values'][$key]);
            }
          }
        }
        if ($key == 'copy_to_biblio') $terms['copy_to_biblio'] = $form_state['values'][$key];
      }

      // Added the $terms argument
      // the array of terms to be attached to the node(s)
      $userid = (isset ($form_state['values']['userid'])) ? $form_state['values']['userid'] : 1;
      $filetype = $form_state['values']['filetype'];
      if ($batch_proc) {
        $session_id = microtime();
        $batch_op = array(
          'title' => t('Importing '.$import_file->filename),
          'operations' => array(
            array('biblio_import', array($import_file, $filetype, $userid, $terms, $batch_proc, $session_id)),
            array('biblio_import_batch_operations', array($session_id, $userid, $terms)),
           ),
          'progressive' => TRUE,
          'finished' => 'biblio_import_batch_finished',
          'init_message' => t('Parsing file...'),
          'progress_message' => t('Saving nodes...'),
          'file' => './'. drupal_get_path('module', 'biblio') .'/biblio.import.export.inc'
        );
        batch_set($batch_op);

        $base = variable_get('biblio_base', 'biblio');
        batch_process("$base/import");

      }else{ //not batch processing the file
        $session_id = microtime();
        $dummy = array();
        $content = biblio_import($import_file, $filetype, $userid, $terms, $batch_proc, $session_id, $dummy);
      }
      file_delete($import_file->filepath);
    } else {
      drupal_set_message(t("File was NOT successfully uploaded"), 'error');
    }
  }
}
function biblio_import_batch_operations($session_id, $userid, $terms, &$context) {
  if (empty($context['sandbox'])) {
    // Initiate multistep processing.
    $context['results']['session_id'] = $session_id;
    $context['results']['userid'] = $userid;
    $context['results']['terms'] = $terms;
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_id'] = 0;
    $context['sandbox']['max'] = db_result(db_query("SELECT COUNT(DISTINCT(id)) FROM {biblio_import_cache} WHERE session_id = '%s'", $session_id));
  }

  // Process the next 20 nodes.
  $limit = 5;
  $result = db_query_range("SELECT id, data FROM {biblio_import_cache} WHERE id > %d AND session_id = '%s' ORDER BY id ASC", $context['sandbox']['current_id'], $session_id, 0, $limit);
  while ($row = db_fetch_array($result)) {
    $node = unserialize($row['data']);
    node_save($node);
    $context['results']['nids'][] = $node->nid;
    $context['sandbox']['progress']++;
    $context['sandbox']['current_id'] = $row['id'];
  }

  // Multistep processing : report progress.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}
function biblio_import_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results['nids']), 'One node saved.', '@count nodes saved.');
  }
  else {
    $message = t('Import finished with an error.');
  }
  drupal_set_message($message);
  foreach ($results['nids'] as $node_id) {
    if (count($results['terms'])) module_invoke('taxonomy', 'node_save', $node_id, $results['terms']);
    db_query('UPDATE {node} SET uid = %d WHERE nid = %d', $results['userid'], $node_id);
    db_query('UPDATE {node_revisions} SET uid = %d WHERE nid = %d', $results['userid'], $node_id);
  }
  //clean up import cache...
  db_query("DELETE FROM {biblio_import_cache} WHERE session_id = '%s'",$results['session_id']);

}

function biblio_import_from_url($URL) {
  $handle = fopen($URL, "r"); // fetch data from URL in read mode
  $data = "";
  if ($handle)
  {
    while (!feof($handle))
    {
      $data .= fread($handle, 4096); // read data in chunks
    }
    fclose($handle);
  }
  else
  {
    $errorMessage = t("Error occurred: Failed to open ") . $URL; // network error
    drupal_set_message($errorMessage, 'error');
  }

  return $data;
}

function biblio_export_form() {
  $form['pot'] = array (
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('POT Export'),
    '#description' => t('Here you may export a ".pot" file which contains the titles and hints from the database which are not normally captured by translation extractors)')
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
function biblio_import($import_file, $type, $userid = 1, $terms = NULL, $batch = FALSE, $id = NULL, &$context ) {

  global $user, $batch_proc, $session_id, $biblio_uid;
  $batch_proc = $batch;
  $session_id = $id;
  $biblio_uid = $userid;
  $parsed = 0;
  if(isset($context['message']))  $context['message'] = t('Parsing file');
  switch ($type) {
    case 'tagged' : // EndNote Tagged
      module_load_include('inc', 'biblio', 'tagged_parser');
      $node_ids = _endnote_tagged_import($import_file, $terms, $batch_proc, $session_id);
      break;
    case 'ris' : // RIS
      module_load_include('inc', 'biblio', 'ris_parser');
      $node_ids = _ris_tagged_import($import_file, $terms, $batch_proc, $session_id);
      break;
    case 'xml' : // EndNote 7 XML
      $node_ids = biblio_endnote_XML_import($import_file, $terms, $batch_proc, $session_id, 7);
      break;
    case 'xml8' : // EndNote 8+ XML
      $node_ids = biblio_endnote_XML_import($import_file, $terms, $batch_proc, $session_id, 8);
      break;
    case 'bib' : // BibTex
      $node_ids = biblio_bibtex_import($import_file, $terms, $batch_proc, $session_id);
      break;
    case 'marc' : // MARC
      $node_ids = biblio_marc_import($import_file, $terms, $batch_proc, $session_id);
      break;
      case 'csv' : // comma separated variable file
      //        $file_content = @ file_get_contents($import_file->filepath);
      //        $parsed = biblio_csv_import($file_content, $node_template, $node_array);
      break;
    case 'biblio_backup' : // a complete backup of all biblio information
      $file_content = @ file_get_contents($import_file->filepath);
      $parsed = biblio_restore($file_content, $node_template, $node_array);
      break;
  }
  if (!empty($node_ids) && !$batch ) {
    if (count($node_ids)) {
      db_query('UPDATE {node} SET uid = %d WHERE nid IN(%s)', $userid, implode(',', $node_ids));
      db_query('UPDATE {node_revisions} SET uid = %d WHERE nid IN(%s)', $userid, implode(',', $node_ids));
    }

    return $node_ids;
  }
  elseif ($parsed && !$save) {
    return $node;
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
function biblio_export($format = "tagged", $nid = null, $popup = false, $version = 8) {
  module_load_include('inc', 'biblio', 'endnote8_export');
  module_load_include('inc', 'biblio', 'biblio.contributors');
  module_load_include('inc', 'biblio', 'biblio.keywords');
  $params = array ();
  if ($nid === null && isset ($_SESSION['last_biblio_query']) && !empty ($_SESSION['last_biblio_query'])) {
    $query = $_SESSION['last_biblio_query'];
    $params = $_SESSION['last_biblio_query_terms'];
  }
  elseif (!empty ($nid)) {
    $query = db_rewrite_sql("SELECT DISTINCT(n.nid) FROM {node} n  WHERE n.nid=%d ");
    $params[] = $nid;
  } else {
    return;
  }
  $result = db_query($query, $params);
  $count = 0;
  while ($node = db_fetch_object($result)) {
    $node = node_load($node->nid, FALSE, TRUE);
//    $node->biblio_contributors = biblio_load_contributors($node->vid);
//    $node->biblio_keywords = biblio_load_keywords($node->vid);
//    if (module_exists("upload")) $node->files = upload_load($node);
    $count++;
    set_time_limit(30);
    switch ($format) {
      case "tagged" :
        if (!$popup && $count == 1) {
          drupal_set_header('Content-type:  application/x-endnote-refer');
          drupal_set_header('Content-Disposition:  filename="Drupal-Biblio.enw"');
        }
        if (!$popup) {
          print biblio_endnote_tagged_export($node);
        }
        else {
          $popup_data .= biblio_endnote_tagged_export($node);
        }
        break;
      case "xml" :
        if ($count == 1) {
          drupal_set_header('Content-type: application/xml; charset=utf-8');
          drupal_set_header('Content-Disposition: attachment; filename="Biblio-EndNote'. $version .'.xml"');
          print _endnote8_XML_export('', 'begin');
        }    
        print _endnote8_XML_export($node);
        break;
      case "bibtex" :
        if (!$popup && $count == 1) {
          drupal_set_header('Content-type: 	application/text; charset=utf-8');
          drupal_set_header('Content-Disposition:  filename="Biblio-Bibtex.bib"');
        }
        if (!$popup) {
          print biblio_bibtex_export($node);
        }
        else{
          $popup_data .=  biblio_bibtex_export($node);
        }
        break;
      case "csv" :
        drupal_set_header('Content-Type: application/text; charset=utf-8');
        drupal_set_header('Content-Disposition: attachment; filename=Biblio-export.csv');
        print biblio_csv_export($node);
        break;
    }
  }
  
  if ($format == 'xml' && $count > 0) print _endnote8_XML_export('', 'end');
  if ($popup && !empty($popup_data)) return '<pre>' . $popup_data . '</pre>';

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
function biblio_bibtex_import($file, $terms = array(), $batch = FALSE, $session_id = NULL, $save = TRUE, $string = FALSE) {
  $nids = array();
  module_load_include('php', 'biblio', 'bibtexParse/PARSEENTRIES');
  $bibtex = new PARSEENTRIES();
  if ($string) {
    $bibtex->loadBibtexString($file);
  }
  else {
    $bibtex->openBib($file->filepath);
  }
  $bibtex->extractEntries();
  if ($bibtex->count) {
    $nids = $bibtex->bib2node($terms, $batch, $session_id, $save);
  }
  return $nids;
}
function biblio_marc_import($file, $terms, $batch, $session_id) {
  $nids = array();
  module_load_include('php', 'biblio', 'marcParse/php-marc');
  $marcfile = new File($file->filepath);
  while ($record = $marcfile->next() ) {
    $node=array();
    foreach($record->fields() as $fields) {
      foreach ($fields as $field){
        switch ($field->tagno) {
          case '008':
            $data = $field->data();
            $node['biblio_year'] = substr($data,7,4);
            $node['biblio_lang'] = substr($data,35,3);
            break;
          case '020':
            $node['biblio_isbn'] = $field->subfield('a');
            break;
          case '022':
            $node['biblio_issn'] = $field->subfield('a');
            break;
          case '024':
            $node['biblio_other_number'] = $field->subfield('a');
            break;
          case '050': //LIBRARY OF CONGRESS CALL NUMBER
          case '055': //CLASSIFICATION NUMBERS ASSIGNED IN CANADA
          case '060': //NATIONAL LIBRARY OF MEDICINE CALL NUMBER
            $node['biblio_call_number'] = $field->subfield('a');
            break;
          case '130':
            $node['title'] = str_replace(' /', '', $field->subfield('a'));
            break;
          case '210':
            $node['biblio_short_title'] = str_replace(' /', '', $field->subfield('a'));
            break;
          case '245':
            $node['title'] = str_replace(' /', '', $field->subfield('a')).' '.$field->subfield('b');
            break;
          case '250':
            $node['biblio_edition'] = $field->subfield('a');
            break;
          case '260':
            $node['biblio_place_published'] = str_replace(' :', '', $field->subfield('a'));
            $node['biblio_publisher'] = $field->subfield('b');
            $node['biblio_date'] = $field->subfield('c');
            break;
          case '300':
            $node['biblio_pages'] = $field->subfield('a');
            break;
          case '490':
            $node['biblio_volumne'] = $field->subfield('v');
            break;
          case '100':
          case '700':
            $node['biblio_contributors'][1][] = array(
              'name' => $field->subfield('a'),
              'auth_type' => 1
            );
            break;
          case '110':
          case '710':
            $node['biblio_contributors'][5][] = array(
              'name' => $field->subfield('a'),
              'auth_type' => 5
            );
            break;
        }
      }
    }
    if (!empty($node)) {
      if (!empty($terms)) {
        if (!isset($node['taxonomy'])) $node['taxonomy'] = array();
        $node['taxonomy'] = array_merge($terms,$node['taxonomy']);
      }
      $nids[] = biblio_save_node($node, $batch, $session_id);
    }

  }
  return $nids;
}
/**
 * Export data in bibtex format.
 *
 * @param $result
 *   a database result set pointer
 * @return
 *   none
 */
function biblio_bibtex_export($node) {
  $bibtex = '';
  $type = "article";
  $journal = $series = $booktitle = $school = $organization = $institution = null;
  $type = _bibtex_type_map($node->biblio_type);
  switch ($node->biblio_type) {
    case 100 :
      $series = $node->biblio_secondary_title;
      $organization = $node->biblio_publisher;
      break;
    case 101 :
    case 103 :
      $booktitle = $node->biblio_secondary_title;
      $organization = $node->biblio_publisher;
      $series = $node->biblio_tertiary_title;
      break;
    case 108 :
      $school = $node->biblio_publisher;
      $node->biblio_publisher = null;
      if (stripos($node->biblio_type_of_work, 'masters')) {
        $type = "mastersthesis";
      }
      break;
    case 109 :
      $institution  = $node->biblio_publisher;
      $node->biblio_publisher = null;
      break;
    case 102 :
    default:
      $journal = $node->biblio_secondary_title;
      break;
  }
  $bibtex .= '@'. $type .' {';
  $bibtex .= ($node->biblio_citekey) ? $node->biblio_citekey  : "";
  $bibtex .= _bibtex_format_entry('title', $node->title);
  $bibtex .= _bibtex_format_entry('journal', $journal);
  $bibtex .= _bibtex_format_entry('booktitle', $booktitle);
  $bibtex .= _bibtex_format_entry('series', $series);
  $bibtex .= _bibtex_format_entry('volume', $node->biblio_volume);
  $bibtex .= _bibtex_format_entry('number', $node->biblio_number);
  $bibtex .= _bibtex_format_entry('year', $node->biblio_year);
  $bibtex .= _bibtex_format_entry('note', $node->biblio_notes);
  $bibtex .= _bibtex_format_entry('month', $node->biblio_date);
  $bibtex .= _bibtex_format_entry('pages', $node->biblio_pages);
  $bibtex .= _bibtex_format_entry('publisher', $node->biblio_publisher);
  $bibtex .= _bibtex_format_entry('school', $school);
  $bibtex .= _bibtex_format_entry('organization', $organization);
  $bibtex .= _bibtex_format_entry('institution', $institution);
  $bibtex .= _bibtex_format_entry('type', $node->biblio_type_of_work);
  $bibtex .= _bibtex_format_entry('edition', $node->biblio_edition);
  $bibtex .= _bibtex_format_entry('chapter', $node->biblio_section);
  $bibtex .= _bibtex_format_entry('address', $node->biblio_place_published);
  $bibtex .= _bibtex_format_entry('abstract', $node->biblio_abst_e);

  $kw_array = array();
  if (!empty($node->terms)){
    foreach($node->terms as $term){
      $kw_array[] = $term->name;
    }
  }
  if (!empty($node->biblio_keywords)) {
    foreach($node->biblio_keywords as $term){
      $kw_array[] = $term;
    }
  }
  if (!empty($kw_array)){
    $kw_array = array_unique($kw_array);
    $bibtex .= _bibtex_format_entry('keywords', implode(', ', $kw_array));
  }
  $bibtex .= _bibtex_format_entry('isbn', $node->biblio_isbn);
  $bibtex .= _bibtex_format_entry('issn', $node->biblio_issn);
  $bibtex .= _bibtex_format_entry('doi', $node->biblio_doi);
  $bibtex .= _bibtex_format_entry('url', $node->biblio_url);
  if (!empty ($node->files) && count($node->files) && user_access('view uploaded files')) {
    foreach($node->files as $file) {
      $attachments[] = file_create_url($file->filepath);
    }
    $bibtex .= _bibtex_format_entry('attachments', implode(' , ', $attachments));
  }

  $a = $e = array();
  foreach ((array)$node->biblio_contributors[1] as $auth) $a[] = trim($auth['name']);
  foreach ((array)$node->biblio_contributors[2] as $auth) $e[] = trim($auth['name']);
  $a = implode(' and ', $a);
  $e = implode(' and ', $e);
  if (!empty ($a)) $bibtex .= _bibtex_format_entry('author', $a);
  if (!empty ($e)) $bibtex .= _bibtex_format_entry('editor', $e);
  $bibtex .= "\n}\n";


  //now convert any special characters to the latex equivelents...
  module_load_include('php', 'biblio', 'bibtexParse/PARSEENTRIES');
  include(drupal_get_path('module', 'biblio') . '/bibtexParse/transtab_unicode_bibtex.inc.php');
  $converter = new PARSEENTRIES();
  $bibtex = $converter->searchReplaceText($transtab_unicode_bibtex, $bibtex, false);

  return $bibtex;
}
function _bibtex_format_entry($key, $value) {
  return !empty($value) ? ",\n\t$key = {".$value."}" : '';
}

function _bibtex_type_map($bibliotype) {
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('bibtex');
  }
  return ($type = array_search($bibliotype, $map)) ? $type : 'article';
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
    foreach ($node_array as $imp_node) {
      $node_ids[] = biblio_save_node($imp_node);
    }
  }
/*  if ($dup_count)
    drupal_set_message(t("Detected @dupcount duplicate node(s) when importing", array ('@dupcount' => $dup_count)), 'error');

  drupal_set_message(t("Succesfully imported @count entries.", array ('@count' => count($node_ids))), 'status');
*/
  return $node_ids;
}
function biblio_save_node($node, $batch = FALSE, $session_id = NULL, $save_node = TRUE) {
  $options = variable_get('node_options_biblio', array ('status'));
  if (module_exists('i18n') && variable_get('i18n_node_biblio', 0) && variable_get('language_content_type_biblio', 0) ){
    $node['language'] = module_invoke('i18n', 'default_language');
  }
  $node_template = array (
      'type' => 'biblio',
      'comment' => variable_get('comment_biblio', 0),
      'promote' => in_array('promote', $options),
      'moderate' => in_array('moderate', $options),
      'sticky' => in_array('sticky', $options),
      'format' => 0,
      'status' => in_array('status', $options),
  );

  $node = (object) array_merge($node, $node_template);
  if(!isset($node->biblio_type)) $node->biblio_type = 129; // default to misc if not set.
  if ($batch && $session_id){ // we are batch processing some import data
    $node = serialize($node);
    db_query("INSERT INTO {biblio_import_cache} (session_id, data) VALUES ('%s', %b)", $session_id, $node);
    return;
  }
  elseif ($save_node) { // $save_node = TRUE, the normal save path
    node_save($node);
    return (isset($node->nid)) ? $node->nid : 0;
  }
  else { // $save_node = FALSE, primarily used to parse data and return it to the input form
    return (array)$node;
  }
}

function biblio_crossref_xml_import($doi, $terms = array(), $batch = FALSE, $session_id = NULL, $save = FALSE) {
  global $user, $node, $save_node, $nids;
  if (!isset($user->biblio_crossref_pid) || empty($user->biblio_crossref_pid) ) return FALSE;
  $save_node = $save;
  $nids = array();
  $url = 'http://www.crossref.org/openurl/?pid='. $user->biblio_crossref_pid .'&noredirect=true&format=unixref&id=doi%3A'. $doi;
  if (!($fp = fopen($url, "r"))) {
    drupal_set_message(t('Could not open crossref.org for XML input'),'error');
    return;
  }
  $xml = fread($fp, 2048);
  $xml_parser = drupal_xml_parser_create($xml);
  // use case-folding so we are sure to find the tag in
  xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, false);
  xml_parser_set_option($xml_parser, XML_OPTION_SKIP_WHITE, true);

  module_load_include('inc', 'biblio', 'crossref_unixref_parser');
  xml_set_element_handler($xml_parser, 'unixref_startElement', 'unixref_endElement');
  xml_set_character_data_handler($xml_parser, 'unixref_characterData');

  xml_parse($xml_parser, $xml);
  while ($xml = fread($fp, 2048)){
    set_time_limit(30);
    if(!xml_parse($xml_parser, $xml, feof($fp))){
      drupal_set_message(sprintf("XML error: %s at line %d",
      xml_error_string(xml_get_error_code($xml_parser)),
      xml_get_current_line_number($xml_parser)),'error');
    }
  }
  xml_parser_free($xml_parser);
  fclose($fp);
  return (!empty($nids)) ? $nids : array();

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
function biblio_endnote_XML_import($xml_file, $taxo_terms = array(), $batch_proc = FALSE, $session_id = NULL, $ver = 8) {
  global $user, $records, $rec_count, $node,  $terms, $batch_proc, $nids, $session_id;
  $terms = $taxo_terms;
  $nids = array();
  if (!($fp = fopen($xml_file->filepath, "r"))) {
    drupal_set_message("could not open XML input",'error');
    return;
  }
  $data = fread($fp, 2048);
  $xml_parser = drupal_xml_parser_create($data);
  // use case-folding so we are sure to find the tag in
  xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, false);
  xml_parser_set_option($xml_parser, XML_OPTION_SKIP_WHITE, true);

  module_load_include('inc', 'biblio', 'endnote'.$ver.'_parser');
  xml_set_element_handler($xml_parser, 'en'.$ver.'_startElement', 'en'.$ver.'_endElement');
  xml_set_character_data_handler($xml_parser, 'en'.$ver.'_characterData');

  xml_parse($xml_parser, $data, feof($fp));
  while ($data = fread($fp, 2048)){
   // $data = fread($fp, 2048);
    set_time_limit(30);
    if(!xml_parse($xml_parser, $data, feof($fp))){
      drupal_set_message(sprintf("XML error: %s at line %d",
      xml_error_string(xml_get_error_code($xml_parser)),
      xml_get_current_line_number($xml_parser)),'error');

    }
  }
  xml_parser_free($xml_parser);
  fclose($fp);
  return (!empty($nids)) ? $nids : array();
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
    module_load_include('inc', 'biblio', 'endnote8_export');
    $xml = _endnote8_XML_export($result);
  }
  elseif ($version == 7) {
    module_load_include('inc', 'biblio', 'endnote7_export');
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
function biblio_endnote_tagged_export($node) {

  $tagged = "";
  $tagged .= "%0 ". _endnote_tagged_type_map($node->biblio_type) ."\r\n";
  switch ($node->biblio_type) {
    case 100 :
    case 101 :
    case 103 :
    case 104 :
    case 105 :
    case 108 :
    case 119 :
      if (!empty($node->biblio_secondary_title))
      $tagged .= "%B ". trim($node->biblio_secondary_title) ."\r\n";
      break;
    case 102 :
      if (!empty($node->biblio_secondary_title))
      $tagged .= "%J ". trim($node->biblio_secondary_title) ."\r\n";
      break; // journal
  }
  if (isset($node->biblio_year) && $node->biblio_year < 9998)
  $tagged .= "%D ". trim($node->biblio_year) ."\r\n";
  if (!empty($node->title))
  $tagged .= "%T ". trim($node->title) ."\r\n";

  foreach ((array)$node->biblio_contributors[1] as $auth) {
    $tagged .= "%A " . trim($auth['name']) ."\r\n";
  }
  foreach ((array)$node->biblio_contributors[2] as $auth) {
    $tagged .= "%E " . trim($auth['name']) ."\r\n";
  }
  foreach ((array)$node->biblio_contributors[3] as $auth) {
    $tagged .= "%Y " . trim($auth['name']) ."\r\n";
  }

  if (!empty($node->biblio_place_published))
  $tagged .= "%C ". trim($node->biblio_place_published) ."\r\n";
  if (!empty($node->biblio_publisher)) { $tagged .= "%I ". trim($node->biblio_publisher) ."\r\n"; }

  $kw_array = array();
  if (!empty($node->terms)){
    foreach($node->terms as $term){
      $kw_array[] = $term->name;
    }
  }
  if (!empty($node->biblio_keywords)) {
    foreach($node->biblio_keywords as $term){
      $kw_array[] = $term;
    }
  }
  if (!empty($kw_array)){
    $kw_array = array_unique($kw_array);
    foreach($kw_array as $term){
      $tagged .= "%K ". trim($term) ."\r\n";
    }
  }
  if (!empty($node->biblio_call_number))
  $tagged .= "%L ". trim($node->biblio_call_number) ."\r\n";
  if (!empty($node->biblio_accession_number))
  $tagged .= "%M ". trim($node->biblio_accession_number) ."\r\n";
  if (!empty($node->biblio_issue))
  $tagged .= "%N ". trim($node->biblio_issue) ."\r\n";
  if (!empty($node->biblio_pages))
  $tagged .= "%P ". trim($node->biblio_pages) ."\r\n";
  if (!empty($node->biblio_doi))
  $tagged .= "%R ". trim($node->biblio_doi) ."\r\n";
  if (!empty($node->biblio_tertiary_title))
  $tagged .= "%S ". trim($node->biblio_tertiary_title) ."\r\n";
  if (!empty($node->biblio_url))
  $tagged .= "%U ". trim($node->biblio_url) ."\r\n";
  if (!empty($node->biblio_volume))
  $tagged .= "%V ". trim($node->biblio_volume) ."\r\n";
  $abst = "";
  if (!empty($node->biblio_abst_e))
  $abst .= trim($node->biblio_abst_e);
  if (!empty($node->biblio_abst_f))
  $abst .= trim($node->biblio_abst_f);
  if ($abst) {
    $search = array("/\r/", "/\n/");
    $replace = " ";
    $abst = preg_replace($search, $replace, $abst);
    $tagged .= "%X ". $abst ."\r\n";
  }
  if (!empty($node->biblio_notes))
  $tagged .= "%Z ". trim($node->biblio_notes) ."\r\n";
  if (!empty($node->biblio_edition))
  $tagged .= "%7 ". trim($node->biblio_edition) ."\r\n";
  if (!empty($node->biblio_date))
  $tagged .= "%8 ". trim($node->biblio_date) ."\r\n";
  if (!empty($node->biblio_type_of_work))
  $tagged .= "%9 ". trim($node->biblio_type_of_work) ."\r\n";
  if (!empty($node->biblio_isbn))
  $tagged .= "%@ ". trim($node->biblio_isbn) ."\r\n";
  if (!empty ($node->files) && count($node->files)  && user_access('view uploaded files')) {
    foreach($node->files as $file) {
      $tagged .= "%> ". file_create_url($file->filepath) . "\r\n"; // insert file here.
    }
  }
  $tagged .= "\r\n";
  return $tagged;
}
function _endnote_tagged_type_map($bibliotype) {
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('tagged');
  }
  return ($type = array_search($bibliotype, $map)) ? $type : 'Generic'; //return the biblio type or 129 (Misc) if type not found
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
function biblio_dump_db_data_for_pot() {
  $query = "SELECT name, description FROM {biblio_types} ";
  $result = db_query($query);
  $strings = array();
  while ($type = db_fetch_object($result)) {
    $strings[] = $type->name;
    if (!empty($type->description)) $strings[] = $type->description;
  }
  $query = "SELECT title, hint FROM {biblio_field_type_data} ";
  $result = db_query($query);
  while ($type_data = db_fetch_object($result)) {
    $strings[] = $type_data->title;
    if (!empty($type_data->hint)) $strings[] = $type_data->hint;
  }
  $query = "SELECT title, hint FROM {biblio_contributor_type_data} ";
  $result = db_query($query);
  while ($type_data = db_fetch_object($result)) {
    $strings[] = $type_data->title;
    if (!empty($type_data->hint)) $type_data->hint;
  }
  $strings = array_unique($strings);
  foreach ($strings as $string) {
    $output .= "t(\"$string\"\);\n";
  }

  drupal_set_header('Content-Type: text/plain; charset=utf-8');
  drupal_set_header('Content-Disposition: attachment; filename=biblio_db_values.pot');
  print $output;
}

function biblio_pubmed_query () {
  $query = ''; //your query term
    $dnum = 100; // total number of documents here it's set to 100
    $pids = ''; // PubMED record ID's from e-search initialize to NULL
    $term = 360; // time interval of when documents were published - this one is one year=360days
    //retreive PID's of all articles published withing past year that contain query term
    $esearch = "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=pubmed&term=$query&reldate=$term&datetype=edat&retmax=100&usehistory=y";
    $handle = fopen($esearch, "r");
    $rettype = "abstract"; //retreives abstract of the record, rather than full record
    $retmode = "xml";
    $utils = "http://www.ncbi.nlm.nih.gov/entrez/eutils";
    if (!$handle) {die();}
    //collect returned pubmed PID's
    while (!feof ($handle)) $pids .= fgets($handle, 4096);
    fclose($handle);
    //Get query string from eSearch
    preg_match("/(\w+)<\/QueryKey>/i",$pids,$match);
    $queryKey = $match[1];
    //get webenv
    preg_match("/(\S+)<\/WebEnv>/i",$pids,$match);
    $webEnv = $match[1];
    $retstart = 0;
    //fetch xml docs from PUBMED for returned PID's
    $efetch = "$utils/efetch.fcgi?rettype=$rettype&retmode=$retmode&retstart=$retstart&retmax=$dnum&db=pubmed&query_key=$queryKey&WebEnv=$webEnv&email=abc@xyz.com";
    $pids = '';
    $handle = fopen($efetch, "r");
    if(!$handle) { die(); }
    while (!feof ($handle)) $pids .= fgets($handle, 4096);
    fclose($handle);

}