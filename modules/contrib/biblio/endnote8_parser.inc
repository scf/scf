<?php
// $Id: endnote8_parser.inc,v 1.10.2.4 2008/05/20 02:01:01 rjerome Exp $
function en8_startElement($parser, $name, $attrs) {
  global $node_array, $node_template, $node_id, $element, $contributors, $contrib_count, $titles, $periodical, $dates, $urls;
  switch ($name) {
    case 'record' :
      $node_id = array_push($node_array, $node_template) - 1;
      break;
    case 'style' :
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
  global $element, $contributors, $contrib_count, $dates, $urls;
  switch ($name) {
    case 'authors' :
    case 'secondary-authors' :
    case 'tertiary-authors' :
    case 'subsidiary-authors' :
    case 'translated-authors' :
      $contributors = '';
      $element = '';
      break;
    case 'author' :
      $contrib_count++;
      break;
    case 'year' :
    case 'pub-dates' :
    case 'copyright-dates' :
      $dates = '';
      $element = '';
      break;
    case 'web-urls' :
    case 'pdf-urls' :
    case 'text-urls' :
    case 'related-urls' :
    case 'image-urls' :
      $urls = '';
      $element = '';
      break;
    case 'style' :
      break;
    default :
      $element = '';
  }
}
function en8_characterData($parser, $data) {
  global $node_array, $node_id, $element, $contributors, $contrib_count, $dates, $urls;
  if (trim($data)) {
    switch ($element) {
      case 'source-app' :
        break;
      case 'rec-number' :
        break;
      case 'ref-type' :
        switch ((int) $data) {
          case 8 : // Journal Article
          case 11 :
          case 15 :
          case 16 :
          case 17 :
          case 18 :
          case 22 :
          case 24 :
          case 29 :
          case 30 :
          case 33 :
            $node_array[$node_id]['biblio_type'] = 102;
            break;
          case 6 : // Book
            $node_array[$node_id]['biblio_type'] = 100;
            break;
          case 5 : // Book Section
            $node_array[$node_id]['biblio_type'] = 101;
            break;
          case 49 : // conference_paper
            $node_array[$node_id]['biblio_type'] = 103;
            break;
          case 47 : // Conference Proceeding
            $node_array[$node_id]['biblio_type'] = 104;
            break;
          case 10 : // proceedings_article
            $node_array[$node_id]['biblio_type'] = 103;
            break;
          case 23 : // newspaper_article
            $node_array[$node_id]['biblio_type'] = 105;
            break;
          case 19 : // magazine_article
            $node_array[$node_id]['biblio_type'] = 106;
            break;
          case 12 : // web_article
            $node_array[$node_id]['biblio_type'] = 107;
            break;
          case 32 : // Thesis
            $node_array[$node_id]['biblio_type'] = 108;
            break;
          case 27 : // Report
            $node_array[$node_id]['biblio_type'] = 109;
            break;
          case 21 : // film
            $node_array[$node_id]['biblio_type'] = 110;
            break;
          case 21 : // broadcast
            $node_array[$node_id]['biblio_type'] = 111;
            break;
          case 2 : // artwork
            $node_array[$node_id]['biblio_type'] = 112;
            break;
          case 9 : // software
            $node_array[$node_id]['biblio_type'] = 113;
            break;
          case 14 : // hearing
            $node_array[$node_id]['biblio_type'] = 115;
            break;
          case 7 : // case
            $node_array[$node_id]['biblio_type'] = 116;
            break;
          case 4 : // bill
            $node_array[$node_id]['biblio_type'] = 117;
            break;
          case 31 : // statute
            $node_array[$node_id]['biblio_type'] = 118;
            break;
          case 25 : // patent
            $node_array[$node_id]['biblio_type'] = 119;
            break;
          case 26 : // personal
            $node_array[$node_id]['biblio_type'] = 120;
            break;
          case 36 : // manuscript
            $node_array[$node_id]['biblio_type'] = 121;
            break;
          case 20 : // map
            $node_array[$node_id]['biblio_type'] = 122;
            break;
          case 38 : // chart
            $node_array[$node_id]['biblio_type'] = 123;
            break;
          case 34 : // unpublished
            $node_array[$node_id]['biblio_type'] = 124;
            break;
          case 45 : // database
            $node_array[$node_id]['biblio_type'] = 125;
            break;
          case 46 : // government_report
            $node_array[$node_id]['biblio_type'] = 126;
            break;
          case 50 : // legal_ruling
            $node_array[$node_id]['biblio_type'] = 128;
            break;
          case 13 : // miscellaneous
            $node_array[$node_id]['biblio_type'] = 129;
            break;
          case 10 : // Conference Proceeding
            //$node_array[$node_id]['biblio_type'] = 2;
            break;
          default :
            $node_array[$node_id]['biblio_type'] = 129;
            break;
        }
        break;
        //Author information
      case 'author' :
        $node_array[$node_id]['biblio_contributors'][$contrib_count]['name'] .= $data;
        switch ($contributors) {
          case 'authors' :
            $node_array[$node_id]['biblio_contributors'][$contrib_count]['type'] = 1;
            break;
          case 'secondary-authors' :
            $node_array[$node_id]['biblio_contributors'][$contrib_count]['type'] = 2;
            break;
          case 'tertiary-authors' :
            $node_array[$node_id]['biblio_contributors'][$contrib_count]['type'] = 3;
            break;
          case 'subsidiary-authors' :
            $node_array[$node_id]['biblio_contributors'][$contrib_count]['type'] = 4;
            break;
          case 'translated-authors' :
            $node_array[$node_id]['biblio_contributors'][$contrib_count]['type'] = 5;
            break;
        }
        break;
      case 'auth-address' :
        $node_array[$node_id]['biblio_auth_address'] .= $data;
        break;
      case 'auth-affiliaton' :
        break;
        // Title information
      case 'title' :
        $node_array[$node_id]['title'] .= $data;
        break;
      case 'secondary-title' :
        $node_array[$node_id]['biblio_secondary_title'] .= $data;
        break;
      case 'tertiary-title' :
        $node_array[$node_id]['biblio_tertiary_title'] .= $data;
        break;
      case 'alt-title' :
        $node_array[$node_id]['biblio_alternate_title'] .= $data;
        break;
      case 'short-title' :
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
        $node_array[$node_id][$periodical] .= $data;
        break;
      case 'pages' :
        $node_array[$node_id]['biblio_pages'] .= $data;
        break;
        //Volume, issue, and edition information
      case 'volume' :
        $node_array[$node_id]['biblio_volume'] .= $data;
        break;
      case 'number' :
        $node_array[$node_id]['biblio_number'] .= $data;
        break;
      case 'issue' :
        $node_array[$node_id]['biblio_issue'] .= $data;
        break;
      case 'secondary-volume' :
        break;
      case 'secondary-issue' :
        break;
      case 'num-vols' :
        $node_array[$node_id]['biblio_number_of_volumes'] .= $data;
        break;
      case 'edition' :
        $node_array[$node_id]['biblio_edition'] .= $data;
        break;
      case 'section' :
        $node_array[$node_id]['biblio_section'] .= $data;
        break;
      case 'reprint-edition' :
        $node_array[$node_id]['biblio_reprint_edition'] .= $data;
        break;
      case 'reprint-status' :
        break;
      case 'keyword' :
        $node_array[$node_id]['biblio_keywords'] .= (empty ($node_array[$node_id]['biblio_keywords'])) ? $data : ", $data";
        //        $node_array[$node_id]['biblio_keywords'][] = $data;
        break;
      case 'dates' :
        switch ($dates) {
          case 'year' :
            $node_array[$node_id]['biblio_year'] .= $data;
            break;
        }
        break;
      case 'date' :
        switch ($dates) {
          case 'pub-dates' :
            $node_array[$node_id]['biblio_date'] .= $data;
            break;
          case 'copyright-dates' :
            break;
        }
        break;
      case 'pub-location' :
        $node_array[$node_id]['biblio_place_published'] .= $data;
        break;
      case 'publisher' :
        $node_array[$node_id]['biblio_publisher'] .= $data;
        break;
      case 'orig-pub' :
        break;
      case 'isbn' :
        $node_array[$node_id]['biblio_isbn'] .= $data;
        break;
      case 'accession-num' :
        $node_array[$node_id]['biblio_accession_number'] .= $data;
        break;
      case 'call-num' :
        $node_array[$node_id]['biblio_call_number'] .= $data;
        break;
      case 'report-id' :
        break;
      case 'coden' :
        break;
      case 'electronic-resource-num' :
        break;
      case 'abstract' :
        $node_array[$node_id]['biblio_abst_e'] .= $data;
        break;
      case 'label' :
        $node_array[$node_id]['biblio_label'] .= $data;
        break;
      case 'image' :
        break;
      case 'caption' :
        break;
      case 'notes' :
        $node_array[$node_id]['biblio_notes'] .= $data;
        break;
      case 'research-notes' :
        $node_array[$node_id]['biblio_notes'] .= $data;
        break;
      case 'work-type' :
        $node_array[$node_id]['biblio_type_of_work'] .= $data;
        break;
      case 'reviewed-item' :
        break;
      case 'availability' :
        break;
      case 'remote-source' :
        break;
      case 'meeting-place' :
        break;
      case 'work-location' :
        break;
      case 'work-extent' :
        break;
      case 'pack-method' :
        break;
      case 'size' :
        break;
      case 'repro-ratio' :
        break;
      case 'remote-database-name' :
        $node_array[$node_id]['biblio_remote_db_name'] .= $data;
        break;
      case 'remote-database-provider' :
        $node_array[$node_id]['biblio_remote_db_provider'] .= $data;
        break;
      case 'language' :
        $node_array[$node_id]['biblio_lang'] .= $data;
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
            $node_array[$node_id]['biblio_url'] .= $data;
            break;
        }
        break;
      case 'access-date' :
        $node_array[$node_id]['biblio_access_date'] .= $data;
        break;
      case 'modified-date' :
        break;
      case 'custom1' :
        $node_array[$node_id]['biblio_custom1'] .= $data;
        break;
      case 'custom2' :
        $node_array[$node_id]['biblio_custom2'] .= $data;
        break;
      case 'custom3' :
        $node_array[$node_id]['biblio_custom3'] .= $data;
        break;
      case 'custom4' :
        $node_array[$node_id]['biblio_custom4'] .= $data;
        break;
      case 'custom5' :
        $node_array[$node_id]['biblio_custom5'] .= $data;
        break;
      case 'custom6' :
        $node_array[$node_id]['biblio_custom6'] .= $data;
        break;
      case 'custom7' :
        $node_array[$node_id]['biblio_custom7'] .= $data;
        break;
      case 'misc1' :
        break;
      case 'misc2' :
        break;
      case 'misc3' :
        break;
    }
  }
}

