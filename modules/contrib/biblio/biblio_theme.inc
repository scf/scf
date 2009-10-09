<?php
// $Id: biblio_theme.inc,v 1.8.2.87 2009/09/30 15:48:25 rjerome Exp $
/**
 *
 *   Copyright (C) 2006-2008  Ron Jerome
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
 ****************************************************************************/
module_load_include('inc','biblio','biblio.pages');

/**
 * @param $element
 * @return unknown_type
 */
function theme_biblio_coin_button($element) {
  return '<a href="/biblio/regen_coins"><input type="button"  name="'. $element['#name'] .'" value="'. $element['#value'] .'"  /></a>';
}

/**
 * @param $openURL
 * @return unknown_type
 */
function theme_biblio_openurl($openURL) {
  $openurl_image = variable_get('biblio_openurlimage', '');
  if ($openurl_image != '') {
    $output .= '<div class="biblio-openurl-image"><a href="'. $openURL .'"target=_blank><img border="0" src="'. $openurl_image .'" /></a></div>';
  }
  else {
    $output .= '<div class="biblio-openurl-text"><a href="'. $openURL .'" target=_blank>'. t('Find It Via OpenURL!') .'</a></div>';
  }
  return $output;
}

/**
 * @param $node
 * @return unknown_type
 */
function biblio_openURL($node) {
  global $user;
  $openURL = '';
  // Copyright:          Matthias Steffens <mailto:refbase@extracts.de> and the file's
  //                     original author.
  // Original Author:    Richard Karnesky <mailto:karnesky@gmail.com>  //
  // Adapted for biblio: Ron Jerome
  //  global $openURLResolver; // these variables are defined in 'ini.inc.php'
  //  global $hostInstitutionAbbrevName;
  $openURLResolver = !empty($user->biblio_baseopenurl)?$user->biblio_baseopenurl:'';
  if (empty($openURLResolver)) $openURLResolver = variable_get('biblio_baseopenurl', '');
  if (empty($openURLResolver)) {
    $co = biblio_contextObject($node);

    $sid = "biblio:". variable_get('site_name', 'Drupal');
    $user_sid = !empty($user->biblio_openurl_sid)?$user->biblio_openurl_sid:'';
    $co["sid"] = !empty($user_sid) ? $user_sid : variable_get('biblio_openurl_sid', $sid);

    $openURL = $openURLResolver;
    if (!ereg("\?", $openURLResolver))
    $openURL .= "?";
    else
    $openURL .= "&amp;";
    $openURL .= "ctx_ver=Z39.88-2004";
    foreach ($co as $coKey => $coValue) {
      $coKey = ereg_replace("rft.", "", $coKey);
      $coKey = ereg_replace("au[0-9]*", "au", $coKey);
      $openURL .= "&amp;". $coKey ."=". rawurlencode($coValue);
    }
  }
  return $openURL;
}


/**
 * DEPRECIATED! this was the original output format which is not to flexable it will be removed
 * TODO: remove this function
 * @param $node
 * @param $base
 * @param $style
 * @return unknown_type
 */
function theme_biblio_long($node, $base = 'biblio', $style = 'classic') {
  if (module_exists('popups')){
     popups_add_popups();
  }
  if (empty ($node->biblio_coins))
    biblio_coins_generate($node);
  $output .= filter_xss($node->biblio_coins, array('span'));
  $layout = variable_get('biblio_node_layout', 'orig');
  if (variable_get('biblio_node_layout', 'orig') == 'ft' && strlen(trim($node->body)) && user_access('view full text')) {
    $output .= '<div class="biblio-head">'. theme('biblio_style', $node, $base, $style) .'</div>';
    $annotation_field = variable_get('biblio_annotations', 'none');
    if ($annotation_field != 'none' && $node-> $annotation_field) {
      $output .= '<div class="biblio-annotation">';
      $output .= check_markup($node->$annotation_field, $node->format, FALSE);
      $output .= '</div>';
    }
    $output .= check_markup($node->body, $node->format, FALSE);
    if (biblio_access('export', $node)) {
      $output .= theme('biblio_export_links', $node);
    }
    return $output;
  }
  foreach ((array)$node->biblio_contributors[1] as $auth) {
    $authors[] = theme('biblio_author_link', $auth['name'], $auth['cid'], $base);
  }
  $authors = implode('; ', (array)$authors);
  $openurl_base = variable_get('biblio_baseopenurl', '');
  if ($openurl_base)
    $output .= theme('biblio_openurl', biblio_openurl($node, $openurl_base));
  $output .= '<div class="biblio_type"><h3>'. t("Publication Type") .":</h3> $node->biblio_type_name</div>\n";
  $output .= '<div class="biblio_authors"><h3>'. t("Authors") .':</h3> '. $authors ."</div>\n";
  $output .= '<div class="biblio_source"><h3>'. t("Source") .': </h3> ';
  $source = null;
  if ($node->biblio_secondary_title)
    $source .= check_plain($node->biblio_secondary_title);
  if ($node->biblio_publisher) {
    $source .= $source ? ", " : "";
    $source .= check_plain($node->biblio_publisher);
  }
  if ($node->biblio_volume) {
    $source .= $source ? ", " : "";
    $source .= t('Volume') .' '. check_plain($node->biblio_volume);
  }
  if ($node->biblio_issue) {
    $source .= $source ? ", " : "";
    $source .= t('Issue') . ' '.check_plain($node->biblio_issue);
  }
  if ($node->biblio_number) {
    $source .= $source ? ", " : "";
    $source .= t('Number') . ' '.check_plain($node->biblio_number);
  }
  if ($node->biblio_place_published) {
    $source .= $source ? ", " : "";
    $source .= check_plain($node->biblio_place_published);
  }
  if ($node->biblio_pages) {
    $source .= $source ? ", " : "";
    $source .= 'p.'. check_plain($node->biblio_pages);
  }
  if (isset ($node->biblio_year)) {
    $node->biblio_year = _biblio_text_year($node->biblio_year);
    $source .= ' ('. check_plain($node->biblio_year) .')';
  }
  $output .= "$source</div>\n";
  if ($node->biblio_isbn)
    $output .= '<h3>'. t("ISBN") .':</h3> '. check_plain($node->biblio_isbn) ."\n";
  if ($node->biblio_call_number)
    $output .= '<h3>'. t("Call Number") .':</h3> '. check_plain($node->biblio_call_number) ."\n";
  if ($node->biblio_accession_number)
    $output .= '<h3>'. t("Accession Number") .':</h3> '. check_plain($node->biblio_accession_number) ."\n";
  if ($node->biblio_other_number)
    $output .= '<h3>'. t("Other Number") .':</h3> '. check_plain($node->biblio_other_number) ."\n";
  if ($node->biblio_url) {
    $attrib = (variable_get('biblio_links_target_new_window', null)) ? array(
      'target' => '_blank'
    ) : array();
    $output .= '<h3>'. t("URL") .':</h3>'. l($node->biblio_url, $node->biblio_url, $attrib) ."\n";
  }
  if (!empty($node->biblio_keywords)) {
    $output .= '<h3>'. t("Keywords") .':</h3> '. _biblio_keyword_links( $node->biblio_keywords, $base) ."\n";
  }
  if ($node->biblio_abst_e)
    $output .= '<h3>'. t("Abstract") .':</h3> '. check_markup($node->biblio_abst_e, $node->format, FALSE) ."\n";
  if ($node->biblio_abst_f)
    $output .= '<p>'. check_markup($node->biblio_abst_f, $node->format, FALSE) ."\n";
  if ($node->biblio_notes)
    $output .= '<h3>'. t("Notes") .': </h3>'. check_markup($node->biblio_notes, $node->format, FALSE) ."\n";
  if ( strlen(trim($node->body)) && user_access('view full text') ) {
       $output .= '<h3>'.t('Full Text').':</h3>'.check_markup($node->body, $node->format, FALSE);
  }

  return $output;
}
/**
 * @param $node
 * @param $base
 * @param $teaser
 * @return unknown_type
 */
