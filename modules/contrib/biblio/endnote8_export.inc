<?PHP
// $Id: endnote8_export.inc,v 1.8.2.1 2008/03/25 02:11:59 rjerome Exp $
function en8_XMLout($result){
  $xml = new SimpleXMLElement('<?xml version="1.0" encoding="UTF-8"?>');
  $records = $xml->addChild('records');

  while ($rec = db_fetch_object($result)){
    $record = $records->addChild('record');
    $sa = $record->addChild('source-app');
    $sa->addAttribute('name','Drupal-Biblio');
    $sa->addAttribute('version','6.0');
//<!ELEMENT record (database?, source-app?, rec-number?, ref-type?, contributors?, auth-address?, auth-affiliaton?, titles?, periodical?, pages?, volume?, number?, issue?, secondary-volume?, secondary-issue?, num-vols?, edition?, section?, reprint-edition?, reprint-status?, keywords?, dates?, pub-location?, publisher?, orig-pub?, isbn?, accession-num?, call-num?, report-id?, coden?, electronic-resource-num?, abstract?, label?, image?, caption?, notes?, research-notes?, work-type?, reviewed-item?, availability?, remote-source?, meeting-place?, work-location?, work-extent?, pack-method?, size?, repro-ratio?, remote-database-name?, remote-database-provider?, language?, urls?, access-date?, modified-date?, custom1?, custom2?, custom3?, custom4?, custom5?, custom6?, custom7?, misc1?, misc2?, misc3?)>

/*
<!ELEMENT database (#PCDATA)>
<!ATTLIST database
	name CDATA #IMPLIED
	path CDATA #IMPLIED
>
<!ELEMENT source-app (#PCDATA)>
<!ATTLIST source-app
	name CDATA #IMPLIED
	version CDATA #IMPLIED
>
<!ELEMENT rec-number (#PCDATA)>
*/
    switch ((int) $data) {
//      case 8: // Journal Article
//      case 11:
//      case 15:
//      case 16:
//      case 17:
//      case 18:
//      case 22:
//      case 24:
//      case 29:
//      case 30:
      case 102:
        $type = 8;
        $node_array[$node_id]['biblio_type'] = 102;
        break;
      case 100: // Book
        $type = 6;
       // $node_array[$node_id]['biblio_type'] = 100;
        break;
      case 101: // Book Section
        $type = 5;
        //$node_array[$node_id]['biblio_type'] = 101;
        break;
      case 103: // conference_paper
        $type = 49;
       // $node_array[$node_id]['biblio_type'] = 103;
        break;
      case 103: // Conference Proceeding
        $type = 47;
        //$node_array[$node_id]['biblio_type'] = 103;
        break;
      case 104: // proceedings_article
        $type = 10;
        //$node_array[$node_id]['biblio_type'] = 104;
        break;
      case 105: // newspaper_article
        $type = 23;
       // $node_array[$node_id]['biblio_type'] = 105;
        break;
      case 106: // magazine_article
        $type = 19;
       // $node_array[$node_id]['biblio_type'] = 106;
        break;
      case 107: // web_article
        $type = 12;
       // $node_array[$node_id]['biblio_type'] = 107;
        break;
      case 108: // Thesis
        $type = 32;
        //$node_array[$node_id]['biblio_type'] = 108;
        break;
      case 109: // Report
        $type = 27;
        //$node_array[$node_id]['biblio_type'] = 109;
        break;
      case 110: // film
        $type = 21;
        //$node_array[$node_id]['biblio_type'] = 110;
        break;
      case 111: // broadcast
        $type = 21;
        //$node_array[$node_id]['biblio_type'] = 111;
        break;
      case 112: // artwork
        $type = 2;
        //$node_array[$node_id]['biblio_type'] = 112;
        break;
      case 113: // software
        $type = 9;
        //$node_array[$node_id]['biblio_type'] = 113;
        break;
      case 115: // hearing
        $type = 14;
        //$node_array[$node_id]['biblio_type'] = 115;
        break;
      case 116: // case
        $type = 7;
        //$node_array[$node_id]['biblio_type'] = 116;
        break;
      case 117: // bill
        $type = 4;
        //$node_array[$node_id]['biblio_type'] = 117;
        break;
      case 118: // statute
        $type = 31;
        //$node_array[$node_id]['biblio_type'] = 118;
        break;
      case 119: // patent
        $type = 25;
        //$node_array[$node_id]['biblio_type'] = 119;
        break;
      case 120: // personal
        $type = 26;
        //$node_array[$node_id]['biblio_type'] = 120;
        break;
      case 121: // manuscript
        $type = 36;
        //$node_array[$node_id]['biblio_type'] = 121;
        break;
      case 122: // map
        $type = 20;
        //$node_array[$node_id]['biblio_type'] = 122;
        break;
      case 123: // chart
        $type = 38;
        //$node_array[$node_id]['biblio_type'] = 123;
        break;
      case 124: // unpublished
        $type = 34;
        //$node_array[$node_id]['biblio_type'] = 124;
        break;
      case 125: // database
        $type = 45;
        //$node_array[$node_id]['biblio_type'] = 125;
        break;
      case 126: // government_report
        $type = 46;
        //$node_array[$node_id]['biblio_type'] = 126;
        break;
      case 128: // legal_ruling
       $type = 50;
       //$node_array[$node_id]['biblio_type'] = 128;
        break;
      case 129: // miscellaneous
        $type = 13;
        $node_array[$node_id]['biblio_type'] = 129;
        break;
//      case 10: // Conference Proceeding
        //$node_array[$node_id]['biblio_type'] = 2;
 //       break;
      default:
        $type = 13;
         break;

    }

    $rt = $record->addChild('ref-type',$type);
    $rt->addAttribute('name',' ');

//<!-- Author information -->
  $xml .= '<contributors>';
  $xml .= '<authors>';
  $author_array = explode(";", $rec->biblio_authors);
    foreach($author_array as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= trim($auth); // insert author here.
      $xml .= '</style></author> ';
    }
  $xml .= '</authors>';
  $author_array = explode(";", $rec->biblio_secondary_authors);
  $xml .= '<secondary-authors>';
    foreach($author_array as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= trim($auth); // insert author here.
      $xml .= '</style></author> ';
    }
  $xml .= '</secondary-authors>';
  $author_array = explode(";", $rec->biblio_tertiary_authors);
  $xml .= '<tertiary-authors>';
    foreach($author_array as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= trim($auth); // insert author here.
      $xml .= '</style></author> ';
    }
  $xml .= '</tertiary-authors>';
  $xml .= '<subsidiary-authors>';
  $xml .= '<author><style face="normal" font="default" size="100%">';
  $xml .= ; // insert author here.
  $xml .= '</style></author> ';
  $xml .= '</subsidiary-authors>';
  $xml .= '<translated-authors>';
  $xml .= '<author><style face="normal" font="default" size="100%">';
  $xml .= ; // insert author here.
  $xml .= '</style></author> ';
  $xml .= '</translated-authors>';
  $xml .= '</contributors>';

  $xml .= '<auth-address><style face="normal" font="default" size="100%">';
  // todo auth address here
  $xml .= '</style></auth-address>';
  $xml .= '<auth-affiliaton><style face="normal" font="default" size="100%">';
  // todo auth affiliation here
  $xml .= '</style></auth-affiliaton>';


//<!-- Title information -->
  $xml .= '<titles>';
  $xml .= '  <title><style face="normal" font="default" size="100%">';
  // todo
  $xml .= '  </style></title>';
  $xml .= '  <secondary-title><style face="normal" font="default" size="100%">';
  // todo
  $xml .= '  </style></secondary-title>';
  $xml .= '  <tertiary-title><style face="normal" font="default" size="100%">';
  // todo
  $xml .= '  </style></tertiary-title>';
  $xml .= '  <alt-title><style face="normal" font="default" size="100%">';
  // todo
  $xml .= '  </style></alt-title>';
  $xml .= '  <short-title><style face="normal" font="default" size="100%">';
  // todo
  $xml .= '  </style></short-title>';
  $xml .= '  <translated-title><style face="normal" font="default" size="100%">';
  // todo
  $xml .= '  </style></translated-title>';
  $xml .= '</titles>';
//<!-- Periodical information -->
  $xml .= '<periodical>';
  $xml .=   '<full-title><style face="normal" font="default" size="100%">';
  $xml .= '  </style></full-title>';
  $xml .=   '<abbr-1><style face="normal" font="default" size="100%">';
  $xml .= '  </style></abbr-1>';
  $xml .=   '<abbr-2><style face="normal" font="default" size="100%">';
  $xml .= '  </style></abbr-2>';
  $xml .=   '<abbr-3><style face="normal" font="default" size="100%">';
  $xml .= '  </style></abbr-3>';
  $xml .= '</periodical>';

//<!-- Page range information -->
  $xml .= '<pages><style face="normal" font="default" size="100%">';
  $xml .= '</style></pages>';

//<!-- Volume, issue, and edition information -->
  $xml .= '<volume><style face="normal" font="default" size="100%">';
  $xml .= '</style></volume>';
  $xml .= '<number><style face="normal" font="default" size="100%">';
  $xml .= '</style></number>';
  $xml .= '<issue><style face="normal" font="default" size="100%">';
  $xml .= '</style></issue>';
  $xml .= '<secondary-volume><style face="normal" font="default" size="100%">';
  $xml .= '</style></secondary-volume>';
  $xml .= '<secondary-issue><style face="normal" font="default" size="100%">';
  $xml .= '</style></secondary-issue>';
  $xml .= '<num-vols><style face="normal" font="default" size="100%">';
  $xml .= '</style></num-vols>';
  $xml .= '<edition><style face="normal" font="default" size="100%">';
  $xml .= '</style></edition>';
  $xml .= '<section><style face="normal" font="default" size="100%">';
  $xml .= '</style></section>';
  $xml .= '<reprint-edition><style face="normal" font="default" size="100%">';
  $xml .= '</style></reprint-edition>';
/*
<!ELEMENT reprint-status EMPTY>
<!ATTLIST reprint-status
	date CDATA #IMPLIED
	status (in-file | no-file | on-request) #REQUIRED
>
*/
//<!-- Keywords -->
  $xml .= '<keywords>';
  $xml .= '  <keyword><style face="normal" font="default" size="100%">';
  $xml .= '  </style></keyword>';
  $xml .= '</keywords>';
//<!-- Date information -->
  $xml .= '<dates>';
  $xml .= '  <year><style face="normal" font="default" size="100%">';
  $xml .= '  </style></year>';
<!ELEMENT pub-dates (date+)>
<!ELEMENT copyright-dates (date+)><style face="normal" font="default" size="100%">';
<!ELEMENT date (#PCDATA | style)*><style face="normal" font="default" size="100%">';
<!ATTLIST date
	day CDATA #IMPLIED
	julian CDATA #IMPLIED
	month CDATA #IMPLIED
	year CDATA #IMPLIED
>
  $xml .= '</dates>';

<!-- Publisher information -->
<!ELEMENT pub-location (#PCDATA | style)*>
<!ELEMENT publisher (#PCDATA | style)*>
<!ELEMENT orig-pub (#PCDATA | style)*>
<!-- Identifying numbers -->
<!ELEMENT isbn (#PCDATA | style)*>
<!ELEMENT accession-num (#PCDATA | style)*>
<!ELEMENT call-num (#PCDATA | style)*>
<!ELEMENT report-id (#PCDATA | style)*>
<!ELEMENT coden (#PCDATA | style)*>
<!ELEMENT electronic-resource-num (#PCDATA | style)*>
<!-- Abstract, notes, images, etc. -->
<!ELEMENT abstract (#PCDATA | style)*>
<!ELEMENT label (#PCDATA | style)*>
<!ELEMENT image (#PCDATA)>
<!ATTLIST image
	file CDATA #IMPLIED
	name CDATA #IMPLIED
>
<!ELEMENT caption (#PCDATA | style)*>
<!ELEMENT notes (#PCDATA | style)*>
<!ELEMENT research-notes (#PCDATA | style)*>
<!-- Other detailed information on item -->
<!ELEMENT work-type (#PCDATA | style)*>
<!ELEMENT reviewed-item (#PCDATA | style)*>
<!ELEMENT availability (#PCDATA | style)*>
<!ELEMENT remote-source (#PCDATA | style)*>
<!ELEMENT meeting-place (#PCDATA | style)*>
<!ELEMENT work-location (#PCDATA | style)*>
<!ELEMENT work-extent (#PCDATA | style)*>
<!ELEMENT pack-method (#PCDATA | style)*>
<!ELEMENT size (#PCDATA | style)*>
<!ELEMENT repro-ratio (#PCDATA | style)*>
<!ELEMENT remote-database-name (#PCDATA | style)*>
<!ELEMENT remote-database-provider (#PCDATA | style)*>
<!ELEMENT language (#PCDATA | style)*>
<!-- URL information -->
<!ELEMENT urls (web-urls?, pdf-urls?, text-urls?, related-urls?, image-urls?)>
<!ELEMENT web-urls (url+)>
<!ELEMENT pdf-urls (url+)>
<!ELEMENT text-urls (url+)>
<!ELEMENT related-urls (url+)>
<!ELEMENT image-urls (url+)>
<!ELEMENT url (#PCDATA | style)*>
<!ATTLIST url
	has-ut (yes | no) #IMPLIED
	ppv-app CDATA #IMPLIED
	ppv-ref (yes | no) #IMPLIED
	ppv-ut CDATA #IMPLIED
>
<!-- Information about record -->
<!ELEMENT access-date (#PCDATA | style)*>
<!ELEMENT modified-date (#PCDATA | style)*>
<!-- Custom and miscellaneous fields -->
<!ELEMENT custom1 (#PCDATA | style)*>
<!ELEMENT custom2 (#PCDATA | style)*>
<!ELEMENT custom3 (#PCDATA | style)*>
<!ELEMENT custom4 (#PCDATA | style)*>
<!ELEMENT custom5 (#PCDATA | style)*>
<!ELEMENT custom6 (#PCDATA | style)*>
<!ELEMENT custom7 (#PCDATA | style)*>
<!ELEMENT misc1 (#PCDATA | style)*>
<!ELEMENT misc2 (#PCDATA | style)*>
<!ELEMENT misc3 (#PCDATA | style)*>
<!-- Styled text handling -->
<!ELEMENT style (#PCDATA)>
<!ATTLIST style
	color CDATA #IMPLIED
	face CDATA "normal"
	font CDATA "default"
	size CDATA #IMPLIED
>
  $xml .='</record>';
  }
return $xml;
}
