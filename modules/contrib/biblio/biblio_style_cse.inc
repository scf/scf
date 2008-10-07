<?php
// $Id: biblio_style_cse.inc,v 1.9.2.4 2008/05/14 02:08:00 rjerome Exp $
/**
 * Get the style information
 *
 * @return
 *   The name of the style
 */
function biblio_style_cse_info() {
  return array(
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
  $contrib_array = _cse_format_authors($node->biblio_contributors);
  $authors = $contrib_array[1];
  //  }
  $output .= '<span class="biblio-authors">'. $authors ."</span>.&nbsp; \n";
  //  $output .= '<span class="biblio-authors">'. str_replace("; ", ", ", str_replace(array(".",","),"",_biblio_author_links($authors,$base,$inline))) ."</span>.&nbsp; \n";
  //  $output .= (strrpos($authors, '.' ) == strlen($authors))? ".&nbsp;&nbsp;":" ";
  switch ($node->biblio_type) {
    case 1 : // Journal Article
    case 2 : //Conference Paper
    case 3 : // are all
    case 4 :
    case 5 :
    case 6 :
    case 7 :
    case 8 :
    case 9 :
    default :
      if (isset ($node->biblio_year)) {
        $output .= check_plain($node->biblio_year) .".&nbsp;&nbsp;";
      }
      $output .= '<span class="biblio-title">';
      $output .= $inline ? l("$node->title", "$base/viewinline/$node->nid") : l("$node->title", "node/$node->nid");
      $output .= ". </span> \n";
      $output .= ($node->biblio_secondary_title) ? check_plain($node->biblio_secondary_title) .'. ' : '';
      $output .= ($node->biblio_volume) ? check_plain($node->biblio_volume) : '';
      $output .= ($node->biblio_issue) ? '('. check_plain($node->biblio_issue) .')' : '';
      $output .= ($node->biblio_pages) ? ':'. str_replace(" ", "", check_plain($node->biblio_pages)) .'.' : '';
      break; // generic
  }
  /*  if ($node->biblio_date) $output .= ', '. check_plain($node->biblio_date);
    if ($node->biblio_number) $output .= ', Number '. check_plain($node->biblio_number);

    if ($node->biblio_place_published) $output .= ', '. check_plain($node->biblio_place_published);
  */
  return $output;
}
function _cse_format_authors($contributors) {
  if (count($contributors)) {
    foreach ($contributors as $key => $author) {
      $format = $author['prefix'] .' '. $author['lastname'] .' ';
      $format .= !empty ($author['firstname']) ? ' '. drupal_substr($author['firstname'], 0, 1) : '';
      $format .= !empty ($author['initials']) ? str_replace(' ', '', $author['initials']) : '';
      if (variable_get('biblio_author_links', 1)) {
        $base = variable_get('biblio_base', 'biblio');
        $format = _biblio_author_link($format, $author['cid'], $base, $inline);
      }
      $author_array[$author['ctid']] .= (!empty ($author_array[$author['ctid']])) ? ', '. $format : $format;
    }
  }
  if (count($author_array)) {
    return $author_array;
  }
  else {
    return array(
      1 => t('No author information available'
    ));
  }
}