function theme_biblio_tabular($node, $base = 'biblio', $teaser = false) {

  if (module_exists('popups')){
     popups_add_popups();
  }
  $tid = $node->biblio_type;
  $style_name = biblio_get_style();
  $style_function = "biblio_style_$style_name"."_author_options";
  module_load_include('inc','biblio',"biblio_style_$style_name");
  $fields = _biblio_get_field_information($node->biblio_type, TRUE);
  $rows[] = array(
    array('data' => t('Title'), 'class' => 'biblio-row-title'),
    array('data' => filter_xss($node->title, biblio_get_allowed_tags()))
  );
  $rows[] = array(
    array('data' => t('Publication Type'), 'class' => 'biblio-row-title'),
    array('data' => $node->biblio_type_name)
  );

  if ($node->biblio_url) {
    $attrib = (variable_get('biblio_links_target_new_window', null)) ? array('target' => '_blank') : array();
    $node->biblio_url = l($node->biblio_url, $node->biblio_url, $attrib);
  }
  if ($node->biblio_doi) {
    $doi_url = '';
    $attrib = (variable_get('biblio_links_target_new_window', null)) ? array('target' => '_blank') : array();
    if ( ($doi_start = strpos($node->biblio_doi, '10.')) !== FALSE) {
      $doi = substr($node->biblio_doi, $doi_start);
      $doi_url .= 'http://dx.doi.org/'. $doi;
    }
    $node->biblio_doi = l($node->biblio_doi, $doi_url, $attrib);
  }

  foreach ($fields as $key => $row) {
    // handling the contributor categories like any other field orders them correctly by weight
    if ($row['type'] == 'contrib_widget' && !empty($node->biblio_contributors[$row['fid']][0]['name']) ) {
      $author_options = $style_function();
      $author_options['numberOfAuthorsTriggeringEtAl'] = 100; //set really high so we see all authors
      $data = theme('biblio_format_authors', $node->biblio_contributors[$row['fid']], $author_options, $inline);
    }
    else if (empty ($node->$row['name']) || $row['name'] == 'biblio_coins') continue;
    else {
      switch ($row['name']) {
        case 'biblio_keywords' :
         $data = _biblio_keyword_links($node->$row['name'], $base);
          break;
        case 'biblio_url' :
        case 'biblio_doi' :
          // check_plain is not need on these since they have gone through
          // the l() function which does a check_plain
          $data = $node-> $row['name'];
          break;
        default :
          if ($row['type'] == 'textarea') {
            $data = check_markup($node-> $row['name'], $node->format, FALSE);
          }
          else {
            $data = check_plain($node-> $row['name']);
          }
      }
    }
    $rows[] = array(
      array(
        'data' => $row['title'],
        'class' => 'biblio-row-title'
      ),
      array(
        'data' => $data
      )
    );
  }


  if (strlen(trim($node->body)) && user_access('view full text')) {
    $rows[] = array(
        array('data' => t('Full Text'),  'valign' => 'top'),
        array('data' =>  check_markup($node->body, $node->format, FALSE))
    );

  }
  $output = '<div id="biblio-node">';
  $output .= filter_xss($node->biblio_coins, array('span'));
  $header = array();
  $output .= theme('table', $header, $rows);
  $output .= '</div>';
  return $output;
}

