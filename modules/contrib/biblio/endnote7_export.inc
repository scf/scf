<?php
// $Id
//
function _endnote7_XML_export($results) {
  $xml = '<?xml version="1.0" encoding="UTF-8"?>';
  $xml .= "\n<XML><RECORDS>\n";
  if (!is_array($results)) {
    $result_array[] = $results;
  }
  else {
    $result_array = $results;
  }
  foreach ($result_array as $pub) {
    $xml .= "<RECORD>\n";
    $xml .= "\t<REFERENCE_TYPE>". _endnote7_type_map($pub->biblio_type) ."</REFERENCE_TYPE>\n";
    $a = $e = $t = $s = '';
    foreach ((array)$pub->biblio_contributors[1] as $auth) {
      $a .= "\t\t<AUTHOR>". htmlspecialchars(trim($auth['name'])) ."</AUTHOR>\n";
    }
    foreach ((array)$pub->biblio_contributors[2] as $auth) {
      $e .= "\t\t<SECONDARY_AUTHOR>". htmlspecialchars(trim($auth['name'])) ."</SECONDARY_AUTHOR>\n";
    }
    foreach ((array)$pub->biblio_contributors[3] as $auth) {
      $t .= "\t\t<TERTIARY_AUTHOR>". htmlspecialchars(trim($auth['name'])) ."</TERTIARY_AUTHOR>\n";
    }
    foreach ((array)$pub->biblio_contributors[4] as $auth) {
      $s .= "\t\t<SUBSIDIARY_AUTHOR>". htmlspecialchars(trim($auth['name'])) ."</SUBSIDIARY_AUTHOR>\n";
    }
    if (!empty ($a))
      $xml .= "\t<AUTHORS>\n$a\t</AUTHORS>\n";
    if (!empty ($e))
      $xml .= "\t<SECONDARY_AUTHORS>\n$e\t</SECONDARY_AUTHORS>\n";
    if (!empty ($t))
      $xml .= "\t<TERTIARY_AUTHORS>\n$t\t</TERTIARY_AUTHORS>\n";
    if (!empty ($s))
      $xml .= "\t<SUBSIDIARY_AUTHORS>\n$s\t</SUBSIDIARY_AUTHORS>\n";
    $xml .= (!empty ($pub->biblio_year)) ? "\t<YEAR>$pub->biblio_year</YEAR>\n" : "";
    $xml .= (!empty ($pub->title)) ? "\t<TITLE>".htmlspecialchars($pub->title)."</TITLE>\n" : "";
    $xml .= (!empty ($pub->biblio_secondary_title)) ? "\t<SECONDARY_TITLE>". htmlentities($pub->biblio_secondary_title) ."</SECONDARY_TITLE>\n" : "";
    $xml .= (!empty ($pub->biblio_place_published)) ? "\t<PLACE_PUBLISHED>". htmlentities($pub->biblio_place_published) ."</PLACE_PUBLISHED>\n" : "";
    $xml .= (!empty ($pub->biblio_publisher)) ? "\t<PUBLISHER>". htmlentities($pub->biblio_publisher) ."</PUBLISHER>\n" : "";
    $xml .= (!empty ($pub->biblio_volume)) ? "\t<VOLUME>".htmlspecialchars($pub->biblio_volume)."</VOLUME>\n" : "";
    $xml .= (!empty ($pub->biblio_number_of_volumes)) ? "\t<NUMBER_OF_VOLUMES>".htmlspecialchars($pub->biblio_number_of_volumes)."</NUMBER_OF_VOLUMES>\n":'';
    $xml .= (!empty ($pub->biblio_issue)) ? "\t<NUMBER>".htmlspecialchars($pub->biblio_issue)."</NUMBER>\n" : "";
    $xml .= (!empty ($pub->biblio_pages)) ? "\t<PAGES>".htmlspecialchars($pub->biblio_pages)."</PAGES>\n" : "";
    $xml .= (!empty ($pub->biblio_section)) ? "\t<SECTION>".htmlspecialchars($pub->biblio_section)."</SECTION>\n" : "";
    $xml .= (!empty ($pub->biblio_tertiary_title)) ? "\t<TERTIARY_TITLE>".htmlspecialchars($pub->biblio_tertiary_title)."</TERTIARY_TITLE>\n" : "";
    $xml .= (!empty ($pub->biblio_edition)) ? "\t<EDITION>".htmlspecialchars($pub->biblio_edition)."</EDITION>\n" : "";
    $xml .= (!empty ($pub->biblio_date)) ? "\t<DATE>".htmlspecialchars($pub->biblio_date)."</DATE>\n" : "";
    $xml .= (!empty ($pub->biblio_type_of_work)) ? "\t<TYPE_OF_WORK>".htmlspecialchars($pub->biblio_type_of_work)."</TYPE_OF_WORK>\n" : "";
    $xml .= (!empty ($pub->biblio_short_title)) ? "\t<SHORT_TITLE>".htmlspecialchars($pub->biblio_short_title)."</SHORT_TITLE>\n" : "";
    $xml .= (!empty ($pub->biblio_alternate_title)) ? "\t<ALTERNATE_TITLE>".htmlspecialchars($pub->biblio_alternate_title)."</ALTERNATE_TITLE>\n" : "";
    $xml .= (!empty ($pub->biblio_isbn)) ? "\t<ISBN>".htmlspecialchars($pub->biblio_isbn)."</ISBN>\n" : "";
    $xml .= (!empty ($pub->biblio_original_publication)) ? "\t<ORIGINAL_PUB>".htmlspecialchars($pub->biblio_original_publication)."</ORIGINAL_PUB>\n" : "";
    $xml .= (!empty ($pub->biblio_reprint_edition)) ? "\t<REPRINT_EDITION>reprint edition</REPRINT_EDITION>\n" : "";
    //    <REVIEWED_ITEM>reviewed item</REVIEWED_ITEM>
    $xml .= (!empty ($pub->biblio_custom1)) ? "\t<CUSTOM1>".htmlspecialchars($pub->biblio_custom1)."</CUSTOM1>\n" : "";
    $xml .= (!empty ($pub->biblio_custom2)) ? "\t<CUSTOM2>".htmlspecialchars($pub->biblio_custom2)."</CUSTOM2>\n" : "";
    $xml .= (!empty ($pub->biblio_custom3)) ? "\t<CUSTOM3>".htmlspecialchars($pub->biblio_custom3)."</CUSTOM3>\n" : "";
    $xml .= (!empty ($pub->biblio_custom4)) ? "\t<CUSTOM4>".htmlspecialchars($pub->biblio_custom4)."</CUSTOM4>\n" : "";
    $xml .= (!empty ($pub->biblio_custom5)) ? "\t<CUSTOM5>".htmlspecialchars($pub->biblio_custom5)."</CUSTOM5>\n" : "";
    $xml .= (!empty ($pub->biblio_custom6)) ? "\t<CUSTOM6>".htmlspecialchars($pub->biblio_custom6)."</CUSTOM6>\n" : "";
    $xml .= (!empty ($pub->biblio_accession_number)) ? "\t<ACCESSION_NUMBER>".htmlspecialchars($pub->biblio_accession_number)."</ACCESSION_NUMBER>\n" : "";
    $xml .= (!empty ($pub->biblio_call_number)) ? "\t<CALL_NUMBER>".htmlspecialchars($pub->biblio_call_number)."</CALL_NUMBER>\n" : "";
    $xml .= (!empty ($pub->biblio_label)) ? "\t<LABEL>".htmlspecialchars($pub->biblio_label)."</LABEL>\n" : "";
    $kw_xml = '';
    $kw_array = array();
    if (!empty($pub->terms)) {
      foreach($pub->terms as $term) {
        $kw_array[] =  trim($term->name) ;
      }
    }
    if (!empty($pub->biblio_keywords)) {
      foreach($pub->biblio_keywords as $term) {
        $kw_array[] = trim($term);
      }
    }
    if (!empty($kw_array)) {
      $kw_array = array_unique($kw_array);
      $xml .= "\t<KEYWORDS>\n";
      foreach($kw_array as $word) {
        $xml .= "\t\t<KEYWORD>". htmlspecialchars(trim($word)) ."</KEYWORD>\n";
      }
      $xml .= "\t</KEYWORDS>\n";
    }
    $xml .= (!empty ($pub->biblio_abst_e)) ? "\t<ABSTRACT>". htmlspecialchars($pub->biblio_abst_e) ."</ABSTRACT>\n" : "";
    $xml .= (!empty ($pub->biblio_notes)) ? "\t<NOTES>".htmlspecialchars($pub->biblio_notes)."</NOTES>\n" : "";
    $xml .= (!empty ($pub->biblio_url)) ? "\t<URL>". urlencode($pub->biblio_url) ."</URL>\n" : "";
    //    <AUTHOR_ADDRESS>author address</AUTHOR_ADDRESS>
    //    <CAPTION>caption</CAPTION>
    $xml .= "</RECORD>\n";
  } //end while
  $xml .= "</RECORDS></XML>";
  return $xml;
}

function _endnote7_type_map($bibliotype) {
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('endnote7');
  }
  return ($en7_type = array_search($bibliotype, $map)) ? $en7_type : 31; //return the biblio type or 129 (Misc) if type not found
}