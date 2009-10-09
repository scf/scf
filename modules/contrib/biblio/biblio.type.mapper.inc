<?php
// $Id: biblio.type.mapper.inc,v 1.1.2.2 2009/07/22 18:20:51 rjerome Exp $
function biblio_get_type_map($file_format, $default = FALSE) {
  if ($default) {
    switch ($file_format) {
      case 'bibtex':
        return $map = array(
              'article'       => 102,
              'book'          => 100,
              'booklet'       => 129,
              'conference'    => 103,
              'inbook'        => 101,
              'incollection'  => 101,
              'inproceedings' => 103,
              'manual'        => 129,
              'mastersthesis' => 108,
              'misc'          => 129,
              'phdthesis'     => 108,
              'proceedings'   => 104,
              'techreport'    => 129,
              'unpublished'   => 124,
        );
      case 'tagged':
        return $map = array(
              "Journal Article"         => 102,
              "Conference Paper"        => 103,
              "Conference Proceedings"  => 104,
              "Report"                  => 109,
              "Book"                    => 100,
              "Edited Book"             => 100,
              "Book Section"            => 101,
              "Thesis"                  => 108,
              "Patent"                  => 119,
              "Generic"                 => 129,
              "Newspaper Article"       => 105,
              "Web Page"                => 107,
              "Film or Broadcast"       => 110,
              "Artwork"                 => 112,
              "Audiovisual Material"    => 114,
              "Hearing"                 => 115,
              "Case"                    => 116,
              "Bill"                    => 117,
              "Statute"                 => 118,
              "Personal Communication"  => 120,
              "Manuscript"              => 121,
              "Map"                     => 122,
              "Chart or Table"          => 123,
              "Unpublished Work"        => 124,
              "Online Database"         => 125,
              "Government Document"     => 126,
              "Classical Work"          => 127,
              "Legal Rule or Regulation" => 128,
        );
      case 'ris':
        return $map = array(
            'ABST' => 129,
            'ADVS' => 114,
            'ART'  => 112,
            'BILL' => 117,
            'BOOK' => 100,
            'CASE' => 116,
            'CHAP' => 101,
            'COMP' => 113,
            'CONF' => 103,
            'CTLG' => 129,
            'DATA' => 125,
            'ELEC' => 129,
            'GEN'  => 129,
            'HEAR' => 115,
            'ICOMM' => 107,
            'INPR'  => 129,
            'JFULL' => 129,
            'JOUR'  => 102,
            'MAP'   => 122,
            'MGZN'  => 106,
            'MPCT'  => 110,
            'MUSIC' => 129,
            'NEWS'  => 105,
            'PAMP'  => 129,
            'PAT'   => 119,
            'PCOMM' => 120,
            'RPRT'  => 109,
            'SER'   => 100,
            'SLIDE' => 129,
            'SOUND' => 129,
            'STAT'  => 125,
            'THES'  => 108,
            'UNBILl' => 129,
            'UNPB'  => 124,
            'VIDEO' => 129,
        );
      case 'endnote7':
        return $map = array(
        0 => 102, // Journal Article
        1 => 100, // Book
        2 => 108, // Thesis
        3 => 103, // Conference Proceedings
        4 => 120, // Personal Communication
        5 => 105, // NewsPaper Article
        6 => 113, // Computer Program
        7 => 101, // Book Section
        8 => 106, // Magazine Article
        9 => 100, // Edited Book
        10 => 109, // Report
        11 => 122, // Map
        12 => 114, // Audiovisual Material
        13 => 112, // Artwork
        15 => 119, // Patent
        16 => 107, // Electronic Source
        17 => 117, // Bill
        18 => 116, // Case
        19 => 115, // Hearing
        20 => 121, // Manuscript
        21 => 110, // Film or Broadcast
        22 => 118, // Statute
        26 => 123, // Chart or Table
        31 => 129 // Generic
        );
      case 'endnote8':
        return  $map = array(
        2 => 112, // artwork
        3 => 114, // Audio Visual
        4 => 117, // bill
        5 => 101, // Book Section
        6 => 100, // Book
        7 => 116, // case
        9 => 113, // software
        17 => 102, // Journal Article
        10 => 104, // Conference Proceeding
        12 => 107, // web page
        13 => 129, // Generic
        14 => 115, // hearing
        19 => 106, // magazine_article
        20 => 122, // map
        21 => 110, // film
        21 => 111, // broadcast
        23 => 105, // newspaper_article
        25 => 119, // patent
        26 => 120, // personal communication
        27 => 109, // Report
        28 => 129, // Edited Book
        31 => 118, // statute
        32 => 108, // Thesis
        34 => 124, // unpublished
        36 => 121, // manuscript
        37 => 129, // figure
        38 => 123, // chart
        39 => 129, // equation
        43 => 129, // electronic article
        44 => 129, // electronic book
        45 => 125, // online database
        46 => 126, // government_document
        47 => 103, // conference_paper
        48 => 129, // online multimedia
        49 => 127, // Classical Work
        50 => 128, // legal_ruling
        52 => 129, // Dictionary
        53 => 129, // Encyclopedia
        54 => 129, // Grant
        );
      case 'unixref':
        return $map = array(
            'error'        => 0,
            'book'         => 100, // Book
            'journal'      => 102, // Journal Article
            'standard'     => 129, // Generic
            'conference'   => 103, // conference_paper
            'report-paper' => 109, // Report
            'dissertation' => 108, // Thesis
            'database'     => 125, // online database
            'sa_component' => 129
           );
    }
  }
  else {
    $map = variable_get('biblio_'.$file_format.'_type_map', array());
    if (empty($map)) $map = biblio_get_type_map($file_format, TRUE);
    return $map;

  }
}
function biblio_get_type_names($file_format, $default = FALSE) {
  if ($default) {
    switch ($file_format) {
      case 'bibtex':
        return $names = array(
              'article' => 'An article from a journal',
              'book'  => 'A book with an explicit publisher',
              'booklet' => 'A work that is printed and bound, but without a named publisher or sponsoring institution',
              'conference' => 'An article in a conference proceedings',
              'inbook' => 'A part of a book, usually untitled. May be a chapter (or section or whatever) and/or a range of pages',
              'incollection'  => 'A part of a book having its own title',
              'inproceedings' => 'An article in a conference proceedings',
              'manual'  => 'Technical documentation',
              'mastersthesis' => 'A Master\'s thesis',
              'misc' => 'For use when nothing else fits',
              'phdthesis' => 'A Ph.D. thesis',
              'proceedings' => 'The proceedings of a conference',
              'techreport' => 'A report published by a school or other institution, usually numbered within a series',
              'unpublished' => 'A document having an author and title, but not formally published',
        );
      case 'tagged':
        return $names = array(
              "Journal Article"         => "Journal Article",
              "Conference Paper"        => "Conference Paper",
              "Conference Proceedings"  => "Conference Proceedings",
              "Report"                  => "Report",
              "Book"                    => "Book",
              "Edited Book"             => "Edited Book",
              "Book Section"            => "Book Section",
              "Thesis"                  => "Thesis",
              "Patent"                  => "Patent",
              "Generic"                 => "Generic",
              "Newspaper Article"       => "Newspaper Article",
              "Web Page"                => "Web Page",
              "Film or Broadcast"       => "Film or Broadcast",
              "Artwork"                 => "Artwork",
              "Audiovisual Material"    => "Audiovisual Material",
              "Hearing"                 => "Hearing",
              "Case"                    => "Case",
              "Bill"                    => "Bill",
              "Statute"                 => "Statute",
              "Personal Communication"  => "Personal Communication",
              "Manuscript"              => "Manuscript",
              "Map"                     => "Map",
              "Chart or Table"          => "Chart or Table",
              "Unpublished Work"        => "Unpublished Work",
              "Online Database"         => "Online Database",
              "Government Document"     => "Government Document",
              "Classical Work"          => "Classical Work",
              "Legal Rule or Regulation" => "Legal Rule or Regulation",
        );
      case 'ris':
        return $names = array(
              'ABST' => 'Abstract',
              'ADVS' => 'Audiovisual material',
              'ART'  => 'Art Work',
              'BILL' => 'Bill/Resolution',
              'BOOK' => 'Book, Whole',
              'CASE' => 'Case',
              'CHAP' => 'Book chapter',
              'COMP' => 'Computer program',
              'CONF' => 'Conference proceeding',
              'CTLG' => 'Catalog',
              'DATA' => 'Data file',
              'ELEC' => 'Electronic Citation',
              'GEN'  => 'Generic',
              'HEAR' => 'Hearing',
              'ICOMM' => 'Internet Communication',
              'INPR'  => 'In Press',
              'JFULL' => 'Journal (full)',
              'JOUR'  => 'Journal',
              'MAP'   => 'Map',
              'MGZN'  => 'Magazine article',
              'MPCT'  => 'Motion picture',
              'MUSIC' => 'Music score',
              'NEWS'  => 'Newspaper',
              'PAMP'  => 'Pamphlet',
              'PAT'   => 'Patent',
              'PCOMM' => 'Personal communication',
              'RPRT'  => 'Report',
              'SER'   => 'Serial (Book, Monograph)',
              'SLIDE' => 'Slide',
              'SOUND' => 'Sound recording',
              'STAT'  => 'Statute',
              'THES'  => 'Thesis/Dissertation',
              'UNBILl' => 'Unenacted bill/resolution',
              'UNPB'  => 'Unpublished work',
              'VIDEO' => 'Video recording',
        );
      case 'endnote7':
        return $names = array(
        0 => 'Journal Article',
        1 => 'Book',
        2 => 'Thesis',
        3 => 'Conference Proceedings',
        4 => 'Personal Communication',
        5 => 'NewsPaper Article',
        6 => 'Computer Program',
        7 => 'Book Section',
        8 => 'Magazine Article',
        9 => 'Edited Book',
        10 => 'Report',
        11 => 'Map',
        12 => 'Audiovisual Material',
        13 => 'Artwork',
        15 => 'Patent',
        16 => 'Electronic Source',
        17 => 'Bill',
        18 => 'Case',
        19 => 'Hearing',
        20 => 'Manuscript',
        21 => 'Film or Broadcast',
        22 => 'Statute',
        26 => 'Chart or Table',
        31 => 'Generic',
        );

      case 'endnote8':
        return $names = array(
        2 => 'Artwork',
        3 => 'Audio Visual',
        4 => 'Bill',
        5 => 'Book Section',
        6 => 'Book',
        7 => 'Case',
        9 => 'Software',
        17 => 'Journal Article',
        10 => 'Conference Proceeding',
        12 => 'Web page',
        13 => 'Generic',
        14 => 'Hearing',
        19 => 'Magazine Article',
        20 => 'Map',
        21 => 'Film',
        21 => 'Broadcast',
        23 => 'Newspaper Article',
        25 => 'Patent',
        26 => 'Personal Communication',
        27 => 'Report',
        28 => 'Edited Book',
        31 => 'Statute',
        32 => 'Thesis',
        34 => 'Unpublished',
        36 => 'Manuscript',
        37 => 'Figure',
        38 => 'Chart',
        39 => 'Equation',
        43 => 'Electronic Article',
        44 => 'Electronic Book',
        45 => 'Online Database',
        46 => 'Government Document',
        47 => 'Conference Paper',
        48 => 'Online Multimedia',
        49 => 'Classical Work',
        50 => 'Legal Ruling',
        52 => 'Dictionary',
        53 => 'Encyclopedia',
        54 => 'Grant',
        );
      case 'unixref':
        return $name = array(
            'error'        => 'Error',
            'book'         => 'Book',
            'journal'      => 'Journal Article',
            'standard'     => 'Generic',
            'conference'   => 'Conference Paper',
            'report-paper' => 'Report',
            'dissertation' => 'Thesis',
            'database'     => 'Online database',
            'sa_component' => 'SA Component',
           );
    }
  }
  else {
    $names = variable_get('biblio_'.$file_format.'_type_names', array());
    if (empty($names))  $names = biblio_get_type_names($file_format, TRUE);
    return $names;
  }
}