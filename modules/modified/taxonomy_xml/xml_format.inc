<?php
/* double-commented to avoid conflict with svn
// $Id: xml_format.inc,v 1.1.4.5 2008/10/30 00:48:20 dman Exp $ 
*/

/**
 * @file Include routines for the original XML parsing and taxonomy/term
 * creation.
 */

/**
 * taxonomy_xml_xml_create
 * @param $vid
 *   Which vocabulary to generate the tree for.
 *
 * @param $parent
 *   The term ID under which to generate the tree. If 0, generate the tree
 *   for the entire vocabulary.
 *
 * @param $depth
 *   Internal use only.
 *
 * @param $max_depth
 *   The number of levels of the tree to return. Leave NULL to return all levels.
 *
 * @return
 *   The text of an XML document.
 */
function taxonomy_xml_xml_create($vid, $parent = 0, $depth = -1, $max_depth = NULL) {
  $output = '<?xml version="1.0" standalone="no"?>'."\n";
  $output .= '<!DOCTYPE taxonomy SYSTEM "taxonomy.dtd">'."\n";
  $tree = module_invoke('taxonomy', 'get_tree', $vid, $parent, $depth, $max_depth);
  if ($tree) {
    $vocabulary = module_invoke('taxonomy', 'get_vocabulary', $vid);
    $output .= "<vocabulary>\n";
    foreach ($vocabulary as $key => $value) {
      if (is_array($value)) {
        $output .= "<$key>". check_plain(implode(',', $value)) ."</$key>";
      }
      else {
        $output .= "<$key>". check_plain($value) ."</$key>";
      }
    }
    foreach ($tree as $term) {
      
      $synonyms = taxonomy_get_synonyms($term->tid);
      $output .= "<term>";
      foreach ($term as $key => $value) {
        if ($key == 'parents') {
          foreach ($value as $parent) {
            $output .= "<parent>". check_plain($parent) ."</parent>";
          }
        }
        else {
          $output .= "<$key>". check_plain($value) ."</$key>";
        }
      }
      if ($synonyms) {
        $output .= "<synonyms>";
        $output .= implode("\n", $synonyms);
        $output .= "</synonyms>";
      }
      $output .= "</term>";
    }
    $output .= "</vocabulary>\n";
  }
  return $output;
}

/**
 * Call-back function used by the XML parser.
 */
function taxonomy_xml_element_start($parser, $name, $attributes) {
  global $_tx_term, $_tx_element, $_tx_tag;

  switch ($name) {
    case 'vocabulary':
      $_tx_element = $name;
      break;
    case 'term':
      $_tx_element = $name;
      $_tx_term += 1;
  }
  $_tx_tag = $name;
}

/**
 * Call-back function used by the XML parser.
 */
function taxonomy_xml_element_end($parser, $name) {
  global $_tx_element;

  switch ($name) {
    case 'vocabulary':
    case 'term':
      $_tx_element = '';
  }
}

/**
 * Call-back function used by the XML parser.
 */
function taxonomy_xml_element_data($parser, $data) {
  global $_tx_vocabulary, $_tx_element, $_tx_terms, $_tx_term, $_tx_tag;

  switch ($_tx_element) {
    case 'term':
      if ($_tx_tag == 'parent') {
        if (trim($data)) {
          $_tx_terms[$_tx_term][$_tx_tag][] = $data;
        }
      }
      else {
        @$_tx_terms[$_tx_term][$_tx_tag] .= trim($data);
      }
      break;
    default:
      $_tx_vocabulary[$_tx_tag] = isset($_tx_vocabulary[$_tx_tag]) ? $_tx_vocabulary[$_tx_tag] .= trim($data) : trim($data);
      // (PHP strict is boring for lazy coders.)
  }
}

/**
 * Initiate the parser on the custom XML schema.
 * 
 * This uses the XML callback parser with tag callbacks.
 * 
 * @see taxonomy_xml_element_start
 * @see taxonomy_xml_element_end
 * @see taxonomy_xml_element_data
 *
 */
