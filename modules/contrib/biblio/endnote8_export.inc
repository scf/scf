<?PHP
// $Id: endnote8_export.inc,v 1.8.2.13 2009/09/29 01:34:42 rjerome Exp $
function _endnote8_XML_export($node, $part = 'record'){
  $style_attr = 'face="normal" font="default" size="100%"';
  switch ($part) {
    case 'begin':
      $xml = '<?xml version="1.0" encoding="UTF-8"?>';
      $xml .= "<xml><records>";
      break;
    case 'record':
      $xml .= "<record>";
      $xml .= '<source-app name="Biblio" version="6.x">Drupal-Biblio</source-app>';
      $xml .= "<ref-type>". _endnote8_type_map($node->biblio_type) ."</ref-type>";
      unset($node->biblio_type);
      //<!-- Author information -->
      $xml .= en8_add_contributors($node);
      $xml .= en8_add_titles($node);
      $xml .= en8_add_keywords($node);
      $xml .= en8_add_dates($node);
      $xml .= en8_add_urls($node);
  
      foreach($node as $key => $value) {
        $entag = en8_field_map($key);
        if (!empty($entag) && !empty($value)) {
          $xml .=  "<".$entag.'><style face="normal" font="default" size="100%">'.htmlspecialchars($value)."</style></$entag>";
        }
      }
      $xml .= "</record>";
      break;
    case 'end': 
      $xml .= '</records></xml>';
      break;
  }
    return $xml;
}

function en8_encode_font_faces(&$node) {
  $search = array('<b>','<i>', '<u>', '<sup>', '<sub>','</b>','</i>', '</u>', '</sup>', '</sub>');
  $replace = array(
    '<style face="bold" font="default" size="100%">',
    '<style face="italic" font="default" size="100%">',
    '<style face="underline" font="default" size="100%">',
    '<style face="superscript" font="default" size="100%">',
    '<style face="subscript" font="default" size="100%">',
    '</sytle>',
    '</sytle>',
    '</sytle>',
    '</sytle>',
    '</sytle>',
  );
  foreach ($node as $key => $value) {
    $node->$key = str_ireplace($search, $replace, $value);
  }
}

function en8_add_titles(&$node) {
  $xml .= '<titles>';
  $xml .= (!empty ($node->title)) ? '<title><style face="normal" font="default" size="100%">'.htmlspecialchars($node->title)."</style></title>" :'';
  $xml .= (!empty ($node->biblio_secondary_title)) ? '<secondary-title><style face="normal" font="default" size="100%">'.htmlspecialchars($node->biblio_secondary_title)."</style></secondary-title>" :'';
  $xml .= (!empty ($node->biblio_tertiary_title)) ? '<tertiary-title><style face="normal" font="default" size="100%">'.htmlspecialchars($node->biblio_tertiary_title)."</style></tertiary-title>" :'';
  $xml .= (!empty ($node->biblio_alternate_title)) ? '<alt-title><style face="normal" font="default" size="100%">'.htmlspecialchars($node->biblio_alternate_title)."</style></alt-title>" :'';
  $xml .= (!empty ($node->biblio_short_title)) ? '<short-title><style face="normal" font="default" size="100%">'.htmlspecialchars($node->biblio_short_title)."</style></short-title>" :'';
  $xml .= (!empty ($node->biblio_translated_title)) ? '<translated-title><style face="normal" font="default" size="100%">'.htmlspecialchars($node->biblio_translated_title)."</style></translated-title>" :'';
  $xml .= '</titles>';
  unset($node->title);
  unset($node->biblio_secondary_title);
  unset($node->biblio_tertiary_title);
  unset($node->biblio_alternate_title);
  unset($node->biblio_short_title);
  unset($node->biblio_translated_title);

  return $xml;
}
function en8_add_urls(&$node) {
  global $base_path;
  $xml = '';
  // TODO: fix URLS
  if (!empty($node->biblio_url)) {
    $xml .= "<web-urls>";
    $xml .= '<url><style face="normal" font="default" size="100%">'.$node->biblio_url."</style></url>";
    $xml .= "</web-urls>";
  }
  unset($node->biblio_url);
  if (!empty ($node->files) && count($node->files) && user_access('view uploaded files')) {
    $xml .= "<related-urls>";
    foreach($node->files as $file) {
      $xml .= '<url><style face="normal" font="default" size="100%">';
      $xml .= file_create_url($file->filepath);
      $xml .= "</style></url>";
    }
    $xml .= "</related-urls>";
  }
  unset($node->files);
  if (!empty($xml)) return "<urls>$xml</urls>";
  return ;
}

