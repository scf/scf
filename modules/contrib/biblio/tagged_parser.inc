<?php
// $Id: tagged_parser.inc,v 1.8.2.8 2008/11/29 03:27:53 rjerome Exp $
/**
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
 */
function _endnote_tagged_import(& $data, $node_template, & $node_array) {
  $incite = false;
  $node_id = null;
  $contributors = null;
  foreach (explode("\n", $data) as $line) {
    $line = trim($line);
    $line_len = strlen($line);
    if ($line_len) {
      $start = strpos($line, "%"); // There could be some unprintables at the beginning of the line so fine the location of the %
      if ($start !== false) {
        $tag = drupal_substr($line, $start, 2);
        $value = trim(drupal_substr($line, $start +3));
      }
      else {
        $value = $line;
      }
    }
    if ($line_len) { // if this is not a blank line
      if (!$incite) {
        $node_id = array_push($node_array, $node_template) - 1;
        $node_array[$node_id]['biblio_contributors'] = array();
        $incite = true;
      }
      switch ($tag) {
        case '%0' :
          switch ($value) {
            case "Journal Article" :
              $node_array[$node_id]['biblio_type'] = 102;
              break; // journal
            case "Conference Paper" :
              $node_array[$node_id]['biblio_type'] = 103;
              break;
            case "Conference Proceedings" :
              $node_array[$node_id]['biblio_type'] = 104;
              break; // conference proceedings
            case "Report" :
              $node_array[$node_id]['biblio_type'] = 109;
              break; // report
            case "Book" :
            case "Edited Book" :
              $node_array[$node_id]['biblio_type'] = 100;
              break; // book section
            case "Book Section" :
              $node_array[$node_id]['biblio_type'] = 101;
              break; // book section
            case "Thesis" :
              $node_array[$node_id]['biblio_type'] = 108;
              break; // thesis
            case "Patent" :
              $node_array[$node_id]['biblio_type'] = 119;
              break; // patent
            case "Generic" :
            default :
              $node_array[$node_id]['biblio_type'] = 129;
              break; // generic
          }
          break;
        case '%A' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $value,
            'ct_id' => '1'
          );
          break;
        case '%B' :
          $node_array[$node_id]['biblio_secondary_title'] = $value;
          break;
        case '%C' :
          $node_array[$node_id]['biblio_place_published'] = $value;
          break;
        case '%D' :
          $node_array[$node_id]['biblio_year'] = $value;
          break;
        case '%E' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $value,
            'ct_id' => '2'
          );
          break;
        case '%F' :
          $node_array[$node_id]['biblio_label'] = $value;
          break;
        case '%I' :
          $node_array[$node_id]['biblio_publisher'] = $value;
          break;
        case '%J' : // Journal
          $node_array[$node_id]['biblio_secondary_title'] = $value;
          break;
        case '%K' :
          $node_array[$node_id]['biblio_keywords'] .= ($node_array[$node_id]['biblio_keywords']) ? "; $value" : $value;
          break;
        case '%L' :
          $node_array[$node_id]['biblio_call_number'] = $value;
          break;
        case '%M' : // accession_number
          $node_array[$node_id]['biblio_accession_number'] = $value;
          break;
        case '%N' :
          $node_array[$node_id]['biblio_issue'] = $value;
          break;
        case '%P' :
          $node_array[$node_id]['biblio_pages'] = $value;
          break;
        case '%R' :
          $node_array[$node_id]['biblio_doi'] = $value;
          break;
        case '%S' :
          $node_array[$node_id]['biblio_tertiary_title'] = $value;
          break;
        case '%T' :
          $node_array[$node_id]['title'] = $value;
          break;
        case '%U' :
          $node_array[$node_id]['biblio_url'] = $value;
          break;
        case '%V' :
          $node_array[$node_id]['biblio_volume'] = $value;
          break;
        case '%X' :
          $node_array[$node_id]['biblio_abst_e'] .= $value;
          break;
        case '%Y' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $value,
            'ct_id' => '3'
          );
          break;
        case '%Z' :
          $node_array[$node_id]['biblio_notes'] .= $value;
          break;
        case '%1' : // CUSTOM 1
          $node_array[$node_id]['biblio_custom1'] = $value;
          break;
        case '%2' : // CUSTOM 2
          $node_array[$node_id]['biblio_custom2'] = $value;
          break;
        case '%3' : // CUSTOM 3
          $node_array[$node_id]['biblio_custom3'] = $value;
          break;
        case '%4' : // CUSTOM 4
          $node_array[$node_id]['biblio_custom4'] = $value;
          break;
        case '%#' : // CUSTOM 5
          $node_array[$node_id]['biblio_custom5'] = $value;
          break;
        case '%$' : // CUSTOM 6
          $node_array[$node_id]['biblio_custom6'] = $value;
          break;
        case '%6' :
          break;
        case '%7' :
          $node_array[$node_id]['biblio_edition'] = $value;
          break;
        case '%8' :
          $node_array[$node_id]['biblio_date'] = $value;
          break;
        case '%9' :
          $node_array[$node_id]['biblio_type_of_work'] = $value;
          break;
        case '%?' :
          break;
        case '%@' :
          $node_array[$node_id]['biblio_isbn'] = $value;
          break;
        case '%!' :
          break;
        case '%&' :
          break;
        case '%(' :
          break;
        case '%)' :
          break;
        case '%*' :
          break;
        case '%+' :
          break;
        default :
          break;
      } //end switch
    }
    else {
      $incite = false;
    } // end if ($start !== false)
  } // end while
  return count($node_array);
}