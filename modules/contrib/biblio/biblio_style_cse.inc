<?php

// $Id: biblio_style_cse.inc,v 1.9.2.10 2008/11/21 20:50:18 rjerome Exp $
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
function biblio_style_cse($node, $base = 'biblio', $inline = false) {
  $contrib_array = _cse_format_authors($node->biblio_contributors, $inline);
  $authors = $contrib_array[1];
  $output .= '<span class="biblio-authors">' . $authors . "</span>.&nbsp; \n";
  switch ($node->biblio_type) {

    default :
      if (isset ($node->biblio_year)) {
        $output .= check_plain($node->biblio_year) . ".&nbsp;&nbsp;";
      }
      $output .= '<span class="biblio-title">';
      $output .= $inline ? l("$node->title", "$base/viewinline/$node->nid") : l("$node->title", "node/$node->nid");
      $output .= ". </span> \n";
      $output .= (!empty ($node->biblio_secondary_title)) ? check_plain($node->biblio_secondary_title) . '. ' : '';
      $output .= (!empty ($node->biblio_volume)) ? check_plain($node->biblio_volume) : '';
      $output .= (!empty ($node->biblio_issue)) ? '(' . check_plain($node->biblio_issue) . ')' : '';
      $output .= (!empty ($node->biblio_pages)) ? ':' . str_replace(" ", "", check_plain($node->biblio_pages)) . '.' : '';
      break; // generic
  }
  /*  if ($node->biblio_date) $output .= ', '. check_plain($node->biblio_date);
    if ($node->biblio_number) $output .= ', Number '. check_plain($node->biblio_number);
  
    if ($node->biblio_place_published) $output .= ', '. check_plain($node->biblio_place_published);
  */
  return $output;
}
function _cse_format_authors($contributors, $inline = FALSE) {
  if (count($contributors)) {
    foreach ($contributors as $key => $author) {
      $format = $author['prefix'] . ' ' . $author['lastname'] . ' ';
      $format .= !empty ($author['firstname']) ? ' ' . drupal_substr($author['firstname'], 0, 1) : '';
      $format .= !empty ($author['initials']) ? str_replace(' ', '', $author['initials']) : '';
      if (variable_get('biblio_author_links', 1)) {
        $base = variable_get('biblio_base', 'biblio');
        $format = _biblio_author_link($format, $author['cid'], $base, $inline);
      }
      $author_array[$author['type']] .= (!empty ($author_array[$author['type']])) ? ', ' . $format : $format;
    }
  }
  if (count($author_array)) {
    return $author_array;
  } else {
    return array (
      1 => t('No author information available')
    );
  }
}