function theme_biblio_authors($contributors, $style = 'classic', $cat = 1, $inline = false, $glue = ', ') {
  if (empty($contributors)) return; // t('No author information available');
  $author_style_function = '_'.$style.'_format_author';
  $author_links = variable_get('biblio_author_links', 1);
  $base  = variable_get('biblio_base', 'biblio');
  foreach ($contributors as $key => $author) {
    if (!empty($author['name']) && !isset($author['lastname'])) {
      $author = biblio_parse_author($author, $cat); // this is needed for form preview to fill in all fields
    }
    if (strlen($author['name'])) {
      $format = $author_style_function($author);
      if ($author_links) $format = theme('biblio_author_link', $format, $author['cid'], $base, $inline);
      // add the auth_type as css id to allow later formatting
      $author_array[] = '<span id="'.$author['auth_type'].'">'.$format.'</span>';
    }
  }
  if (empty($author_array)) return '';

  $output  = '<span class="biblio_authors">';
  $output .= implode($glue, $author_array);
  $output .= '</span>';
  return $output;
}
function _biblio_get_latin1_regex() {
  $alnum = "[:alnum:]ÄÅÁÀÂÃÇÉÈÊËÑÖØÓÒÔÕÜÚÙÛÍÌÎÏÆäåáàâãçéèêëñöøóòôõüúùûíìîïæÿß";

  // Matches ISO-8859-1 letters:
  $alpha = "[:alpha:]ÄÅÁÀÂÃÇÉÈÊËÑÖØÓÒÔÕÜÚÙÛÍÌÎÏÆäåáàâãçéèêëñöøóòôõüúùûíìîïæÿß";

  // Matches ISO-8859-1 control characters:
  $cntrl = "[:cntrl:]";

  // Matches ISO-8859-1 dashes & hyphens:
  $dash = "-–";

  // Matches ISO-8859-1 digits:
  $digit = "[\d]";

  // Matches ISO-8859-1 printing characters (excluding space):
  $graph = "[:graph:]ÄÅÁÀÂÃÇÉÈÊËÑÖØÓÒÔÕÜÚÙÛÍÌÎÏÆäåáàâãçéèêëñöøóòôõüúùûíìîïæÿß";

  // Matches ISO-8859-1 lower case letters:
  $lower = "[:lower:]äåáàâãçéèêëñöøóòôõüúùûíìîïæÿß";

  // Matches ISO-8859-1 printing characters (including space):
  $print = "[:print:]ÄÅÁÀÂÃÇÉÈÊËÑÖØÓÒÔÕÜÚÙÛÍÌÎÏÆäåáàâãçéèêëñöøóòôõüúùûíìîïæÿß";

  // Matches ISO-8859-1 punctuation:
  $punct = "[:punct:]";

  // Matches ISO-8859-1 whitespace (separating characters with no visual representation):
  $space = "[\s]";

  // Matches ISO-8859-1 upper case letters:
  $upper = "[:upper:]ÄÅÁÀÂÃÇÉÈÊËÑÖØÓÒÔÕÜÚÙÛÍÌÎÏÆ";

  // Matches ISO-8859-1 "word" characters:
  $word = "_[:alnum:]ÄÅÁÀÂÃÇÉÈÊËÑÖØÓÒÔÕÜÚÙÛÍÌÎÏÆäåáàâãçéèêëñöøóòôõüúùûíìîïæÿß";

  // Defines the PCRE pattern modifier(s) to be used in conjunction with the above variables:
  // More info: <http://www.php.net/manual/en/reference.pcre.pattern.modifiers.php>
  $patternModifiers = "";

  return array($alnum, $alpha, $cntrl, $dash, $digit, $graph, $lower,
       $print, $punct, $space, $upper, $word, $patternModifiers);

}
/*
 * Helper function for theme_biblio_format_authors() and theme_biblio_page_number()
 */
function _biblio_get_utf8_regex() {

    // Matches Unicode letters & digits:
  $alnum = "\p{Ll}\p{Lu}\p{Lt}\p{Lo}\p{Nd}"; // Unicode-aware equivalent of "[:alnum:]"

  // Matches Unicode letters:
  $alpha = "\p{Ll}\p{Lu}\p{Lt}\p{Lo}"; // Unicode-aware equivalent of "[:alpha:]"

  // Matches Unicode control codes & characters not in other categories:
  $cntrl = "\p{C}"; // Unicode-aware equivalent of "[:cntrl:]"

  // Matches Unicode dashes & hyphens:
  $dash = "\p{Pd}";

  // Matches Unicode digits:
  $digit = "\p{Nd}"; // Unicode-aware equivalent of "[:digit:]"

  // Matches Unicode printing characters (excluding space):
  $graph = "^\p{C}\t\n\f\r\p{Z}"; // Unicode-aware equivalent of "[:graph:]"

  // Matches Unicode lower case letters:
  $lower = "\p{Ll}"; // Unicode-aware equivalent of "[:lower:]"

  // Matches Unicode printing characters (including space):
  $print = "\P{C}"; // same as "^\p{C}", Unicode-aware equivalent of "[:print:]"

  // Matches Unicode punctuation (printing characters excluding letters & digits):
  $punct = "\p{P}"; // Unicode-aware equivalent of "[:punct:]"

  // Matches Unicode whitespace (separating characters with no visual representation):
  $space = "\t\n\f\r\p{Z}"; // Unicode-aware equivalent of "[:space:]"

  // Matches Unicode upper case letters:
  $upper = "\p{Lu}\p{Lt}"; // Unicode-aware equivalent of "[:upper:]"

  // Matches Unicode "word" characters:
  $word = "_\p{Ll}\p{Lu}\p{Lt}\p{Lo}\p{Nd}"; // Unicode-aware equivalent of "[:word:]" (or "[:alnum:]" plus "_")

  // Defines the PCRE pattern modifier(s) to be used in conjunction with the above variables:
  // More info: <http://www.php.net/manual/en/reference.pcre.pattern.modifiers.php>
  $patternModifiers = "u"; // the "u" (PCRE_UTF8) pattern modifier causes PHP/PCRE to treat pattern strings as UTF-8

  return array($alnum, $alpha, $cntrl, $dash, $digit, $graph, $lower,
       $print, $punct, $space, $upper, $word, $patternModifiers);
}

