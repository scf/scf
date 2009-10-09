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

function en7_startElement($parser, $name, $attrs) {
  global $node, $element, $auth_count, $keyword_count;
  switch ($name) {
    case 'RECORD' :
      $node = array();
      $node['biblio_type'] = 102; // we set 102 here because the xml parser won't
      // process a value of 0 (ZERO) which is the
      // ref-type 102. if there is a non-zero value it will be overwritten
      $element = '';
      break;
    case 'AUTHORS':
    case 'SECONDARY_AUTHORS':
    case 'TERTIARY_AUTHORS':
    case 'SUBSIDIARY_AUTHORS':
      $auth_count = 0;
      break;
    case 'KEYWORDS':
      $keyword_count = 0;
      break;
    default:
      $element = $name;
  }
}

function en7_endElement($parser, $name) {
  global $node, $element, $terms, $batch, $session_id, $nids, $auth_count, $keyword_count;
  switch ($name) {
    case 'RECORD' :
      if (!empty($terms)) {
        if (!isset($node['taxonomy'])) $node['taxonomy'] = array();
        $node['taxonomy'] = array_merge($terms, $node['taxonomy']);
      }
      $nids[] = biblio_save_node($node, $batch, $session_id);
      break;
    case 'AUTHOR':
    case 'SECONDARY_AUTHOR':
    case 'TERTIARY_AUTHOR':
    case 'SUBSIDIARY_AUTHOR':
      $auth_category = en7_field_map($name);
      $node['biblio_contributors'][$auth_category][$auth_count]['auth_type'] = _biblio_get_auth_type($auth_category, $node['biblio_type']);
      $auth_count++;
      break;
    case 'KEYWORD':
      $keyword_count++;
      break;
    default:

  }
  $element = '';
}

function en7_characterData($parser, $data) {
  global $node, $element, $auth_count, $keyword_count;
  if (trim($data)) {
    switch ($element) {
      case 'REFERENCE_TYPE':
        $node['biblio_type'] = en7_type_map($data);
        break;
      case 'AUTHOR':
        $node['biblio_contributors'][1][$auth_count]['name'] .= $data;
        break;
      case 'SECONDARY_AUTHOR':
        $node['biblio_contributors'][2][$auth_count]['name'] .= $data;
        break;
      case 'TERTIARY_AUTHOR':
        $node['biblio_contributors'][3][$auth_count]['name'] .= $data;
        break;
      case 'SUBSIDIARY_AUTHOR':
        $node['biblio_contributors'][4][$auth_count]['name'] .= $data;
        break;
      case 'KEYWORD':
        $node['biblio_keywords'][$keyword_count] .= $data;
        break;
      default:
        if ($field = en7_field_map(trim($element))) {
          $node[$field] .= $data;
        }
    }
  }
}
function en7_field_map($enfield) {
  static $fmap = array();
  if(empty($fmap)) {
    $fmap = array(
    'AUTHOR' => 1,            //
    'SECONDARY_AUTHOR' => 2,  // The 4 author fields map back to the author category
    'TERTIARY_AUTHOR' => 3,   // rather than a field name.
    'SUBSIDIARY_AUTHOR' => 4, //
    'REFERENCE_TYPE'  => 'biblio_type',
    'REFNUM'          => '',
    'YEAR'            => 'biblio_year',
    'TITLE'           => 'title',
    'SECONDARY_TITLE' => 'biblio_secondary_title',
    'PLACE_PUBLISHED' => 'biblio_place_published',
    'PUBLISHER'       => 'biblio_publisher',
    'VOLUME'          => 'biblio_volume',
    'ISSUE'           => 'biblio_issue',
    'NUMBER_OF_VOLUMES' => 'biblio_number_of_volumes',
    'NUMBER'            => 'biblio_number',
    'PAGES'             => 'biblio_pages',
    'SECTION'           => 'biblio_section',
    'TERTIARY_TITLE'    => 'biblio_tertiary_title',
    'EDITION'           => 'biblio_edition',
    'DATE'              => 'biblio_date',
    'TYPE_OF_WORK'      => 'biblio_type_of_work',
    'SHORT_TITLE'       => 'biblio_short_title',
    'ALTERNATE_TITLE'   => 'biblio_alternate_title',
    'ISBN'              => 'biblio_isbn',
    'ORIGINAL_PUB'      => 'biblio_original_publication',
    'REPRINT_EDITION'   => 'biblio_reprint_edition',
    'REVIEWED_ITEM'     => '',
    'CUSTOM1'           => 'biblio_custom1',
    'CUSTOM2'           => 'biblio_custom2',
    'CUSTOM3'           => 'biblio_custom3',
    'CUSTOM4'           => 'biblio_custom4',
    'CUSTOM5'           => 'biblio_custom5',
    'CUSTOM6'           => 'biblio_custom6',
    'ACCESSION_NUMBER'  => 'biblio_accession_number',
    'CALL_NUMBER'       => 'biblio_call_number',
    'LABEL'             => 'biblio_label',
    'KEYWORD'           => 'biblio_keywords',
    'ABSTRACT'          => 'biblio_abst_e',
    'NOTES'             => 'biblio_notes',
    'URL'               => 'biblio_url',
    'AUTHOR_ADDRESS'    => '',
    'IMAGE'             => '',
    'CAPTION'           => '',
    );
  }
  return (!empty($fmap[$enfield])) ? $fmap[$enfield] : '';
}
function en7_type_map($entype) {
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('endnote7');
  }
  return (isset($map[$entype]))?$map[$entype]:129; //return the biblio type or 129 (Misc) if type not found
}