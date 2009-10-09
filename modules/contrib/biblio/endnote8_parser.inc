<?php
// $Id: endnote8_parser.inc,v 1.10.2.23 2009/07/22 18:20:51 rjerome Exp $
function en8_startElement($parser, $name, $attrs) {
  global $node, $element, $contributors, $contrib_count, $titles, $periodical, $dates, $urls, $keyword_count, $font_attr;
  switch ($name) {
    case 'record' :
      $node = array();
      break;
    case 'style' :
      $font_attr = explode(' ',$attrs['face']);
      foreach ($font_attr as $fatt) {
        switch ($fatt) {
          case 'normal':
            break;
          case 'bold':
            en8_characterData(NULL, '<b>');
            break;
          case 'italic':
            en8_characterData(NULL, '<i>');
            break;
          case 'underline':
            en8_characterData(NULL, '<u>');
            break;
          case 'superscript':
            en8_characterData(NULL, '<sup>');
            break;
          case 'subscript':
            en8_characterData(NULL, '<sub>');
            break;
        }
      }
      break;
    case 'keywords' :
      $keyword_count = 0;
      break;
    case 'authors' :
    case 'secondary-authors' :
    case 'tertiary-authors' :
    case 'subsidiary-authors' :
    case 'translated-authors' :
      $contributors = $name;
      if (!isset($contrib_count))
        $contrib_count = 0;
      break;
    case 'year' :
    case 'pub-dates' :
    case 'copyright-dates' :
      $dates = $name;
      break;
    case 'web-urls' :
    case 'pdf-urls' :
    case 'text-urls' :
    case 'related-urls' :
    case 'image-urls' :
      $urls = $name;
      break;
    default :
      $element = $name;
  }
}
function en8_endElement($parser, $name) {
  global $node, $nids, $element, $terms, $batch_proc, $session_id, $contributors, $contrib_count, $dates, $urls, $keyword_count, $font_attr;
  switch ($name) {
    case 'record' :
      $element = $contributors = $contrib_count = $dates = $urls = '';
      if (!empty($terms)) {
        if (!isset($node['taxonomy'])) $node['taxonomy'] = array();
        $node['taxonomy'] = array_merge($terms,$node['taxonomy']);
      }
      $nids[] = biblio_save_node($node, $batch_proc, $session_id);
      break;
    case 'authors' :
    case 'secondary-authors' :
    case 'tertiary-authors' :
    case 'subsidiary-authors' :
    case 'translated-authors' :
      $contributors = '';
      break;
    case 'author' :
      $contrib_count++;
      break;
    case 'keyword' :
      $keyword_count++;
      break;
    case 'year' :
    case 'pub-dates' :
    case 'copyright-dates' :
      $dates = '';
      break;
    case 'web-urls' :
    case 'pdf-urls' :
    case 'text-urls' :
    case 'related-urls' :
    case 'image-urls' :
      $urls = '';
      break;
    case 'ref-type':
       $node['biblio_type'] = en8_parser_type_map($node['biblio_type']);
        $element = '';
       break;
    case 'style' :
      foreach ($font_attr as $fatt) {
        switch ($fatt) {
          case 'normal':
            break;
          case 'bold':
            en8_characterData(NULL, '</b>');
            break;
          case 'italic':
            en8_characterData(NULL, '</i>');
            break;
          case 'underline':
            en8_characterData(NULL, '</u>');
            break;
          case 'superscript':
            en8_characterData(NULL, '</sup>');
            break;
          case 'subscript':
            en8_characterData(NULL, '</sub>');
            break;
        }
      }
      $font_attr = array();
      break;
    default :
      $element = '';
  }


}
function en8_characterData($parser, $data) {
  global $node, $element, $contributors, $contrib_count, $dates, $urls, $keyword_count;
  if (trim($data)) {
    switch ($element) {
        //Author information
      case 'author' :
        switch ($contributors) {
          case 'authors' :
            $node['biblio_contributors'][1][$contrib_count]['name'] .= $data;
            $node['biblio_contributors'][1][$contrib_count]['auth_type'] = _biblio_get_auth_type(1, $node['biblio_type']);
            break;
          case 'secondary-authors' :
            $node['biblio_contributors'][2][$contrib_count]['name'] .= $data;
            $node['biblio_contributors'][2][$contrib_count]['auth_type'] = _biblio_get_auth_type(2, $node['biblio_type']);
            break;
          case 'tertiary-authors' :
            $node['biblio_contributors'][3][$contrib_count]['name'] .= $data;
            $node['biblio_contributors'][3][$contrib_count]['auth_type'] = _biblio_get_auth_type(3, $node['biblio_type']);
            break;
          case 'subsidiary-authors' :
            $node['biblio_contributors'][4][$contrib_count]['name'] .= $data;
            $node['biblio_contributors'][4][$contrib_count]['auth_type'] = _biblio_get_auth_type(4, $node['biblio_type']);
            break;
          case 'translated-authors' :
            $node['biblio_contributors'][5][$contrib_count]['name'] .= $data;
            $node['biblio_contributors'][5][$contrib_count]['auth_type'] = _biblio_get_auth_type(5, $node['biblio_type']);
            break;
        }
        break;
      case 'keyword' :
        $node['biblio_keywords'][$keyword_count] .= $data;
        break;
      case 'dates' :
        switch ($dates) {
          case 'year' :
            $node['biblio_year'] .= $data;
            break;
        }
        break;
      case 'date' :
        switch ($dates) {
          case 'pub-dates' :
            $node['biblio_date'] .= $data;
            break;
          case 'copyright-dates' :
            break;
        }
        break;
      case 'urls' :
      case 'url' :
        switch ($urls) {
          case 'web-urls' :
          case 'pdf-urls' :
          case 'text-urls' :
          case 'image-urls' :
            break;
          case 'related-urls' :
            $node['biblio_url'] .= $data;
            break;
        }
        break;

      default:
        if ($field = en8_parser_field_map(trim($element))) {
          $node[$field] .= $data;
        }
    }
  }
}