function _biblio_get_regex_patterns() {
  // Checks if PCRE is compiled with UTF-8 and Unicode support
  if (!@preg_match('/\pL/u', 'a')) {
    // probably a broken PCRE library
    return _biblio_get_latin1_regex();
  } else {
    // Unicode safe filter for the value
    return _biblio_get_utf8_regex();
  }
}
function theme_biblio_format_authors($contributors, $options, $inline = false)
{
  if (empty($contributors)) return;
  list($alnum, $alpha, $cntrl, $dash, $digit, $graph, $lower,
       $print, $punct, $space, $upper, $word, $patternModifiers) = _biblio_get_regex_patterns();
  $base  = variable_get('biblio_base', 'biblio');
  $author_links = variable_get('biblio_author_links', 1);

  $authorCount = count($contributors); // check how many authors we have to deal with
  $output = ""; // this variable will hold the final author string
  $includeStringAfterFirstAuthor = false;

  if (empty($options['numberOfAuthorsTriggeringEtAl']))  $options['numberOfAuthorsTriggeringEtAl'] = $authorCount;

  if (empty($options['includeNumberOfAuthors']))  $options['includeNumberOfAuthors'] = $authorCount;

  foreach($contributors as $rank => $author)
  {
    if (empty($author['name'])) continue;
    if (!isset($author['lastname'])) {
      module_load_include('inc','biblio',"biblio.contributors");
      $author = biblio_parse_author($author, $author['auth_type']); // this is needed for form preview to fill in all fields
    }

    if (!empty($author['firstname'])) {
      if ($options['shortenGivenNames']) // if we're supposed to abbreviate given names
      {
        // within initials, reduce all full first names (-> defined by a starting uppercase character, followed by one ore more lowercase characters)
        // to initials, i.e., only retain their first character
        $author['firstname'] = preg_replace("/([$upper])[$lower]+/$patternModifiers", '\\1', $author['firstname']);
        //$author['firstname'] = drupal_substr($author['firstname'], 0, 1);
        // the next line caused extra betweenInitialsDelim to appear in some circumstances
        //$author['firstname'] = preg_replace("/($space|$dash)?/$patternModifier", $options['betweenInitialsDelim'], $author['firstname']);
      }
    }
    if (isset($author['initials'])) {
      // within initials, remove any dots:
      $author['initials'] = preg_replace("/([$upper])\.+/$patternModifiers", "\\1", $author['initials']);
      //$author['initials'] = str_replace('.', '',  $author['initials']);

      // within initials, remove any spaces *between* initials:
      $author['initials'] = preg_replace("/(?<=[-$upper]) +(?=[-$upper])/$patternModifiers", "", $author['initials']);
      //$author['initials'] = str_replace(' ', '',  $author['initials']);

      // within initials, add a space after a hyphen, but only if ...
      if (ereg(" $", $options['betweenInitialsDelim'])) // ... the delimiter that separates initials ends with a space
      $author['initials'] = preg_replace("/-(?=[$upper])/$patternModifiers", "- ", $author['initials']);

      // then, separate initials with the specified delimiter:
      $delim = $options['betweenInitialsDelim'];
      $author['initials'] = preg_replace("/([$upper])(?=[^$lower]+|$)/$patternModifiers", "\\1$delim", $author['initials']);

      $shortenInitials = (isset($options['numberOfInitialsToKeep'])) ? $options['numberOfInitialsToKeep'] : FALSE;
      if ($shortenInitials) $author['initials'] = drupal_substr($author['initials'], 0, $shortenInitials);

      if ($options['shortenGivenNames'] && !empty($author['firstname'])) {
        $author['firstname'] = $author['firstname'].$options['betweenInitialsDelim'].$author['initials'];
        if ($shortenInitials) $author['firstname'] = drupal_substr($author['firstname'], 0, $shortenInitials);
      }
      elseif(!empty($author['firstname'])) {
        $author['firstname'] = $author['firstname'].' '.$author['initials'];
      }
      elseif(empty($author['firstname'])) {
        $author['firstname'] = $author['initials'];
      }
    }

    // if there is a surname prefix like "van", "von" etc, stick it back before the last name.
    if (!empty($author['prefix'])) $author['lastname'] = $author['prefix'] .' '. $author['lastname'];

    if (!empty($author['firstname'])) {
      if ($rank == 0) { // -> first author
        if($options['initialsBeforeAuthorFirstAuthor']) {
          $singleAuthorString = $author['firstname'].$options['AuthorsInitialsDelimFirstAuthor'].$author['lastname'];
        }
        else{
          $singleAuthorString = $author['lastname'].$options['AuthorsInitialsDelimFirstAuthor'].$author['firstname'];
        }
      }
      else { // $rank > 0 // -> all authors except the first one
        if($options['initialsBeforeAuthorStandard']) {
          $singleAuthorString = $author['firstname'].$options['AuthorsInitialsDelimStandard'].$author['lastname'];
        }
        else{
          $singleAuthorString = $author['lastname'].$options['AuthorsInitialsDelimStandard'].$author['firstname'];
        }
      }
    }
    else {
      $singleAuthorString = $author['lastname'];
    }
    if ($author_links) {
      $singleAuthorString = theme('biblio_author_link',$singleAuthorString, $author['cid'], $base, $inline);
    }
    else {
      $singleAuthorString = check_plain($singleAuthorString);
    }
    // append this author to the final author string:
    if (($rank == 0) OR ($rank + 1) < $authorCount) // -> first author, or (for multiple authors) all authors except the last one
    {
      if ($rank == 0) // -> first author
      $output .= $singleAuthorString;
      else // -> for multiple authors, all authors except the first or the last one
      $output .= $options['BetweenAuthorsDelimStandard'] . $singleAuthorString;

      // we'll append the string in '$customStringAfterFirstAuthors' to the number of authors given in '$includeNumberOfAuthors' if the total number of authors is greater than the number given in '$numberOfAuthorsTriggeringEtAl':
      if ((($rank + 1) == $options['includeNumberOfAuthors']) AND ($authorCount > $options['numberOfAuthorsTriggeringEtAl']))
      {
        if (ereg("__NUMBER_OF_AUTHORS__", $options['customStringAfterFirstAuthors']))
        $customStringAfterFirstAuthors = preg_replace("/__NUMBER_OF_AUTHORS__/", ($authorCount - $options['includeNumberOfAuthors']), $options['customStringAfterFirstAuthors']); // resolve placeholder

        $includeStringAfterFirstAuthor = true;
        break;
      }
    }
    elseif (($authorCount > 1) AND (($rank + 1) == $authorCount)) // -> last author (if multiple authors)
    {
      $output .= $options['BetweenAuthorsDelimLastAuthor'] . $singleAuthorString;
    }
  }

  // do some final clean up:
  if ($options['encodeHTML'])
  //$output = encodeHTML($output); // HTML encode higher ASCII characters within the newly arranged author contents

  if ($includeStringAfterFirstAuthor)
  $output .= $options['customStringAfterFirstAuthors']; // the custom string won't get HTML encoded so that it's possible to include HTML tags (such as '<i>') within the string

  $output = preg_replace("/  +/", " ", $output); // remove double spaces (which occur e.g., when both, $betweenInitialsDelim & $newAuthorsInitialsDelim..., end with a space)
  $output = preg_replace("/ +([,.;:?!()]|$)/", "\\1", $output); // remove excess spaces before [,.;:?!()] and from the end of the author string

  return $output;
}

