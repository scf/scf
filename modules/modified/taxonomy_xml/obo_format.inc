<?php
// $Id$

/**
 * @file Include import routines for OBO Schema as used by
 * http://geneontology.org/GO.format.obo-1_2.shtml
 */


/**
 * Batch processing of a large file.
 * 
 * Implementation of taxonomy_xml_HOOK_batch_parse_file() callback.
 *
 * Basic approach to implementing Drupal 6's Batch API for processing a single
 * file: process 10 terms and then go back and read the file if necessary.
 *
 * LOL I forsee all the work I did to make the file object available won't be necessary because the batch api can keep the data in context...  full speed ahead anyway.
 */
function taxonomy_xml_obo_parse(&$data, $vid = 0) {
  $size = 1; // batch size
  $batch = array(
    'title' => t('Bloody hell'),
    'operations' => array(
      array('taxonomy_xml_obo_batch_import_terms', array($size, $data)),
      array('taxonomy_xml_obo_not_here'),  // another function to run at end
    ),
    'file' => drupal_get_path('module', 'taxonomy_xml') . '/obo_format.inc',
    'finished' => 'taxonomy_xml_obo_batch_finished',
  );
  batch_set($batch);
  batch_process();
}


/**
 * Batch callback operation: Import terms.
 *
 * @param $size
 *   Number of terms to import in each operation.
 * @param $data
 *   Data from file to be processed.
 * @param $c
 *   Batch context containing state information.  Passed by reference.
 */
function taxonomy_xml_obo_batch_import_terms($size, &$data, &$context) {
// echo "After one round, we're using (in bytes): ", memory_get_usage(),"\n<br>";
  // Initialize sandbox the first time through.
  if (!isset($context['sandbox']['lines'])) {
    $lines = taxonomy_xml_obo_prepare_data($data);
    $line_count = count($lines);
    $context['sandbox']['lines'] = $lines;
    $context['sandbox']['line_count_total'] = $line_count;
    $context['sandbox']['line_count_processed'] = 0;
    // used to run relationships after import of terms
    $context['sandbox']['terms'] = array();  // or use the static cache attempt? =& taxonomy_xml_current_terms(); 
    $context['message'] = t('We will import terms @size at a time.', array('@size' => $size));
  }
  // 
  $term_context = array();
  for ($i = 0; $i < $size; $i++) {    
// drupal_set_message('lines: <pre>'.var_export($context['sandbox']['lines'],TRUE).'</pre>');
    if ($term = taxonomy_xml_obo_import_term($context['sandbox']['lines'], $term_context)) {
// drupal_set_message('term: <pre>'.var_export($term,TRUE).'</pre>');
      $context['sandbox']['terms'][] = $term;
    }
// drupal_set_message('term_context: <pre>'.var_export($term_context,TRUE).'</pre>');
    $context['sandbox']['line_count_processed'] += $term_context['line_count'];
  }
// drupal_set_message('term: <pre>'.var_export($context,TRUE).'</pre>');
  if ($context['sandbox']['line_count_processed'] >= $context['sandbox']['line_count_total']) {
    $context['finished'] = 1;  // done!
  }
  else {
    $context['finished'] = $context['sandbox']['line_count_processed'] / $context['sandbox']['line_count_total'];
  }
//  watchdog('obo', 'Finished? ' . $context['finished']);

}



/**
 * Batch processing of a large file.
 * 
 * Implementation of taxonomy_xml_HOOK_batch_parse_file() callback.
 *
 * Basic approach to implementing Drupal 6's Batch API for processing a single
 * file: process 10 terms and then go back and read the file if necessary.
 *
 * LOL I forsee all the work I did to make the file object available won't be necessary because the batch api can keep the data in context...  full speed ahead anyway.
 */
function taxonomy_xml_obo_batch_parse_file(&$file, $vid = 0) {
  $size = 2; // batch size
  $batch = array(
    'title' => t('Bloody hell'),
    'operations' => array(
      array('taxonomy_xml_obo_file_batch_import_terms', array($size, $file)),
      array('taxonomy_xml_obo_not_here'),  // another function to run at end
    ),
    'file' => drupal_get_path('module', 'taxonomy_xml') . '/obo_format.inc',
    'finished' => 'taxonomy_xml_obo_batch_finished',
  );
  batch_set($batch);
  batch_process();
}

