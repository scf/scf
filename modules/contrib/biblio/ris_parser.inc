<?php
// $Id: ris_parser.inc,v 1.4.2.5 2008/05/20 02:01:01 rjerome Exp $
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
function _ris_tagged_import(& $data, $node_template, & $node_array) {
  $incite = false;
  $node_id = null;
  $contributors = null;
  foreach (explode("\n", $data) as $line) {
    //$line = fgets($handle);
    $line_len = strlen($line);
    if ($line_len > 3) {
      $start = strpos($line, "  - "); // There could be some unprintables at the beginning of the line so fine the location of the %
      if ($start !== false) {
        $tag = drupal_substr($line, $start -2, 2);
        $value = trim(drupal_substr($line, $start +4));
      }
      else {
        $value = $line;
      }
    }
    if ($line_len > 3) { // if this is not a blank line
      switch ($tag) {
        case 'ER' :
          if (empty ($node_array[$node_id]['title']))
            $node_array[$node_id]['title'] = t("Untitled");
          $incite = false;
          break;
        case 'TY' :
          $node_id = array_push($node_array, $node_template) - 1;
          $node_array[$node_id]['biblio_contributors'] = array();
          $incite = true;
          switch ($value) {
            case "ART" : // art work
              $node_array[$node_id]['biblio_type'] = 112;
              break;
            case "COMP" : // computer program
              $node_array[$node_id]['biblio_type'] = 113;
              break;
            case "ADVS" : //Audiovisual material
              $node_array[$node_id]['biblio_type'] = 114;
              break;
            case "HEAR" : // Hearing
              $node_array[$node_id]['biblio_type'] = 115;
              break;
            case "CASE" : // Case
              $node_array[$node_id]['biblio_type'] = 116;
              break;
            case "BILL" : // Bill / resolution
              $node_array[$node_id]['biblio_type'] = 117;
              break;
            case "JOUR" : // journal
              $node_array[$node_id]['biblio_type'] = 102;
              break;
            case "CONF" : // conference proceedings
              $node_array[$node_id]['biblio_type'] = 103;
              break;
            case "NEWS" : // newspaper
              $node_array[$node_id]['biblio_type'] = 105;
              break;
            case "MGZN" : // magazine article
              $node_array[$node_id]['biblio_type'] = 106;
              break;
            case "ICOMM" : // internet communication
              $node_array[$node_id]['biblio_type'] = 107;
              break;
            case "RPRT" : // report
              $node_array[$node_id]['biblio_type'] = 109;
              break;
            case "MPCT" : //motion picture
              $node_array[$node_id]['biblio_type'] = 110;
              break;
            case "BOOK" : // book
              $node_array[$node_id]['biblio_type'] = 100;
              break;
            case "CHAP" : // book chapter
              $node_array[$node_id]['biblio_type'] = 101;
              break;
            case "THES" : // thesis
              $node_array[$node_id]['biblio_type'] = 108;
              break;
            case "PAT" : // patent
              $node_array[$node_id]['biblio_type'] = 119;
              break;
            case "PCOMM" : // Personal communication
              $node_array[$node_id]['biblio_type'] = 120;
              break;
            case "MAP" : // map
              $node_array[$node_id]['biblio_type'] = 122;
              break;
            case "DATA" : // database
              $node_array[$node_id]['biblio_type'] = 125;
              break;
            case "STAT" : // statute
              $node_array[$node_id]['biblio_type'] = 125;
              break;
            case "GEN" :
            default :
              $node_array[$node_id]['biblio_type'] = 129;
              break; // generic
          }
          break;
        case 'A1' :
        case 'AU' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $value,
            'ctid' => 1
          );
          break;
        case 'JF' :
          $node_array[$node_id]['biblio_secondary_title'] = $value;
          break;
        case 'CY' :
          $node_array[$node_id]['biblio_place_published'] = $value;
          break;
        case 'Y1' :
        case 'PY' :
          $node_array[$node_id]['biblio_year'] = ($end = strpos($value, "/")) ? substr($value, 0, $end) : $value;
          $node_array[$node_id]['biblio_date'] = $value;
          break;
        case 'A2' :
        case 'ED' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $value,
            'ctid' => '2'
          );
          break;
        case 'PB' :
          $node_array[$node_id]['biblio_publisher'] = $value;
          break;
        case 'T2' :
          $node_array[$node_id]['biblio_secondary_title'] = $value;
          break;
        case 'KW' :
          $node_array[$node_id]['biblio_keywords'] .= ($node_array[$node_id]['biblio_keywords']) ? ", $value" : $value;
          break;
        case 'IS' :
        case 'CP' :
          $node_array[$node_id]['biblio_issue'] = $value;
          break;
        case 'SP' :
        case 'EP' :
          $node_array[$node_id]['biblio_pages'] .= ($tag == "SP") ? $value : " - " . $value;
          break;
        case 'T3' :
          $node_array[$node_id]['biblio_tertiary_title'] = $value;
          break;
        case 'TI' :
        case 'T1' :
        case 'CT' :
        case 'BT' :
          $node_array[$node_id]['title'] = $value;
          break;
        case 'UR' :
          $node_array[$node_id]['biblio_url'] = $value;
          break;
        case 'VL' :
          $node_array[$node_id]['biblio_volume'] = $value;
          break;
        case 'N2' :
          $node_array[$node_id]['biblio_abst_e'] .= $value;
          break;
        case 'A3' :
          $node_array[$node_id]['biblio_contributors'][] = array(
            'name' => $value,
            'ctid' => '5'
          );
          break;
        case 'N1' :
          $node_array[$node_id]['biblio_notes'] .= $value;
          break;
        case 'U1' : // CUSTOM 1
          $node_array[$node_id]['biblio_custom1'] = $value;
          break;
        case 'U2' : // CUSTOM 2
          $node_array[$node_id]['biblio_custom2'] = $value;
          break;
        case 'U3' : // CUSTOM 3
          $node_array[$node_id]['biblio_custom3'] = $value;
          break;
        case 'U4' : // CUSTOM 4
          $node_array[$node_id]['biblio_custom4'] = $value;
          break;
        case 'U5' : // CUSTOM 5
          $node_array[$node_id]['biblio_custom5'] = $value;
          break;
        case 'SN' :
          $node_array[$node_id]['biblio_isbn'] = $value;
          break;
        default :
          break;
      } //end switch
    }
    else {
      //$incite=false;
    } // end if ($start !== false)
  } // end while
  return count($node_array);
}