function theme_biblio_author_link($author, $aid, $base='biblio', $inline = FALSE) {
  $options = array();
  $inline = $inline ? "/inline" : "";

  if (isset($_GET['sort'])) {
    $options['query']  .= "sort=" . $_GET['sort'];
  }
  if (isset($_GET['order'])) {
    $options['query']   .= $options['query'] ? "&" : "" ;
    $options['query']   .= "order=" . $_GET['order'];
  }

  $html = l(trim($author), "$base/author/$aid".$inline,$options );

  return $html;
}

// Format page information:
//
// NOTES: - this function (and refbase in general) assumes following rules for the original formatting of page information in '$origPageInfo':
//          - single-page items are given as a page range with identical start & end numbers (e.g. "127-127")
//          - multi-page items are given as a page range where the end number is greater than the start number (e.g. "127-132")
//          - for multi-page items where only the start page is known, a hyphen is appended to the start page (e.g. "127-")
//          - total number of pages are given with a "pp" suffix (e.g. "498 pp"), see TODO
//          - the given page info is left as is if it does not match any of the above rules (e.g. a single page number is ambiguous since it
//            could mean a single page or the total number of pages)
//        - the function attempts to deal with page locators that contain letters (e.g. "A1 - A3" or "4a-4c") but, ATM, locator parts (e.g. "A1")
//          must contain at least one digit character & must not contain any whitespace
//
// TODO:  - should we only use Unicode-aware regex expressions (i.e. always use '$space', '$digit' or '$word' instead of ' ', '\d' or '\w', etc)?
//        - recognize & process total number of pages
//        - for '$shortenPageRangeEnd=true', add support for page locators that contain letters (e.g. "A1 - A3" or "4a-4c")
function theme_biblio_page_number($origPageInfo, $pageRangeDelim = "-", $singlePagePrefix = "", $pageRangePrefix = "", $totalPagesPrefix = "", $singlePageSuffix = "", $pageRangeSuffix = "", $totalPagesSuffix = "", $shortenPageRangeEnd = false)
{
  list($alnum, $alpha, $cntrl, $dash, $digit, $graph, $lower,
       $print, $punct, $space, $upper, $word, $patternModifiers) = _biblio_get_regex_patterns();

  // Check original page info for any recognized page locators, and process them appropriately:
  if (preg_match("/\w*\d+\w* *[$dash]+ *(?:\w*\d+\w*)?/$patternModifiers", $origPageInfo)) // the original page info contains a page range (like: "127-127", "127-132", "A1 - A3", "4a-4c", or "127-" if only start page given)
  {
    // Remove any whitespace around dashes or hyphens that indicate a page range:
    $origPageInfo = preg_replace("/(\w*\d+\w*) *([$dash]+) *(\w*\d+\w*)?(?=[^\w\d]|$)/$patternModifiers", "\\1\\2\\3", $origPageInfo);

    // Split original page info into its functional parts:
    // NOTE: ATM, we simply split on any whitespace characters, then process all parts with page ranges
    //       (this will also reduce runs of whitespace to a single space)
    $partsArray = preg_split("/ +/", $origPageInfo);
    $partsCount = count($partsArray);

    for ($i=0; $i < $partsCount; $i++)
    {
      // Format parts with page ranges:
      // - single-page item:
      if (preg_match("/(\w*\d+\w*)[$dash]+\\1(?=[^\w\d]|$)/$patternModifiers", $partsArray[$i])) // this part contains a page range with identical start & end numbers (like: "127-127")
      $partsArray[$i] = preg_replace("/(\w*\d+\w*)[$dash]+\\1(?=[^\w\d]|$)/$patternModifiers", $singlePagePrefix . "\\1" . $singlePageSuffix, $partsArray[$i]);

      // - multi-page item:
      elseif (preg_match("/\w*\d+\w*[$dash]+(?:\w*\d+\w*)?(?=[^\w\d]|$)/$patternModifiers", $partsArray[$i])) // this part contains a page range (like: "127-132", or "127-" if only start page given)
      {
        // In case of '$shortenPageRangeEnd=true', we abbreviate ending page numbers so that digits aren't repeated unnecessarily:
        if ($shortenPageRangeEnd AND preg_match("/\d+[$dash]+\d+/$patternModifiers", $partsArray[$i])) // ATM, only digit-only page locators (like: "127-132") are supported
        {
          // NOTE: the logic of this 'if' clause doesn't work if the original page info contains something like "173-190; 195-195" (where, for the first page range, '$endPage' would be "190;" and not "190")
          list($startPage, $endPage) = preg_split("/[$dash]+/$patternModifiers", $partsArray[$i]);

          $countStartPage = strlen($startPage);
          $countEndPage = strlen($endPage);

          if(($countStartPage == $countEndPage) AND ($startPage < $endPage))
          {
            for ($j=0; $j < $countStartPage; $j++)
            {
              if (preg_match("/^" . substr($startPage, $j, 1) . "/", $endPage)) // if the ending page number has a digit that's identical to the starting page number (at the same digit offset)
              $endPage = substr($endPage, 1); // remove the first digit from the remaining ending page number
              else
              break;
            }
          }

          $partsArray[$i] = $pageRangePrefix . $startPage . $pageRangeDelim . $endPage . $pageRangeSuffix;
        }
        else // don't abbreviate ending page numbers:
        $partsArray[$i] = preg_replace("/(\w*\d+\w*)[$dash]+(\w*\d+\w*)?(?=[^\w\d]|$)/$patternModifiers", $pageRangePrefix . "\\1" . $pageRangeDelim . "\\2" . $pageRangeSuffix, $partsArray[$i]);
      }
    }

    $newPageInfo = join(" ", $partsArray); // merge again all parts
  }
  else
  $newPageInfo = $origPageInfo; // page info is ambiguous, so we don't mess with it

  return $newPageInfo;
}


