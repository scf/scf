<?php
// $Id: crossref_unixref_parser.inc,v 1.1.2.10 2009/07/22 18:20:51 rjerome Exp $
function unixref_startElement($parser, $name, $attrs) {
  global $node, $element,  $attribute, $auth_category, $contrib_count, $org_count;
  switch ($name) {
    case 'doi_record' :
      $node = array();
      $element = $name;
      break;
    case 'book':
    case 'journal':
    case 'standard':
    case 'conference':
    case 'report-paper':
    case 'dissertation':
    case 'database':
    case 'sa_component':
      $node['biblio_type'] = _unixref_type_map($name);
      $element = $name;
      break;
    case 'journal_article':
    case 'conference_paper':
    case 'content_item':
    case 'report-paper_metadata':
    case 'standard_metadata':
    case 'database_date':
    case 'component':
      $node['year'] = '';
      $node['doi']  = '';
      $element = $name;
      break;
    case 'person_name' :
      $auth_category = _unixref_get_contributor_category($attrs['contributor_role']);
      if (!isset($contrib_count)) $contrib_count = 0;
      $element = $name;
      break;
    case 'organization':
      if (!isset($org_count)) $org_count = 0;
      $element = $name;
      break;
    case 'issn':
      if ($attrs['media_type'] == 'print') $attribute = 'issn_print';
      $element = $name;
      break;
    case 'isbn':
      if ($attrs['media_type'] == 'print') $attribute = 'isbn_print';
      $element = $name;
      break;
    case 'i':  // HTML font style tags
    case 'b':
    case 'u':
    case 'sub':
    case 'sup':
      unixref_characterData(NULL, ' <'.$name.'>');
      break;
    default :
      $element = $name;
  }

}
function unixref_decode(&$item, $key) {
  $item = html_entity_decode($item, NULL, 'UTF-8');
}
function unixref_endElement($parser, $name) {
  global $node, $nids, $element, $attribute, $terms, $batch_proc, $session_id, $save_node, $auth_category, $contrib_count, $org_count;
  switch ($name) {
    case 'doi_record' :
      array_walk_recursive($node,'unixref_decode' );
      $nids[] = biblio_save_node($node, $batch, $session_id, $save_node);
      break;
    case 'person_name' :
      $node['biblio_contributors'][$auth_category][$contrib_count]['auth_type'] = _biblio_get_auth_type($auth_category, $node['biblio_type']);
      $node['biblio_contributors'][$auth_category][$contrib_count]['name'] =
          $node['biblio_contributors'][$auth_category][$contrib_count]['lastname'];
      if (isset($node['biblio_contributors'][$auth_category][$contrib_count]['firstname'])) {
         $node['biblio_contributors'][$auth_category][$contrib_count]['name'] .=
            ', ' . $node['biblio_contributors'][$auth_category][$contrib_count]['firstname'];
      }

      $auth_category = '';
      $contrib_count++;
      break;
    case 'organization' :
      $node['biblio_contributors'][5][$org_count]['auth_type'] = _biblio_get_auth_type(5, $node['biblio_type']);
      $org_count++;
      break;
    case 'pages':
      if (isset($node['biblio_first_page'])) $node['biblio_pages'] = $node['biblio_first_page'];
      if (isset($node{'biblio_last_page'}))  $node['biblio_pages'] .= ' - ' . $node['biblio_last_page'];
      break;
    case 'publication_date':

      break;
    case 'journal_issue':
      $node['biblio_date'] = (!empty($node['month'])?$node['month'].'/':'').$node['year'];
      break;
    case 'journal_article':
    case 'conference_paper':
    case 'content_item':
    case 'report-paper_metadata':
    case 'standard_metadata':
    case 'database_date':
    case 'component':
      $node['biblio_year'] = $node['year'];
      $node['biblio_doi']  = $node['doi'];
      break;
    case 'issn':
      if($attribute == 'issn_print') $node['biblio_issn'] = $node['issn'];
      $node['issn'] = '';
      break;
    case 'isbn':
      if($attribute == 'isbn_print') $node['biblio_isbn'] = $node['isbn'];
      $node['isbn'] = '';
      break;
    case 'i':  // HTML font style tags
    case 'b':
    case 'u':
    case 'sub':
    case 'sup':
      unixref_characterData(NULL, '</'.$name.'> ');
      break;
    default :
  }
}

function unixref_characterData($parser, $data) {
  global $node, $element, $auth_category, $contrib_count, $org_count;
  $data = htmlspecialchars_decode($data);
  if (trim($data)) {
    switch ($element) {
      case 'surname' :
        $node['biblio_contributors'][$auth_category][$contrib_count]['lastname'] .= $data;
        break;
      case 'given_name' :
        $node['biblio_contributors'][$auth_category][$contrib_count]['firstname'] .= $data;
        break;
      case 'suffix':
        $node['biblio_contributors'][$auth_category][$contrib_count]['suffix'] .= $data;
        break;
      case 'affiliation' :
        $node['biblio_contributors'][$auth_category][$contrib_count]['affiliation'] .= $data;
        break;
      case 'organization':
        $node['biblio_contributors'][5][$org_count]['name'] .= $data;
        break;
      case 'year':
        $node['year'] = $data;
        break;
      case 'month':
        $node['month'] = $data;
        break;
      case 'day':
        $node['day'] = $data;
        break;
      default:
        if ($field = _unixref_field_map(trim($element))) {
          $node[$field] .= $data;
        }

    }
  }
}
/*
 * map a unixref XML field to a biblio field
 */
function _unixref_field_map($field) {
  static $field_map = array();
  if (empty($field_map)) {
    $field_map = array(
      'publisher_place'     => 'biblio_place_published',
      'publisher_name'      => 'biblio_publisher',
      'volume'              => 'biblio_volume',
      'number'              => 'biblio_number',
      'issue'               => 'biblio_issue',
      'edition_number'      => 'biblio_edition',
      'section'             => 'biblio_section',
      'doi'                 => 'doi',
      'title'               => 'title',
      'isbn'                => 'isbn',
      'issn'                => 'issn',
      'first_page'          => 'biblio_first_page',
      'last_page'           => 'biblio_last_page',
      // Journal metadata
      'full_title'          => 'biblio_secondary_title',
      'abbrev_title'        => 'biblio_short_title',
      // Conference metadata
      'conference_location' => 'biblio_place_published',
      'conference_name'     => 'biblio_secondary_title',
      'conference_acronym'  => 'biblio_short_title',
      // Proceedings metadata
      'proceedings_title'   => 'biblio_secondary_title',
      'year'                => 'year',
      'month'               => 'month',
      'day'                 => 'day',
      'degree'              => 'biblio_type_of_work',
      'error'               => 'error',
      'language'            => 'biblio_lang',
    );

  }
  return (isset($field_map[$field])) ? $field_map[$field]: FALSE;
}

function _unixref_type_map($type){
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('unixref');
  }
  return (isset($map[$type]))?$map[$type]:129; //return the biblio type or 129 (Misc) if type not found
}

function _unixref_get_contributor_category($role) {
  if ($role == 'author') return 1;
  if ($role == 'editor') return 2;
  if ($role == 'chair') return 3;
  if ($role == 'translator') return 4;
  return NULL;
}