/**
 * Batch callback operation: Import terms.
 *
 * @param $size
 *   Number of terms to import in each operation.
 * @param $data
 *   Data from file to be processed.
 * @param $c
 *   Batch context containing state information.  Passed by reference.
 */
function taxonomy_xml_obo_file_batch_import_terms($size, &$file, &$context) {
  // get the file contents if don't have them already
// drupal_set_message('<pre>'.var_export($file,TRUE).'</pre>');  
//  static $lines;
        $fd = fopen($file->filepath, "rb");        if (!$fd) {          form_set_error('upload_file', t('Vocabulary import failed: file %filename cannot be read.', array('%filename' => $file->filename)));        }
        else {          $info = fstat($fd);          $len = $info["size"];          $data = fread($fd, $len);          fclose($fd);        }    $lines = taxonomy_xml_obo_prepare_data($data);
  
  // Initialize sandbox the first time through.
  if (!isset($context['sandbox']['line_count_total'])) {
    $line_count = count($lines);
//    $context['sandbox']['lines'] = $lines;
    $context['sandbox']['line_count_total'] = $line_count;
    $context['sandbox']['line_count_processed'] = 0;
    // used to run relationships after import of terms
    $context['sandbox']['terms'] = array();  // or use the static cache attempt? =& taxonomy_xml_current_terms(); 
    $context['message'] = t('We will import terms @size at a time.', array('@size' => $size));
  }
  // 
  for ($i = 0; $i < $size; $i++) {
    $term_context = array();
// drupal_set_message('loop: ' . $i);  
// drupal_set_message('lines: <pre>'.var_export($context['sandbox']['lines'],TRUE).'</pre>');
// drupal_set_message('lines before: <pre>'.var_export($lines,TRUE).'</pre>');
    if ($term = taxonomy_xml_obo_import_term($lines, $term_context)) {
// drupal_set_message('term: <pre>'.var_export($term,TRUE).'</pre>');
      $context['sandbox']['terms'][] = $term;
    }
// drupal_set_message('term_context: <pre>'.var_export($term_context,TRUE).'</pre>');
    $context['sandbox']['line_count_processed'] += $term_context['line_count'];
// drupal_set_message('lines+: '. $context['sandbox']['line_count_processed']);
  }
  // open the file for writing 
  if ($fd = fopen($file->filepath, 'w+')) {  // $fd is valid
    // write the array to the file
// drupal_set_message('lines after: <pre>'.var_export($lines,TRUE).'</pre>');
    array_pop($lines);  // get rid of the extra line we added
    foreach($lines as $line) {
      fwrite($fd,$line . "\n");
    }
    // close the file
    fclose($fd);
  }
  else {  
    drupal_set_message(t("Cannot open file."), 'error');
  }


// drupal_set_message('term: <pre>'.var_export($context,TRUE).'</pre>');
  if ($context['sandbox']['line_count_processed'] >= $context['sandbox']['line_count_total']) {
    $context['finished'] = 1;  // done!
  }
  else {
    $context['finished'] = $context['sandbox']['line_count_processed'] / $context['sandbox']['line_count_total'];
  }
//  watchdog('obo', 'Finished? ' . $context['finished']);
}


function taxonomy_xml_obo_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = t('Successful import');
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}