/**
 * Applies a "style" function to a single node.
 *
 * @param $node A node
 * @param $base The base url for biblio (defaults to /biblio)
 * @param $style_name The name of the style to apply
 * @param $inline "inline" mode returns the raw HTML rather than letting drupal render the whole page.
 * @return A string containing the styled (HTML) node
 */
function theme_biblio_style(& $node, $base = 'biblio', $style_name = 'classic', $inline = false) {
  module_load_include('inc','biblio',"biblio_style_$style_name");

  $style_function = "biblio_style_$style_name";
  $styled_node = $style_function ($node, $base, $inline);
  if (empty ($node->biblio_coins))
    biblio_coins_generate($node);
  return ($styled_node . filter_xss($node->biblio_coins, array('span')));
}

/**
 * @param $node
 * @param $base
 * @param $style
 * @param $inline
 * @return unknown_type
 */
function theme_biblio_entry($node, $base = 'biblio', $style = 'classic', $inline = false) {
  $output  = "\n".'<div class="biblio-entry">'."\n" ;

  // first add the styled entry...
  $output .= theme('biblio_style', $node, $base, $style, $inline);

  // now add the various links
  if ($node->biblio_abst_e) {
    $output .= '<span class="biblio-abstract-link">';
    $output .= l(" Abstract", "node/$node->nid") ."\n";
    $output .= '</span>';
  }
  $annotation_field = variable_get('biblio_annotations', 'none');
  if ($annotation_field != 'none' && $node-> $annotation_field) {
    $output .= '<div class="biblio-annotation">';
    $output .= check_markup($node->$annotation_field, $node->format, FALSE);
    $output .= '</div>';
  }
  $openurl_base = variable_get('biblio_baseopenurl', '');

  if ($openurl_base){
    $output .= theme('biblio_openurl', biblio_openurl($node));
  }

  if (biblio_access('export', $node)) {
    $base = variable_get('biblio_base', 'biblio');
    $output .= theme('biblio_export_links',$node);
  }

  if (biblio_access('download', $node)) {
    // add links to attached files (if any)
    $output .= theme('biblio_download_links',$node);
  }
  $output .= "\n</div>";

  return $output;
}

/**
 * @param $form
 * @return unknown_type
 */
function theme_biblio_filters(& $form) {
  if (sizeof($form['current'])) {
    $output .= '<ul>';
    foreach (element_children($form['current']) as $key) {
      $output .= '<li>'. drupal_render($form['current'][$key]) .'</li>';
    }
    $output .= '</ul>';
  }
  $output .= '<dl class="multiselect">'. (sizeof($form['current']) ? '<dt><em>'. t('and') .'</em> '. t('where') .'</dt>' : '') .'<dd class="a">';
  foreach (element_children($form['filter']) as $key) {
    $output .= drupal_render($form['filter'][$key]);
  }
  $output .= '</dd>';
  $output .= '<dt>'. t('is') .'</dt><dd class="b">';
  foreach (element_children($form['status']) as $key) {
    $output .= drupal_render($form['status'][$key]);
  }
  $output .= '</dd>';
  $output .= '</dl>';
  $output .= '<div class="container-inline" id="node-buttons">'. drupal_render($form['buttons']) .'</div>';
  $output .= '<br class="clear" />';
  return $output;
}

/**
 * @param $form
 * @return unknown_type
 */
function theme_biblio_form_filter(& $form) {
  $output .= '<div id="biblio-admin-filter">';
  $output .= drupal_render($form['filters']);
  $output .= '</div>';
  $output .= drupal_render($form);
  return $output;
}

/**
 * @param $form
 * @return unknown_type
 */
function theme_biblio_admin_types_edit_form($form) {
  drupal_add_tabledrag('field-table', 'order', 'sibling', 'weight',NULL,NULL,false);

  $base = variable_get('biblio_base', 'biblio');
  $tid = (!empty ($form['#parameters'][2])) ? $form['#parameters'][2] : FALSE;
  //  drupal_set_title($form['type_name'] ? $form['type_name']['#value'] : t('Common'));

  // build the table with all the fields if no $tid is given, or only the common
  // and customized fields if $tid is given
  $conf_table = array();
  foreach (element_children($form['configured_flds']) as $fld) {
    $form['configured_flds'][$fld]['weight']['#attributes']['class'] = 'weight';

    $conf_row = array();
    $conf_row[] = array('data' => drupal_render($form['configured_flds'][$fld]['name']));
    $conf_row[] = array('data' => drupal_render($form['configured_flds'][$fld]['title']));
    if (isset($form['configured_flds'][$fld]['auth_type'])) {
      $form['configured_flds'][$fld]['hint']['#size'] = 15;
      $conf_row[] = array('data' => drupal_render($form['configured_flds'][$fld]['hint']));
      $conf_row[] = array('data' => drupal_render($form['configured_flds'][$fld]['auth_type']));
    } else {
      $form['configured_flds'][$fld]['hint']['#size'] = ($tid ? 38: 36);
      $conf_row[] = array('data' => drupal_render($form['configured_flds'][$fld]['hint']), 'colspan' => 2);
    }
    foreach (element_children($form['configured_flds'][$fld]['checkboxes']) as $oid) {
      if (is_array($form['configured_flds'][$fld]['checkboxes'])) {
        $conf_row[] = array(
          'data' => drupal_render($form['configured_flds'][$fld]['checkboxes'][$oid]),
          'title' => $oid);
      }
    }
    $conf_row[] = array('data' => drupal_render($form['configured_flds'][$fld]['weight']));
    $conf_table[] = array('data' => $conf_row, 'class' => 'draggable');
  }
  if ($tid) {
    $header = array(t('Field Name'), t('Title'), t('Hint'), '', t('Visible'), t('Required'), t('Weight'));
  }
  else {
    $header = array(t('Field Name'), t('Default Title'), t('Hint'), '', t('Common'), t('Required'), t('Autocomplete'), t('Weight'));
  }
  $output .= '<p>';
  $output .= '<h2>'. drupal_render($form['top_message']) .'</h2>';
  $output .= drupal_render($form['help']);
  $output .= theme('table', $header, $conf_table, array('id' => 'field-table'));
  $output .= '<p><center>'. drupal_render($form['submit']) .'</center></p>';
  $output .= drupal_render($form);
  return $output;
}