function taxonomy_xml_xml_parse(&$data, $vid = 0) {
  global $_tx_terms, $_tx_vocabulary;

  // Unset the global variables before we use them:
  unset($GLOBALS['_tx_element'], $GLOBALS['_tx_term'], $GLOBALS['_tx_tag']);
  $_tx_terms = array();
  $_tx_vocabulary = array();
  $new_terms = array();
  $terms = array();

  // parse the data:
  $xml_parser = drupal_xml_parser_create($data);
  xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, FALSE); //
  xml_set_element_handler($xml_parser, 'taxonomy_xml_element_start', 'taxonomy_xml_element_end');
  xml_set_character_data_handler($xml_parser, 'taxonomy_xml_element_data');

  if (!xml_parse($xml_parser, $data, 1)) {
    watchdog('taxonomy_xml', 'Taxonomy_xml: failed to parse file: %error at line %line.', array('%error' => xml_error_string(xml_get_error_code($xml_parser)), '%line' => xml_get_current_line_number($xml_parser)), array(), WATCHDOG_ERROR);
    drupal_set_message(t('Failed to parse file: %error at line %line.', array('%error' => xml_error_string(xml_get_error_code($xml_parser)), '%line' => xml_get_current_line_number($xml_parser))), 'error');
  }
  xml_parser_free($xml_parser);

  
  // If an existing vocabulary has been chosen or has the same name as the vocabulary being added,
  // terms should be added to the existing vocabulary. Otherwise a new vocabulary should be created.
  
  if ($vid == 0) {
    $name = $_tx_vocabulary['name'];
    $_tx_vocabulary = (array) _taxonomy_xml_get_vocabulary_placeholder(trim($name));
  }
  else {
    $_tx_vocabulary = (array) (module_invoke('taxonomy', 'get_vocabulary', $vid));
  }
  
  // Get the maximum depth of terms
  $term_depth = array();
  foreach ($_tx_terms as $term) {
    $term_depth[] = $term['depth'];
  }
  // Import terms in order of depth
  $new_tid = array();
  for ($i=0;$i<=max($term_depth);$i++) {
    foreach ($_tx_terms as $term) {
      if ($term['depth'] != $i) {
        continue;
      }
      $term['vid'] = $_tx_vocabulary['vid'];
      $term['old_tid'] = $term['tid'];
      unset($term['tid']);
      if (isset($term['parent'])) {
        foreach ((array)$term['parent'] as $key => $value) {
          if ($value) {
            $term['parent'][$key] = $new_tid[$value];
          }
        }
      }
      $term_exists = FALSE;
      if (! variable_get('taxonomy_xml_duplicate', 0)) {
        // Retrieve pre-existing terms - by name - if possible
        $existing_terms = module_invoke('taxonomy', 'get_term_by_name', $term['name']);
        if (count($existing_terms > 0)) {
          foreach ($existing_terms as $existing_term) {
            if ($existing_term->vid == $term['vid']) {
              $term_exists = TRUE;
              $terms[$existing_term->tid] = $existing_term;
              // Map the term tid from the imported XML file to the tid in term_data database table
              $new_tid[$term['old_tid']] = $existing_term->tid;
              $skipped_terms[$existing_term->name] = 1;
            }
          }
        }
      }
      // If the term doesn't already exist in this vocabulary, add it.
      if (!$term_exists) {
        taxonomy_save_term($term);
        // Map the term tid from the imported XML file to the tid in term_data database table
        $new_tid[$term['old_tid']] = $term['tid'];
        $new_terms[] = $term['name'];
        $terms[$term['tid']] = taxonomy_get_term($term['tid']);
      }
    }
  }
  $output = t('Vocabulary %name: ', array('%name' => $_tx_vocabulary['name']));
  if ($new_terms) {
    $output .= t('Added term(s) %terms. ', array('%terms' => implode(', ', $new_terms)));
  }
  else {
    $output .= t('No terms added. ');
  }
  if (isset($skipped_terms)) {
    $output .= t('Ignored duplicate term(s) %terms. ', array('%terms' => implode(', ', array_keys($skipped_terms))));
  }
  drupal_set_message($output);

  return $terms;
}
