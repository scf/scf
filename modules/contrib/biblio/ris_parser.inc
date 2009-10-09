<?php
// $Id: ris_parser.inc,v 1.4.2.22 2009/07/22 18:20:51 rjerome Exp $
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
function _ris_tagged_import($file, $terms = array(), $batch = FALSE, $session_id = NULL) {
  if (!($fp = fopen($file->filepath, "r"))) {
    drupal_set_message("could not open RIS input file", 'error');
    return;
  }
  $incite = FALSE;
  $nids = array();
  $contributors = NULL;
  $node=array();
  while (!feof($fp)) {
    $line = fgets($fp);
    $line_len = strlen($line);
    if ($line_len > 3) {
      $start = strpos($line, '  -'); // There could be some unprintables at the beginning of the line so fine the location of the %
      if ($start !== FALSE) {
        $tag = drupal_substr($line, $start -2, 2);
        $value = trim(drupal_substr($line, $start +3));
      }
      else {
        $value = $line;
      }
    }
    if ($line_len > 3) { // if this is not a blank line
      switch ($tag) {
        case 'ER' :
          if (!empty($node)) {
            if (empty ($node['title'])) $node['title'] = t("Untitled");
            if (!empty($terms)) {
              if (!isset($node['taxonomy'])) $node['taxonomy'] = array();
              $node['taxonomy'] = array_merge($terms, $node['taxonomy']);
            }
            $nids[] = biblio_save_node($node, $batch, $session_id);
          }
          $node = array();
          $incite = FALSE;
          break;
        case 'TY' :
          $node['biblio_contributors'] = array();
          $incite = TRUE;
          $node['biblio_type'] = ris_type_map($value);
          break;
        case 'A1' :
        case 'AU' :
          $node['biblio_contributors'][1][] = array(
                'name' => $value,
                'auth_type' => _biblio_get_auth_type(1, $node['biblio_type']));
          break;
        case 'JF' :
          $node['biblio_secondary_title'] .= $value;
          break;
        case 'CY' :
          $node['biblio_place_published'] .= $value;
          break;
        case 'Y1' :
        case 'PY' :
          $node['biblio_year'] = ($end = strpos($value, "/")) ? substr($value, 0, $end) : $value;
          $node['biblio_date'] = $value;
          break;
        case 'A2' :
        case 'ED' :
          $node['biblio_contributors'][2][] = array(
                'name' => $value,
                'auth_type' => _biblio_get_auth_type(2, $node['biblio_type']));
          break;
        case 'PB' :
          $node['biblio_publisher'] .= $value;
          break;
        case 'T2' :
          $node['biblio_secondary_title'] .= $value;
          break;
        case 'KW' :
          $node['biblio_keywords'][] = $value;
          break;
        case 'IS' :
        case 'CP' :
          $node['biblio_issue'] .= $value;
          break;
        case 'SP' :
        case 'EP' :
          $node['biblio_pages'] .= ($tag == "SP") ? $value : " - " . $value;
          break;
        case 'T3' :
          $node['biblio_tertiary_title'] .= $value;
          break;
        case 'TI' :
        case 'T1' :
        case 'CT' :
        case 'BT' :
          $node['title'] .= $value;
          break;
        case 'UR' :
          $node['biblio_url'] .= $value;
          break;
        case 'VL' :
          $node['biblio_volume'] .= $value;
          break;
        case 'AB' :
        case 'N2' :
          $node['biblio_abst_e'] .= $value;
          break;
        case 'A3' :
          $node['biblio_contributors'][5][] = array(
                'name' => $value,
                'auth_type' => _biblio_get_auth_type(5, $node['biblio_type']));
          break;
        case 'N1' :
          $node['biblio_notes'] .= $value;
          break;
        case 'U1' : // CUSTOM 1
          $node['biblio_custom1'] .= $value;
          break;
        case 'U2' : // CUSTOM 2
          $node['biblio_custom2'] .= $value;
          break;
        case 'U3' : // CUSTOM 3
          $node['biblio_custom3'] .= $value;
          break;
        case 'U4' : // CUSTOM 4
          $node['biblio_custom4'] .= $value;
          break;
        case 'U5' : // CUSTOM 5
          $node['biblio_custom5'] .= $value;
          break;
        case 'SN' :
          $node['biblio_isbn'] .= $value;
          break;
        default :
          break;
      } //end switch
    }
    else {
      //$incite=FALSE;
    } // end if ($start !== FALSE)
  } // end while
  fclose($fp);
  return (!empty($nids)) ? $nids : array();
}
function ris_type_map($type) {
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('ris');
  }
  return (isset($map[$type]))?$map[$type]:129; //return the biblio type or 129 (Misc) if type not found
}
