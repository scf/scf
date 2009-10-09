<?php

// $Id: biblio_style_cse.inc,v 1.9.2.22 2009/09/30 15:48:25 rjerome Exp $
/**
 * Get the style information
 *
 * @return
 *   The name of the style
 */
function biblio_style_cse_info() {
  return array (
    'cse' => 'Council of Science Editors (CSE)'
  );
}
/**
 * Apply a bibliographic style to the node
 *
 *
 * @param $node
 *   An object containing the node data to render
 * @param $base
 *   The base URL of the biblio module (defaults to /biblio)
 * @param $inline
 *   A logical value indicating if this is being rendered within the
 *   Drupal framwork (false) or we are just passing back the html (true)
 * @return
 *   The styled biblio entry
 */
function biblio_style_cse_author_options() {
  $author_options = array(
    'BetweenAuthorsDelimStandard'     =>', ',      //4
    'BetweenAuthorsDelimLastAuthor'   => ', ', //5
    'AuthorsInitialsDelimFirstAuthor' => ', ',     //7
    'AuthorsInitialsDelimStandard'    => ' ',      //8
    'betweenInitialsDelim'            => '',     //9
    'initialsBeforeAuthorFirstAuthor' => false,    //10
    'initialsBeforeAuthorStandard'    => FALSE,     //11
    'shortenGivenNames'               => TRUE,    //12
    'numberOfAuthorsTriggeringEtAl'   => 10,       //13
    'includeNumberOfAuthors'          => 10,       //14
    'customStringAfterFirstAuthors'   => ' et al.',//15
    'encodeHTML'                      => true
  );
  return $author_options;
}

function biblio_style_cse($node, $base = 'biblio', $inline = false) {
  $author_options = biblio_style_cse_author_options();
  $authors = theme('biblio_format_authors', $node->biblio_contributors[1], $author_options, $inline);
  if (empty($authors) && count($node->biblio_contributors[5])) {// if no authors substitute corp author if available
    foreach ($node->biblio_contributors[5] as $rank => $author) {
      $authors .= (empty($authors)) ? '' : ', ';
      $authors .= (variable_get('biblio_author_links', 1)) ?  theme('biblio_author_link',$author['name'], $author['cid'], variable_get('biblio_base', 'biblio'), $inline) : $author['name'];
    }
  }
  if (empty($authors)) $authors = '[' . t('Anonymous') . ']';  // use anonymous if we still have nothing.
  if (!empty ($node->biblio_citekey)&&(variable_get('biblio_display_citation_key',0))) {
    $output .= '[' . check_plain($node->biblio_citekey) . '] ';
  }
  $output .= '<span class="biblio-authors">' . $authors . "</span>.&nbsp; \n";
  switch ($node->biblio_type) {

    default :
      if (isset ($node->biblio_year)) {
        $output .= check_plain($node->biblio_year) . ".&nbsp;&nbsp;";
      }
      $output .= '<span class="biblio-title">';
      $url = biblio_get_title_url_info($node);
      $output .= l($node->title, $url['link'], $url['options']);
      $output .= (strpos($node->title, '?'))? " </span>" : ". </span>";  // if the title ends in a question mark, don't put a period after it.
      $output .= (!empty ($node->biblio_secondary_title)) ? check_plain($node->biblio_secondary_title) . '. ' : '';
      $output .= (!empty ($node->biblio_volume))          ? check_plain($node->biblio_volume) : '';
      $output .= (!empty ($node->biblio_issue))           ? '(' . check_plain($node->biblio_issue) . ')' : '';
      $output .= (!empty ($node->biblio_pages))           ? ':' . str_replace(" ", "", check_plain($node->biblio_pages)) . '.' : '';
      break; // generic
  }
  /*  if ($node->biblio_date) $output .= ', '. check_plain($node->biblio_date);
    if ($node->biblio_number) $output .= ', Number '. check_plain($node->biblio_number);

    if ($node->biblio_place_published) $output .= ', '. check_plain($node->biblio_place_published);
  */
  return filter_xss($output, biblio_get_allowed_tags());

}
function _cse_format_author($author) {
  $format = $author['prefix'] . ' ' . $author['lastname'] . ' ';
  $format .= !empty ($author['firstname']) ? ' ' . drupal_substr($author['firstname'], 0, 1) : '';
  $format .= !empty ($author['initials']) ? str_replace(' ', '', $author['initials']) : '';
  return $format;
}