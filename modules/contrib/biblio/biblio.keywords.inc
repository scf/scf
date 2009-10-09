<?php
// $Id: biblio.keywords.inc,v 1.6.2.12 2009/07/29 18:11:33 rjerome Exp $
/**
 *   biblio.module for Drupal
 *
 *   Copyright (C) 2006-2009  Ron Jerome
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
 * @param $name
 * @return array of keywords
 */
function biblio_get_keyword_by_name($name) {
  static $keywords = array();

  if (!$kid = array_search($name, $keywords)) {
    $term = db_fetch_object(db_query("SELECT * FROM {biblio_keyword_data} k WHERE LOWER(k.word) = LOWER('%s')", trim($name)));
    if ($term) {
      $keywords[$term->kid] = $term;
      return $keywords[$term->kid];
    }
    else {
      return FALSE;
    }
  }

  return $keywords[$kid];
}

/**
 * @param $kid
 * @return unknown_type
 */
function biblio_get_keyword_by_id($kid) {
  static $keywords = array();

  if (!isset($keywords[$kid])) {
    $keywords[$kid] = db_fetch_object(db_query('SELECT * FROM {biblio_keyword_data} WHERE kid = %d', $kid));
  }

  return $keywords[$kid];

}

/**
 * @param $node
 * @return unknown_type
 */
function biblio_load_keywords($vid) {

  $result = db_query('SELECT bkd.* FROM {biblio_keyword} bk INNER JOIN {biblio_keyword_data} bkd ON bk.kid = bkd.kid  WHERE bk.vid = %d ORDER BY bkd.word ASC', $vid);
  $keywords[$vid] = '';
  while ($keyword = db_fetch_object($result)) {
    $keywords[$vid][$keyword->kid] = $keyword->word;
  }
  return $keywords[$vid];

}

/**
 * @param $node
 * @return
 */
function biblio_save_keywords(&$node, $update = FALSE) {
  $kw_vocab = variable_get('biblio_keyword_vocabulary', 0);
  $freetagging = variable_get('biblio_keyword_freetagging', 0);
  $taxo_terms = $typed_keywords = array();
  if (!is_array($node->biblio_keywords)) {
    $typed_keywords = biblio_explode_keywords($node->biblio_keywords);
  }
  else {
    $typed_keywords = $node->biblio_keywords;
  }

  if (!$update) db_query('DELETE FROM {biblio_keyword} WHERE nid = %d AND vid = %d', array($node->nid, $node->vid));
  if (isset($node->taxonomy) && is_array($node->taxonomy) && variable_get('biblio_copy_taxo_terms_to_keywords', 0)) { //add any taxonomy terms to our keyword list
    foreach ($node->taxonomy as $vid => $term) {
      if ($vid == 'copy_to_biblio' && $term == 0 ) {// don't copy if user overrides the default to copy, just set the $taxo_terms to an empty array and break out of the for loop
        $taxo_terms = array();
        break;
      }
      if (is_array($term) && !empty($term)) {
        foreach ($term as $tid) {
          if ($tid) {
            $term_obj = taxonomy_get_term($tid);
            $taxo_terms[$term_obj->tid] = $term_obj->name;
          }
        }
      }
      else if ($term) {
        $term_obj = taxonomy_get_term($term);
        $taxo_terms[$term_obj->tid] = $term_obj->name;
      }
    }
  }

  $keywords = array_merge($typed_keywords, $taxo_terms);

  foreach ($keywords as $keyword) {
    $word = (is_object($keyword)) ? trim($keyword->word) : trim($keyword);
    if (!strlen(trim($word))) continue; //skip if we have a blank
    $kid = FALSE;
    // See if the term exists
    if ( ($term = biblio_get_keyword_by_name($word)) ) {
      $kid = $term->kid;
    }
    if (!$kid) {
      $term = array('word' => trim($word));
      $status = biblio_save_keyword($term);
      $kid = $term['kid'];
    }
    // Defend against duplicate, differently cased tags
    if (!isset($inserted[$kid])) {
      if ($update) db_query('DELETE FROM {biblio_keyword} WHERE nid = %d AND vid = %d AND kid = %d', array($node->nid, $node->vid, $kid));
      db_query('INSERT INTO {biblio_keyword} (kid, nid, vid) VALUES (%d, %d, %d)', $kid, $node->nid, $node->vid);
      $inserted[$kid] = TRUE;
    }
  }

  // now if we are saving keywords into a taxonomy freetagging vocabulary, then create the tags string and add it to the node object.

  if ($freetagging && $kw_vocab ) {
    $node->taxonomy['tags'][$kw_vocab] =  biblio_implode_keywords($typed_keywords, ',');
  }

  return count($inserted);
}
/**
 * @param $word
 * @return
 */
function biblio_save_keyword(&$form_values) {
  if (!empty($form_values['kid']) && $form_values['word']) {
    drupal_write_record('biblio_keyword_data', $form_values, 'kid');
    $status = SAVED_UPDATED;
  } else {
    drupal_write_record('biblio_keyword_data', $form_values);
    $status = SAVED_NEW;
  }

  return $status;
}

/**
 * @param $node
 * @return none
 */
function biblio_delete_node_keywords($node) {
  db_query('DELETE FROM {biblio_keyword} WHERE nid = %d', $node->nid);
}

/**
 * @param $node
 * @return none
 */
function biblio_delete_revision_keywords($node) {
  db_query('DELETE FROM {biblio_keyword} WHERE vid = %d', $node->vid);
}

function biblio_delete_keyword($keyword_id) {
  db_query('DELETE FROM {biblio_keyword} WHERE kid = %d', $keyword_id);
  db_query('DELETE FROM {biblio_keyword_data} WHERE kid = %d', $keyword_id);
}

function biblio_explode_keywords($string) {
  $sep = variable_get('biblio_keyword_sep', ',');
  $regexp = '%(?:^|'.$sep.'\ *)("(?>[^"]*)(?>""[^"]* )*"|(?: [^"'.$sep.']*))%x';
  preg_match_all($regexp, $string, $matches);
  $keyword_array = array_unique($matches[1]);

  $keywords = array();
  foreach ($keyword_array as $keyword) {
    // If a user has escaped a term (to demonstrate that it is a group,
    // or includes a comma or quote character), we remove the escape
    // formatting so to save the term into the database as the user intends.
    $keyword = trim(str_replace('""', '"', preg_replace('/^"(.*)"$/', '\1', $keyword)));
    if ($keyword != "") {
      $keywords[] = $keyword;
    }
  }
  return $keywords;
}
function biblio_implode_keywords($keywords, $sep = '') {

  if (empty($sep)) $sep = variable_get('biblio_keyword_sep', ',');
  $string = '';
  foreach ($keywords as $kid => $keyword) {
    $string .= strlen($string)?"$sep ":'';
    if (strpos($keyword, $sep) !== FALSE) {
      $string .= '"'.$keyword.'"';
    }
    else {
      $string .= $keyword;
    }
  }
  return $string;
}