function theme_biblio_download_links($node = NULL) {
  $files = '';
  if (!empty ($node->files) && count($node->files) > 0 && user_access('view uploaded files')) {
    $files .= '<span class="biblio_file_links">';
    $files .= '&nbsp;'. t('Download') .':&nbsp;';
    $file_count = 0;
    foreach ($node->files as $file) {
      if ($file->list) {
        $file_count++;
        $href = file_create_url($file->filepath);
        $text = $file->description ? $file->description : $file->filename;
        $files .= l($text, $href) .'&nbsp;('. format_size($file->filesize).')';
      }
    }
    $files .= '</span>';
  }
  if (module_exists('filefield')) { // now lets get any CCK FileField files...
    $fields = filefield_get_field_list('biblio');
    foreach ($fields as $field_name => $field) {
      if (filefield_view_access($field_name)) {
        $field_files = filefield_get_node_files($node, $field_name);
        if ($field_files) {
          foreach ($field_files as $file) {
            if ($file['fid']) {
              $files .= theme('filefield_file', $file);
            }
          }
        }
      }
    }
  }

  return $files;
}

/**
 * Creates a group of links for the various export functions
 * @param $nid the node id to export (if omitted, all nodes in the current view will be exported
 * @return an un-ordered list of class "biblio-export-buttons"
 */
function theme_biblio_export_links($node = NULL) {
  global $pager_total_items;
  $base = variable_get('biblio_base', 'biblio');
  $show_link = variable_get('biblio_export_links', array('tagged' => TRUE, 'xml' => TRUE, 'bibtex' => TRUE));
  $show_link['google'] = variable_get('biblio_google_link', 1);

  $links = '';
  if ($show_link['tagged']) $links .= '<li> '. _build_biblio_link($base, $node, 'tagged') .'</li>';
  if ($show_link['xml'])    $links .= '<li> '. _build_biblio_link($base, $node, 'xml') .'</li>';
  if ($show_link['bibtex']) $links .= '<li> '. _build_biblio_link($base, $node, 'bibtex') .'</li>';
  if ($show_link['google'] && !empty($node))
                            $links .= '<li> '. _build_biblio_link($base, $node, 'google') .'</li>';

  if (!empty($links)) $links = '<ul class="biblio-export-buttons">' . $links . '</ul>';
  if (empty($node) && !empty($links)) {
    $links = t('Export @count results', array('@count' => $pager_total_items[0])).':' . $links;
  }
  return $links;
}

/**
 * Creates a link to export a node (or view) in EndNote Tagged format
 *
 * @param $base this is the base url (defaults to /biblio)
 * @param $nid  the node id, if NULL then the current view is exported
 * @return  a link (<a href=...>Tagged</a>)
 */
function _build_biblio_link($base, $node = NULL, $type = NULL) {
  switch ($type) {
    case 'bibtex':
      $link = _build_biblio_bibtex_link($base, $node->nid);
      break;
    case 'tagged':
      $link = _build_biblio_tagged_link($base, $node->nid);
      break;
    case 'xml':
      $link = _build_biblio_xml_link($base, $node->nid);
      break;
    case 'google':
      $link = _build_google_scholar_link($node);
      break;
  }
  return  l($link['title'], $link['href'], $link);
}

function _build_biblio_tagged_link($base, $nid) {
  if (module_exists('popups') && !empty($nid)) {
    $link = array('href' => "$base/export/tagged/$nid/popup", 'attributes' => array('class' => 'popups', 'title' => t("Click to get the EndNote Tagged output ")));
  } else {
    $link = array('href' => "$base/export/tagged/$nid", 'attributes' => array('title' => t("Click to download the EndNote Tagged formatted file")));
  }
  $link['title'] = t('Tagged');

  return $link;
}
/**
 * Creates a link to export a node (or view) in BibTEX format
 *
 * @param $base this is the base url (defaults to /biblio)
 * @param $nid  the node id, if NULL then the current view is exported
 * @return  a link (<a href=...>BibTEX</a>)
 */
function _build_biblio_bibtex_link($base, $nid = NULL) {
  if (module_exists('popups') && !empty($nid)) {
    $link = array('href' => "$base/export/bibtex/$nid/popup", 'attributes' => array('class' => 'popups', 'title' => t("Click to get the BibTEX output ")));
  } else {
    $link = array('href' => "$base/export/bibtex/$nid", 'attributes' => array('title' => t("Click to download the BibTEX formatted file")));
  }
  $link['title'] = t('BibTex');

  return $link;
}

/**
 * Creates a link to export a node (or view) in XML format
 *
 * @param $base this is the base url (defaults to /biblio)
 * @param $nid  the node id, if NULL then the current view is exported
 * @return  a link (<a href=...>XML</a>)
 */
function _build_biblio_xml_link($base, $nid = NULL) {
//  if (module_exists('popups') && !empty($nid)) {
//    $link = array('href' => "$base/export/xml/$nid/popup", 'attributes' => array('class' => 'popups', 'title' => t("Click to get the XML output ")));
//  } else {
    $link = array('href' => "$base/export/xml/$nid", 'attributes' => array('title' => t("Click to download the XML formatted file")));
//  }
  $link['title'] = t('XML');

  return $link;
}
function _build_google_scholar_link($node) {
  $link  = 'http://scholar.google.com/scholar?as_q=';
  $link .= '"'.str_replace(' ', '+', $node->title).'"';
  $link .= '&btnG=Search+Scholar&as_epq=&as_oq=&as_eq=&as_occt=any&as_sauthors='.$node->biblio_contributors[1][0]['lastname'];

  return array(
        'title'      => t('Google Scholar'),
        'href'       => $link,
        'attributes' => array('title' => t("Click to search Google Scholar for this entry")),
        );
}