function taxonomy_xml_obo_import_term(&$lines, &$term_context) {
// drupal_set_message('inside: <pre>'.var_export($lines,TRUE).'</pre>');
  $term = array();
  $term_line = 0;
  $finished = FALSE; // set to TRUE when one term is processed
  if (!is_array($lines) || empty($lines)) {
    return;
  }
  while (!$finished && list($i, $line) = each($lines)) {
    if (trim($line) == '[Term]') {
      if ($term_line > 1) {
        // this check covers us if an OBO file is missing a blank line
        // this was never reset to zero, and we've had at least one line
        // we should try to 'save' the term before going farther
        taxonomy_xml_obo_add_term($terms, $term);
        $finished = TRUE;
      }
      $term_line = 1;
    }
    elseif ($term_line > 1 && trim($line) == '') {
      taxonomy_xml_obo_add_term($terms, $term);
      $term_line = 0;  // vestigal code; not needed
      $finished = TRUE;
    }
    elseif ($term_line) {
      taxonomy_xml_obo_add_pair($line, $term);
      $term_line++;
    }
    $term_context['line_count']++;
// drupal_set_message('i: '. $i);
    unset($lines[$i]);
  }
    
// drupal_set_message('term: <pre>'.var_export($terms,TRUE).'</pre>'); 
  
  if (!empty($terms)) {
    $term = (object)$terms[0];    
  }
  else { // it's not a term and we're done here
    return;  
  }
  if (!isset($term->is_obsolete) || $term->is_obsolete != TRUE) {  
    $term->vid = taxonomy_xml_obo_get_vid($term->namespace);
  
    $term->description = $term->def;
    unset($term->def);
      
    // taxonomy_enhancer likes everything in a "fields" array.
    $term->fields = array();
      // Furthermore, everything can have multiple values (per data structure).
      $term->fields['field_uri'] = array(
        0 => array(
        'value' => $term->id,
        ),
    );
    // This is ugly and I don't like it.  Extreme unnecessary complexity.
    // honestly, not any good reason can't just use $term->field_whatever -- 
    // this is taxonomy, not CCK, single values are fine!
      
    // Data is now massaged and referring to itself correctly, 
    // Start creating terms so we can retrieve term ids
  
    // Ensure name is valid    
    if (! $term->name) {
      drupal_set_message(t("Problem, we were unable to find a specific label for the term referred to as @id.", array('@id' => $term->id)));
      $term->name = $term->id;
    }
  
    // See if a definition already exists in the DB. Build on that. Otherwise start setting up a new prototype $term object.
    // This does a get by name. If we had a better GUID to lookup, should try that instead
    // @TODO - should do this with URI (id) instead of name
    // Because we let OBO terms identify their vocabulary, 
    // this is a term-specific vid rather than a global vid
//      $existing_term = _taxonomy_xml_get_term_placeholder($term->name, $term->vid);
    // Merge the old term objects properties into this one. Really just want its tid, but there may be more info I should not lose.
    // Our new input takes precedence over older data
//    foreach ((array)$existing_term as $key => $value) { 
//      if (! isset($term->$key)) {
//        $term->$key = $value;
//      }
//      }
  
      // The term object is now as tidy as it can be as a self-contained entity.
      $save_term = (array)$term;
drupal_set_message('<pre>'.var_export($save_term,TRUE).'</pre>');      
      $status = taxonomy_save_term($save_term);
      unset($save_term);
  } // end term-construction.
  
}


/**
 * Reads an OBO file and creates the term definitions found in it.
 * 
 * Implementation of the taxonomy_xml_HOOK_parse() callback.
 * 
 * This passes through several times, first finding all TaxonConcepts mentioned
 * in the Doc, then finding if they refer to each other, then creating or
 * retrieving and merging existing term data, then looping again to ensure they
 * each point at each other. At no point can we assume to have all term items in
 * memory, it just tries to instantiate them incrementally, depending on the
 * information available.
 * 
 * @param $data XML string representing the TCS file to be parsed
 * @param $vid Vocabulary ID the terms are to be created under
 * @param $url The source URL of the TCS doc. Used to create URIs from document
 * IDs
 * 
 * @return An array of the terms created in this parsing process.
 *
 */