function en8_add_dates(&$node) {
  $xml = '';
  if (!empty($node->biblio_year) || !empty($node->biblio_date) ) {
    $xml .= '<dates>';
    $xml .=  (!empty($node->biblio_year)) ? '<year><style  face="normal" font="default" size="100%">'. htmlspecialchars($node->biblio_year)."</style></year>":'';
    $xml .=  (!empty($node->biblio_date)) ? '<pub-dates><date><style  face="normal" font="default" size="100%">'. htmlspecialchars($node->biblio_date)."</style></date></pub-dates>":'';
    $xml .= "</dates>";
  }
  unset($node->biblio_year);
  unset($node->biblio_date);
  return $xml;
}

function en8_add_keywords(&$node) {
  $kw_array = array();
  $xml = '';
  if (!empty($node->biblio_keywords)) {
    foreach($node->biblio_keywords as $term) {
      $kw_array[] = trim($term);
    }
  }
  if (!empty($kw_array)) {
    $kw_array = array_unique($kw_array);
    $xml .= '<keywords>';
    foreach($kw_array as $word) {
      $xml .= '<keyword><style  face="normal" font="default" size="100%">'. htmlspecialchars(trim($word)) . "</style></keyword>";
    }
    $xml .= "</keywords>";
  }
  unset($node->biblio_keywords);
  return $xml;
}

function en8_add_contributors(&$node) {
  $xml .= '<contributors>';
  if (count($node->biblio_contributors[1])) {
    $xml .= "<authors>";
    foreach((array)$node->biblio_contributors[1] as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= htmlspecialchars(trim($auth['name'])); // insert author here.
      $xml .= "</style></author>";
    }
    $xml .= "</authors>";
  }
  if (count($node->biblio_contributors[2])) {
    $xml .= "<secondary-authors>";
    foreach((array)$node->biblio_contributors[2] as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= htmlspecialchars(trim($auth['name'])); // insert author here.
      $xml .= "</style></author>";
    }
    $xml .= "</secondary-authors>";
  }
  if (count($node->biblio_contributors[3])) {
    $xml .= "<tertiary-authors>";
    foreach((array)$node->biblio_contributors[3] as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= htmlspecialchars(trim($auth['name'])); // insert author here.
      $xml .= "</style></author>";
    }
    $xml .= "</tertiary-authors>";
  }
  if (count($node->biblio_contributors[4])) {
    $xml .= "<subsidiary-authors>";
    foreach((array)$node->biblio_contributors[4] as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= htmlspecialchars(trim($auth['name'])); // insert author here.
      $xml .= "</style></author>";
    }
    $xml .= "</subsidiary-authors>";
  }
  if (count($node->biblio_contributors[5])) {
    $xml .= "<translated-authors>";
    foreach((array)$node->biblio_contributors[5] as $auth) {
      $xml .= '<author><style face="normal" font="default" size="100%">';
      $xml .= htmlspecialchars(trim($auth['name'])); // insert author here.
      $xml .= "</style></author>";
    }
    $xml .= "</translated-authors>";
  }
  $xml .= '</contributors>';
  unset($node->biblio_contributors);
  return $xml;
}

function en8_field_map($biblio_field) {
  static $fmap = array();
  if(empty($fmap)) {
    $fmap = array(
       'source-app'               => '',
       'rec-number'               => '',
       'ref-type'                 => 'biblio_type',
       'auth-address'             => 'biblio_auth_address',
       'auth-affiliaton'          => '',
       'title'                    => 'title',
       'secondary-title'          => 'biblio_secondary_title',
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
  return ($en8_field = array_search($biblio_field, $fmap)) ? $en8_field : '';
}

function _endnote8_type_map($bibliotype) {
  static $map = array();
  if (empty($map)) {
    module_load_include('inc', 'biblio', 'biblio.type.mapper');
    $map = biblio_get_type_map('endnote8');
  }
  return ($en8_type = array_search($bibliotype, $map)) ? $en8_type : 13; //return the biblio type or 129 (Misc) if type not found
}