/**
 * @param $form
 * @return unknown_type
 */
function theme_biblio_contributors($form) {
  $rows = array();
  if ($form['#hideRole']) $headers = array('', t('Name'), t('Weight'));
  else $headers = array('', t('Name'), t('Role'), t('Weight'));
  drupal_add_tabledrag($form['#id'], 'order', 'sibling', 'rank');

  foreach (element_children($form) as $key) {
    // No need to print the field title every time.
    unset ($form[$key]['name']['#title'], $form[$key]['auth_type']['#title']);
    // Add class to group weight fields for drag and drop.
    $form[$key]['rank']['#attributes']['class'] = 'rank';

    // Build the table row.
    $row = array('');
    $row[] = array('data' => drupal_render($form[$key]['name']),
              		 'class' => 'biblio-contributor');
    if (!$form['#hideRole']) {
      $row[] = array('data' => drupal_render($form[$key]['auth_type']),
      							 'class' => 'biblio-contributor-type');
    }
    $row[] = drupal_render($form[$key]['rank']);
    $rows[] = array('data' => $row, 'class' => 'draggable');
  }
  $output = theme('table', $headers, $rows, array('id' => $form['#id']));
  $output .= drupal_render($form);
  return $output;
}

/**
 * This function creates a string of letters (A - Z), which
 * depending on the sorting are either linked to author or title
 * filters i.e. clicking on the A when in the listing is sorted by
 * authors will bring up a list of all the entries where the first
 * character of the primary authors last name is "A"
 *
 * @param $type either "author or title"
 * @return a chunk of HTML code as described above
 */
function theme_biblio_alpha_line($type = 'author',$current = NULL, $path = NULL) {
  $options = array();
  $base = variable_get('biblio_base', 'biblio');
  $all = '';
  switch ($type){
    case 'authors':
    case 'keywords':
      $path =  (ord(substr($_GET['q'],-1)) > 97) ? $_GET['q'] . "/" : substr($_GET['q'], 0, -1);
      $all = '['.l(t('Show ALL'), substr($_GET['q'], 0, -2)).']' ;
      break;
    case 'author':
      $options['query'] .= 'sort=author';
      $path = "$base/ag/";
      $all_query['query'] = 'sort=author'. ($_GET['order']?'&order='.$_GET['order']:'');
      $all = '['.l(t('Show ALL'), $base,$all_query).']' ;
      break;
    case 'title':
      $options['query'] .= 'sort=title';
      $path = "$base/tg/";
      $all_query['query'] = 'sort=title'. ($_GET['order']?'&order='.$_GET['order']:'');
      $all = '['.l(t('Show ALL'), $base,$all_query).']' ;
      break;
    default:
      if (!isset ($_GET['sort']) || $_GET['sort'] == 'year' || $_GET['sort'] == 'type')
      return;
      $inline = $inline ? "/inline" : "";
      if (isset ($_GET['sort'])) {
        $options['query'] .= "sort=". $_GET['sort'];
        if ($_GET['sort'] == 'author')
        $path = "$base/ag/";
        if ($_GET['sort'] == 'title')
        $path = "$base/tg/";
      }

  }
  if (isset ($_GET['order'])) {
    $options['query'] .= $options['query'] ? "&" : "";
    $options['query'] .= "order=". $_GET['order'];
  }
  $output = '<div class="biblio-alpha-line">';
  for ($i = 65; $i <= 90; $i++) {
    if ($i == ord($current)){
      $output .= '<b>['.chr($i).']</b>&nbsp;';
    }
    else{
      $output .= l(chr($i), $path. chr($i), $options) .'&nbsp;';
    }
  }
  if($current) $output .= '&nbsp;&nbsp;'.$all;
  $output .= '</div>';
  return $output;
}

/**
 * Themes the author editing form
 *
 * @param $form
 * @return rendered form
 */
function theme_biblio_admin_author_edit_form($form){
  $rows = array();
  $header = array();
  $rows[] = array(
            array('data' => drupal_render($form['prefix'])),
            array('data' => drupal_render($form['firstname'])),
            array('data' => drupal_render($form['initials'])),
            array('data' => drupal_render($form['lastname'])),
            array('data' => drupal_render($form['suffix'])));
  $rows[] = array(array('data' => drupal_render($form['name']), 'colspan' => 5));
  $rows[] = array(array('data' => drupal_render($form['affiliation']), 'colspan' => 5));
  $rows[] = array(array('data' => drupal_render($form['drupal_uid']), 'colspan' => 5));

  $output = theme('table', $header, $rows);
  $output .= drupal_render($form['merge']);
  $output .= drupal_render($form['link']);
  $output .= drupal_render($form);

  return $output;

}

function theme_biblio_admin_orphans_form($form) {
  // If there are any orphans, then $form['name'] contains a list of the author names
  $has_items = isset($form['name']) && is_array($form['name']);
  $select_header = $has_items ? theme('table_select_header_cell') : '';
  $header = array($select_header, t('Author Name'), t('Affiliation'));

  $output = '';
  $output .= drupal_render($form['submit']);
  if ($has_items) {
    foreach (element_children($form['name']) as $key) {
      $row = array();
      $row[] = drupal_render($form['authors'][$key]);
      $row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['affiliation'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('There are no orphaned authors in the database.'), 'colspan' => '3'));
  }
  $output .= theme('table', $header, $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}
function theme_biblio_admin_keyword_orphans_form($form) {
  // If there are any orphans, then $form['name'] contains a list of the author names

  $has_items = isset($form['keyword']) && is_array($form['keyword']);
  $select_header = $has_items ? theme('table_select_header_cell') : '';
  $header = array($select_header, t('Keyword'));

  $output = '';
  $output .= drupal_render($form['submit']);
  if ($has_items) {
    foreach (element_children($form['keyword']) as $key) {
      $row = array();
      $row[] = drupal_render($form['keywords'][$key]);
      $row[] = drupal_render($form['keyword'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('There are no orphaned keywords in the database.'), 'colspan' => '2'));
  }
  $output .= theme('table', $header, $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}