function taxonomy_xml_obo_parse_old(&$data, $vid = 0, $url = '') {
  #drupal_set_message(t("Importing from provided OBO data file %url.", array('%url' => $url)));
/*
  if ($vid == 0) {
    // We've been asked to use the vocab described in the source file.
    // OBO files can define different vocabs for different terms
    // but I don't think the parser can handle that
    drupal_set_message(t("No vocabulary specified in the form, using a default one."));
    // Create a placeholder, use that
    $vocabulary = _taxonomy_xml_get_vocabulary_placeholder('Taxa');
    $vid = $vocabulary->vid;
  }
  else {
    // Else using a form-selected vocob.
    $vocabulary = taxonomy_vocabulary_load($vid);
  }
*/  

  // BEGIN the first loop, finding terms in this document

  // Remembering all terms is memory-intensive, but may be more efficient in batch jobs.
  // Use a static list where possible. EXPERIMENTAL

// @TODO the batch API alternative to this approach would be to save chunks, parsing the text to , and then run the below code several times.  That involves factoring out the related terms code (already commented out to save resources) to be run as another pass.

  $terms =& taxonomy_xml_current_terms();
  
  #dpm(array("About to start analyzing a data doc $url, known terms are: " => $terms));
  
  $lines = taxonomy_xml_obo_prepare_data($data);
  $term_line = 0;
  foreach($lines as $i => $line) {
    if (trim($line) == '[Term]') {
      if ($term_line > 1) {
        // this check covers us if an OBO file is missing a blank line
        // this was never reset to zero, and we've had at least one line
        // we should try to 'save' the term before going farther
        taxonomy_xml_obo_add_term($terms, $term);
      }
      $term_line = 1;
    }
    elseif ($term_line > 1 && trim($line) == '') {
      taxonomy_xml_obo_add_term($terms, $term);
      $term_line = 0;
    }
    elseif ($term_line) {
      taxonomy_xml_obo_add_pair($line, $term);
      $term_line++;
    }
    unset($lines[$i]);
  }

  foreach ($terms as &$term) {
    // Skip duplicates (some dupes may exist due to the use of handles)
//    if ($term->taxonomy_xml_presaved) continue;
    $term = (object)$term;
    
    if (!isset($term->is_obsolete) || $term->is_obsolete != TRUE) {
  
      $term->vid = taxonomy_xml_obo_get_vid($term->namespace);
  
      // Translate the predicate statements into the syntax we need
//      taxonomy_xml_canonicize_predicates($term);
      // ^^ does nothing useful?
  
      $term->description = $term->def;
      unset($term->def);
      
      // taxonomy_enhancer likes everything in a "fields" array.
      $term->fields = array();
      // Furthermore, everything can have multiple values (per data structure).
      $term->fields['field_uri'] = array(
          0 => array(
          'value' => $term->id,
          ),
      );
      // This is ugly and I don't like it.  Extreme unnecessary complexity.
      // honestly, not any good reason can't just use $term->field_whatever -- 
      // this is taxonomy, not CCK, single values are fine!
      
      // Data is now massaged and referring to itself correctly, 
      // Start creating terms so we can retrieve term ids
  
      // Ensure name is valid    
      if (! $term->name) {
          drupal_set_message(t("Problem, we were unable to find a specific label for the term referred to as @id.", array('@id' => $term->id)));
          $term->name = $term->id;
      }
  
      // See if a definition already exists in the DB. Build on that. Otherwise start setting up a new prototype $term object.
      // This does a get by name. If we had a better GUID to lookup, should try that instead
      // @TODO - should do this with URI (id) instead of name
      // Because we let OBO terms identify their vocabulary, 
      // this is a term-specific vid rather than a global vid
//      $existing_term = _taxonomy_xml_get_term_placeholder($term->name, $term->vid);
      // Merge the old term objects properties into this one. Really just want its tid, but there may be more info I should not lose.
      // Our new input takes precedence over older data
//      foreach ((array)$existing_term as $key => $value) { 
//        if (! isset($term->$key)) {
//          $term->$key = $value;
//        }
//      }
  
      // The term object is now as tidy as it can be as a self-contained entity.
      $save_term = (array)$term;
      $status = taxonomy_save_term($save_term);
      unset($save_term);

/*
///// BEGIN SECTION WE SKIP, can do in separet function /////
      // Re-retrieve the new term definition, just in case anything extra happened to it during processing
      $new_term = taxonomy_xml_get_term_by_name_from_vocab($term->name, $term->vid);
      if (! $new_term) {
          drupal_set_message(t("It seems like we failed to create and retrieve a term called %term_name", array('%term_name' => $term->name)), 'error');
      }
      // Merge retrieved values back over our main definition so the handles are up-to-date
      //
      // There seems to be a lot of data copying back and forth, 
      // even though we are using handles, but this is because we are
      // temporarily retaining more information in the $term object than 
      // the system knows how to handle.
      foreach ((array)$new_term as $key => $value) { 
          $term->$key = $value; 
      }
  
      if ( $status == SAVED_NEW ) {
          // Just remember this is fresh - for useful feedback messages.
          $term->taxonomy_xml_new_term = TRUE;
      }
  
      // It's possible that not all the referenced items were available in the current document/loop
      // Add referred items to the import queue for later processing
      taxonomy_xml_add_all_children_to_queue($term);
      $term->taxonomy_xml_presaved = TRUE; // A flag to avoid double-processing
////// END SECTION SKIPPED EN MASSE //////
*/
    }
  } // end term-construction loop;

  #dpm(array('created a bunch of terms, now they need relations set.' => $terms));

// as above, all relations will be done in another function  
//  taxonomy_xml_set_term_relations($terms);

  #dpm(array('After re-linking, we now have all terms set' => $terms));
  // returning terms allows taxonomy_xml to give the user feedback
  return $terms;
}


