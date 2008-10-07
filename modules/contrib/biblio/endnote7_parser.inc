<?php
// $Id: endnote7_parser.inc,v 1.9.2.4 2008/05/12 01:03:37 rjerome Exp $
/**
 *
 *   Copyright (C) 2006  Ron Jerome
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
 *
 */
function en7_parse(& $tags, & $node_array, $node) {
  $node_id = null;
  $contributors = null;
  $type_map = array(
    '0' => '102', // Journal Article
    '1' => '100', // Book
    '2' => '108', // Thesis
    '3' => '103', // Conference Proceedings
    '4' => '120', // Personal Communication
    '5' => '105', // NewsPaper Article
    '6' => '113', // Computer Program
    '7' => '101', // Book Section
    '8' => '106', // Magazine Article
    '9' => '100', // Edited Book
    '10' => '109', // Report
    '11' => '122', // Map
    '12' => '114', // Audiovisual Material
    '13' => '112', // Artwork
    '15' => '119', // Patent
    '16' => '107', // Electronic Source
    '17' => '117', // Bill
    '18' => '116', // Case
    '19' => '115', // Hearing
    '20' => '121', // Manuscript
    '21' => '110', // Film or Broadcast
    '22' => '118', // Statute
    '26' => '123', // Chart or Table
    '31' => '129' // Generic

  );
  foreach ($tags as $tag) {
    if ($tag['type'] == "complete" || $tag['type'] == "open") {
      $tag_name = $tag['tag'];
      $data = trim($tag['value']);
      switch ($tag_name) {
        case 'RECORD' :
          if ($tag['type'] == "open") {
            $node_id = array_push($node_array, $node) - 1;
            $node_array[$node_id]['biblio_contributors'] = array();
          }
          break;
        case 'REFERENCE_TYPE' :
          $node_array[$node_id]['biblio_type'] = $type_map[$data];
          break;
          //Author information
        case 'AUTHOR' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $data,
            'ctid' => 1
          );
          //$contributors['author'][] = $data;
          break;
        case 'SECONDARY_AUTHOR' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $data,
            'ctid' => 2
          );
          break;
        case 'TERTIARY_AUTHOR' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $data,
            'ctid' => 3
          );
          break;
        case 'SUBSIDIARY_AUTHOR' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $data,
            'ctid' => 4
          );
          break;
        case 'translated-authors' :
          break;
        case 'auth-address' :
          break;
        case 'auth-affiliaton' :
          break;
          // Title information
        case 'TITLE' :
          $node_array[$node_id]['title'] .= $data;
          break;
        case 'SECONDARY_TITLE' :
          $node_array[$node_id]['biblio_secondary_title'] .= $data;
          break;
        case 'TERTIARY_TITLE' :
          $node_array[$node_id]['biblio_tertiary_title'] .= $data;
          break;
        case 'ALTERNATE_TITLE' :
          $node_array[$node_id]['biblio_alternate_title'] .= $data;
          break;
        case 'SHORT_TITLE' :
          $node_array[$node_id]['biblio_short_title'] .= $data;
          break;
        case 'translated-title' :
          $node_array[$node_id]['biblio_translated_title'] .= $data;
          break;
          // Periodical information
        case 'full-title' :
          break;
        case 'abbr-1' :
          break;
        case 'abbr-2' :
          break;
        case 'abbr-3' :
          $node_array[$node_id][$periodical] = $data;
          break;
        case 'PAGES' :
          $node_array[$node_id]['biblio_pages'] .= $data;
          break;
          //Volume, issue, and edition information
        case 'VOLUME' :
          $node_array[$node_id]['biblio_volume'] .= $data;
          break;
        case 'NUMBER' :
          $node_array[$node_id]['biblio_number'] .= $data;
          break;
        case 'ISSUE' :
          $node_array[$node_id]['biblio_issue'] .= $data;
          break;
        case 'secondary-volume' :
          break;
        case 'secondary-issue' :
          break;
        case 'NUMBER_OF_VOLUMES' :
          $node_array[$node_id]['biblio_number_of_volumes'] .= $data;
          break;
        case 'EDITION' :
          $node_array[$node_id]['biblio_edition'] .= $data;
          break;
        case 'SECTION' :
          $node_array[$node_id]['biblio_section'] .= $data;
          break;
        case 'REPRINT_EDITION' :
          $node_array[$node_id]['biblio_reprint_edition'] .= $data;
          break;
        case 'reprint-status' :
          break;
        case 'KEYWORD' :
          $node_array[$node_id]['biblio_keywords'] .= (empty($node_array[$node_id]['biblio_keywords'])) ? $data : ", $data";
          //       $node_array[$node_id]['biblio_keywords'][] = $data;
          break;
        case 'YEAR' :
          $node_array[$node_id]['biblio_year'] .= $data;
          break;
        case 'DATE' :
          $node_array[$node_id]['biblio_date'] .= $data;
          break;
        case 'PLACE_PUBLISHED' :
          $node_array[$node_id]['biblio_place_published'] .= $data;
          break;
        case 'PUBLISHER' :
          $node_array[$node_id]['biblio_publisher'] .= $data;
          break;
        case 'orig-pub' :
          break;
        case 'ISBN' :
          $node_array[$node_id]['biblio_isbn'] .= $data;
          break;
        case 'ACCESSION_NUMBER' :
          $node_array[$node_id]['biblio_accession_number'] .= $data;
          break;
        case 'CALL_NUMBER' :
          $node_array[$node_id]['biblio_call_number'] .= $data;
          break;
        case 'ABSTRACT' :
          $node_array[$node_id]['biblio_abst_e'] .= $data;
          break;
        case 'label' :
          break;
        case 'image' :
          break;
        case 'caption' :
          break;
        case 'NOTES' :
          $node_array[$node_id]['biblio_notes'] .= $data;
          break;
        case 'research-notes' :
          $node_array[$node_id]['biblio_notes'] .= $data;
          break;
        case 'URL' :
          $node_array[$node_id]['biblio_url'] .= $data;
          break;
        case 'access-date' :
          break;
        case 'modified-date' :
          break;
        case 'CUSTOM1' :
          $node_array[$node_id]['biblio_custom1'] .= $data;
          break;
        case 'CUSTOM2' :
          $node_array[$node_id]['biblio_custom2'] .= $data;
          break;
        case 'CUSTOM3' :
          $node_array[$node_id]['biblio_custom3'] .= $data;
          break;
        case 'CUSTOM4' :
          $node_array[$node_id]['biblio_custom4'] .= $data;
          break;
        case 'CUSTOM5' :
          $node_array[$node_id]['biblio_custom5'] .= $data;
          break;
        case 'CUSTOM6' :
          $node_array[$node_id]['biblio_custom6'] .= $data;
          break;
        case 'custom7' :
          break;
        case 'misc1' :
          break;
        case 'misc2' :
          break;
        case 'misc3' :
          break;
        case 'TYPE_OF_WORK' :
          $node_array[$node_id]['biblio_type_of_work'] .= $data;
          break;
      }
    }
  }
}

