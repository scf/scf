<?php
// $Id: biblio_style_ieee.inc,v 1.8.2.1 2008/05/20 02:01:00 rjerome Exp $
// by Jeffrey Dwoskin
// A style closer to IEEE format
// see http://standards.ieee.org/guides/style/section7.html#992
/**
 * Get the style information
 *
 * @return
 *   The name of the style
 */
function biblio_style_ieee_info() {
  return array(
    'ieee' => 'Institute of Electrical and Electronics Engineers (IEEE)'
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
function biblio_style_ieee($node, $base = 'biblio', $inline = false) {
  if (variable_get('biblio_normalize', 0)) {
    $authors = _biblio_parse_authors($node->biblio_authors);
  }
  else {
    $authors = $node->biblio_authors;
  }
  $contrib_array = _ieee_format_authors($node->biblio_contributors);
  $authors = $contrib_array[1];
  $output .= '<span class="biblio-authors">'. $authors .", </span> \n";
  switch ($node->biblio_type) {
    default :
      if ($node->biblio_secondary_title) {
        $output .= '<span class="biblio-title">&quot;';
        $output .= $inline ? l("$node->title", "$base/viewinline/$node->nid") : l("$node->title", "node/$node->nid");
        $output .= "&quot;, </span> \n";
        $output .= '<i>'. check_plain($node->biblio_secondary_title) .'</i>';
      }
      else {
        $output .= '<span class="biblio-title"><i>';
        $output .= $inline ? l("$node->title", "$base/viewinline/$node->nid") : l("$node->title", "node/$node->nid");
        $output .= ", </i></span> \n";
      }
      if ($node->biblio_edition)
        $output .= ', '. check_plain($node->biblio_edition);
      if ($node->biblio_volume)
        $output .= ', vol. '. check_plain($node->biblio_volume);
      if ($node->biblio_issue)
        $output .= ', issue '. check_plain($node->biblio_issue);
      if ($node->biblio_number)
        $output .= ', no. '. check_plain($node->biblio_number);
      if ($node->biblio_place_published)
        $output .= ', '. check_plain($node->biblio_place_published);
      if ($node->biblio_publisher) {
        $output .= (check_plain($node->biblio_place_published)) ? ', ' : ': ';
        $output .= check_plain($node->biblio_publisher);
      }
      // if a single page instead of a range, should use 'p.' instead of 'pp.'  -- ignoring
      if ($node->biblio_pages)
        $output .= ', pp. '. check_plain($node->biblio_pages);
      // if it is a book, year should go before pages instead -- ignoring
      // for non-books, should also include month of publication (e.g. "Mar. 2006") -- use date instead of year if available
      if ($node->biblio_date) {
        $output .= ', '. check_plain($node->biblio_date);
      }
      elseif (isset ($node->biblio_year)) {
        $output .= ', '. check_plain($node->biblio_year) .'&nbsp;&nbsp;';
      }
      $output .= ".\n";
      break; // generic
  }
  /*  if ($node->biblio_date) $output .= ', '. check_plain($node->biblio_date);

  */
  return $output;
}
function _ieee_format_authors($contributors) {
  if (count($contributors)) {
    foreach ($contributors as $key => $author) {
      $format = $author['prefix'];
      $format .= !empty ($format) ? ' '. $author['lastname'] .' ' : $author['lastname'] .' ';
      $format .= !empty ($author['firstname']) ? drupal_substr($author['firstname'], 0, 1) .'.' : '';
      $format .= !empty ($author['initials']) ? $author['initials'] .'.' : '';
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
      1 => t('No author information available')
     );
  }
}