/**
 * Prepare a dataset (giant string) by breaking it into rows.
 */
function taxonomy_xml_obo_prepare_data(&$data) {
  $rows = explode("\n", $data);
  $row_count = count($rows);
//  drupal_set_message(t('%rowcount rows of data', array('%rowcount' => $row_count)));
  
  // have a blank line at the end to ensure last term will be processed
  if (trim($rows[$row_count - 1]) != '') {
    $rows[$row_count] == '';
  }
  unset($data);
  return $rows;
}


/**
 * Take a single line of an OBO file and parse into key and value.
 *
 * This function takes a term array and adds a new key and value to it for each
 * parsed line.  The term arary is passed by reference.
 *
 * @param $line
 *   A single value line of an OBO formatted file.
 *
 * @param $term
 *   A term array passed by reference.
 *
 * @return
 *   TRUE on success and FALSE on failure.
 *
 */
function taxonomy_xml_obo_add_pair(&$line, &$term) {
  $parts = explode(': ', $line, 2);
  if (count($parts) == 2) {
    $key = $parts[0];
    $value = $parts[1];
    if (taxonomy_xml_obo_defines_multiple($key)) {
      if (!isset($term[$key])) {
        $term[$key] = array();
      }
      $term[$key][] = $value;
    } else {
      $term[$key] = $value;
    }
    return TRUE;
  }
  elseif (count($parts) == 1 && $parts[0]) {
    drupal_set_message(t("Non-parsed line: <tt>%s</tt>", $parts[0]), 'warning');
    return FALSE;
  }
}


/**
 * Returns TRUE if key matches any of a preset list of potential multiple tags.
 *
 * See http://geneontology.org/GO.format.obo-1_2.shtml#S.2.2 "Optional tags".
 */
function taxonomy_xml_obo_defines_multiple($key) {
  $multiple = array(
    'alt_id',
    'is_a',
    'subset',
    'consider',
    'synonym',
  );
  return in_array($key, $multiple);
}

/**
 *
 *
 * The option for user-matched vocabularies to namespaces could be added here. 
 */
function taxonomy_xml_obo_get_vid($namespace, $reset = FALSE) {
  static $vids = array();
  if (!isset($vids[$namespace]) && !$reset) {
    $query = "SELECT vid FROM {vocabulary} WHERE name = '%s'";
    $vid = db_result(db_query($query, $namespace));
    if (!$vid) {  // try a full name
      switch ($namespace) {
        case 'biological_process':
          $name = 'Biological processes';
          break;
        case 'cellular_component':
          $name = 'Cellular components';
          break;
        case 'molecular_function':
          $name = 'Molecular functions';
          break;
        default:
          $name = NULL;
          break;        
      }
      if ($name) {
        $vid = db_result(db_query($query, $name));
      }
      if (!$vid) {
        // create a new vocabulary
        // @TODO
        drupal_set_message(t("OOPS!  Sorry no vocabulary match for @namespace and we haven't implemented creating new vocaularies yet.  This won't work.", array('@nampespace' => $namespace)), 'error');
      }      
    }
    drupal_set_message(t("Namespace @namespace matches vocabulary ID number @vid.", array('@namespace' => $namespace, '@vid' => $vid)));
    $vids[$namespace] = $vid;
  }
  return $vids[$namespace];
}


/**
 * Add term to the array of terms we are building and reset the term.
 */
function taxonomy_xml_obo_add_term(&$terms, &$term) {
  $terms[] = $term;
  $term = array();
  return;
}


/**
 * Batch processing functions modeled on Pro Drupal Development page 562+
 */



////////////////////////////////////////////////////////////////////////
// OBO Writer
// define('TAXONOMY_OBO_1_2', 'http://geneontology.org/GO.format.obo-1_2.shtml');