function en8_parser_field_map($enfield) {
  static $fmap = array();
  if(empty($fmap)) {
    $fmap = array(
       'source-app'               => '',
       'rec-number'               => '',
       'ref-type'                 => 'biblio_type',
       'auth-address'             => 'biblio_auth_address',
       'auth-affiliaton'          => '',
       'title'                    => 'title',
       'secondary-title'          =>'biblio_secondary_title',
       'tertiary-title'           => 'biblio_tertiary_title',
       'alt-title'                => 'biblio_alternate_title',
       'short-title'              => 'biblio_short_title',
       'translated-title'         => 'biblio_translated_title',
       'full-title'               => '',
       'abbr-1'                   => '',
       'abbr-2'                   => '',
       'abbr-3'                   => '',
       'pages'                    => 'biblio_pages',
       'volume'                   => 'biblio_volume',
       'number'                   => 'biblio_number',
       'issue'                    => 'biblio_issue',
       'secondary-volume'         => '',
       'secondary-issue'          => '',
       'num-vols'                 => 'biblio_number_of_volumes',
       'edition'                  => 'biblio_edition',
       'section'                  => 'biblio_section',
       'reprint-edition'          => 'biblio_reprint_edition',
       'reprint-status'           => '',
       'year'                     => 'biblio_year',
       'pub-dates'                => 'biblio_date',
       'copyright-dates'          => '',
       'pub-location'             => 'biblio_place_published',
       'publisher'                => 'biblio_publisher',
       'orig-pub'                 => 'biblio_original_publication',
       'isbn'                     => 'biblio_isbn',
       'accession-num'            => 'biblio_accession_number',
       'call-num'                 => 'biblio_call_number',
       'report-id'                => '',
       'coden'                    => '',
       'electronic-resource-num'  => '',
       'abstract'                 => 'biblio_abst_e',
       'label'                    => 'biblio_label',
       'image'                    => '',
       'caption'                  => '',
       'notes'                    => 'biblio_notes',
       'research-notes'           => 'biblio_research_notes',
       'work-type'                => 'biblio_type_of_work',
       'reviewed-item'            => '',
       'availability'             => '',
       'remote-source'            => '',
       'meeting-place'            => '',
       'work-location'            => '',
       'work-extent'              => '',
       'pack-method'              => '',
       'size'                     => '',
       'repro-ratio'              => '',
       'remote-database-name'     => 'biblio_remote_db_name',
       'remote-database-provider' => 'biblio_remote_db_provider',
       'language'                 => 'biblio_lang',
       'web-urls'                 => '',
       'pdf-urls'                 => '',
       'text-urls'                => '',
       'image-urls'               => '',
       'related-urls'             => 'biblio_url',
       'access-date'              => 'biblio_access_date',
       'modified-date'            => '',
       'custom1'                  => 'biblio_custom1',
       'custom2'                  => 'biblio_custom2',
       'custom3'                  => 'biblio_custom3',
       'custom4'                  => 'biblio_custom4',
       'custom5'                  => 'biblio_custom5',
       'custom6'                  => 'biblio_custom6',
       'custom7'                  => 'biblio_custom7',
       'misc1'                    => '',
       'misc2'                    => '',
       'misc3'                    => '',
    );
  }
  return (!empty($fmap[$enfield])) ? $fmap[$enfield] : '';

}

function en8_parser_type_map($en8type){
  static $map = array();
  if (empty($map)) {// first check to see if there are user settings else load the defaults
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('endnote8');
  }
  return (isset($map[$en8type]))?$map[$en8type]:129; //return the biblio type or 129 (Misc) if type not found
}
