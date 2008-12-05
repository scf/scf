<?php
//$Id: biblio.contributors.inc,v 1.1.2.18 2008/11/24 03:19:00 rjerome Exp $
/**
 * @param $authors
 * @param $withLinks
 * @return unknown_type
 */
function biblio_get_contributor_strings($authors, $withLinks = TRUE) {
  if (count($authors)) {
    foreach ($authors as $key => $author) {
      $author_array[$author['ct_id']] .= (!empty ($author_array[$author['ct_id']])) ? '; ' . $author['name'] : $author['name']; // $author['firstname'] .' '. $author['initials'].' '.$author['lastname'];
    }
  }
  if (count($author_array)) {
    //ksort($author_array);
    return $author_array;
  }
  else {
    return array(
    1 => t('No author information available'
    ));
  }
}
/**
 * @param $aid
 * @return unknown_type
 */
function biblio_get_contributor($aid) {
  static $contributor = array();
  if (!isset($contributor[$aid])) {
    $contributor[$aid] = db_fetch_object(db_query('SELECT * FROM {biblio_contributor_data} WHERE cid = %d', $aid));
  }

  return $contributor[$aid];
}

/**
 * @param $vid
 * @return unknown_type
 */
function biblio_load_contributors($vid) {
  $authors = array();
  $query = 'SELECT bcd.*, bct.type, bct.ct_id, bct.ctdid, bc.rank
    				  FROM    {biblio_contributor} bc,
    					 {biblio_contributor_data} bcd,
    					 {biblio_contributor_type} bct
    				  WHERE bc.vid = %d
    				  	 AND bc.cid = bcd.cid
    				  	 AND  bc.ct_id = bct.ct_id
    				  ORDER BY bc.ct_id ASC, bc.rank ASC;';
  $result = db_query($query, array($vid));
  while ($creator = db_fetch_array($result)) {
    $contributors[] = $creator;
  }
  return $contributors;
}
/**
 * @param $node
 * @param $update
 * @return unknown_type
 */
function biblio_save_contributors($node, $update = FALSE) {
  if (!empty ($node->biblio_contributors))
  biblio_parse_save($node->biblio_contributors, $node->nid, $node->vid, $update);
}
/**
 * @param $authors
 * @param $nid
 * @param $vid
 * @param $update
 * @return unknown_type
 */
function _save_contributors($authors, $nid, $vid, $update = FALSE) {
  $rank = array();
  $md5 = _loadMD5();
  db_query('DELETE FROM {biblio_contributor} WHERE nid = %d AND vid = %d', array($nid, $vid));
  foreach ($authors as $key => $author) {
    if ($update && !empty($author['cid']))  $author['cid'] = null; // null out the cid so we don't do a global change
    if (empty ($author['cid']) && !empty ($md5))
    $author['cid'] = array_search($author['md5'], $md5);
    if (empty ($author['cid'])) {
      drupal_write_record('biblio_contributor_data', $author);
      $cid = db_last_insert_id('biblio_contributor_data', 'cid');
    } else {
      $cid = $author['cid'];
    }
    $author = (array) $author;
    $link_array = array(
      'nid' => $nid,
      'vid' => $vid,
      'cid' => $cid,
      'rank' => $rank[$author['type']]++,
      'ct_id' => $author['ct_id']
    );
    drupal_write_record('biblio_contributor', $link_array);
  }
}
/**
 * @param $authors_array
 * @param $nid
 * @param $vid
 * @param $update
 * @return unknown_type
 */
function biblio_parse_save($authors_array, $nid, $vid, $update = FALSE) {
  $authors = array();
  foreach ($authors_array as $author) {
    if (strlen(trim($author['name']))) {
      //$author['type'] = $ctid;
      biblio_parse_author($author);
      $authors[] = (array) $author;
    }
  }
  if (!empty ($authors))
  _save_contributors($authors, $nid, $vid, $update);
  return;
}
/*
 Released through http://bibliophile.sourceforge.net under the GPL licence.
 Do whatever you like with this -- some credit to the author(s) would be appreciated.

 A collection of PHP classes to manipulate bibtex files.

 If you make improvements, please consider contacting the administrators at bibliophile.sourceforge.net so that your improvements can be added to the release package.

 Mark Grimshaw 2004/2005
 http://bibliophile.sourceforge.net

 28/04/2005 - Mark Grimshaw.
 Efficiency improvements.

 11/02/2006 - Daniel Reidsma.
 Changes to preg_matching to account for Latex characters in names such as {\"{o}}
 */
// For a quick command-line test (php -f PARSECREATORS.php) after installation, uncomment these lines:
/***********************
 $authors = "Mark \~N. Grimshaw and Bush III, G.W. & M. C. H{\\'a}mmer Jr. and von Frankenstein, Ferdinand Cecil, P.H. & Charles Louis Xavier Joseph de la Vallee P{\\\"{o}}ussin";
 $creator = new PARSECREATORS();
 $creatorArray = $creator->parse($authors);
 print_r($creatorArray);
 ***********************/
/* Create writer arrays from bibtex input.
 'author field can be (delimiters between authors are 'and' or '&'):
 1. <first-tokens> <von-tokens> <last-tokens>
 2. <von-tokens> <last-tokens>, <first-tokens>
 3. <von-tokens> <last-tokens>, <jr-tokens>, <first-tokens>
 */
/**
 * @param $author_array
 * @return unknown_type
 */
function biblio_parse_author(& $author_array) {
  $value = trim($author_array['name']);
  $appellation = $prefix = $surname = $firstname = $initials = '';
  $prefix = "";
  //		$prefix2= array();
  $author = explode(",", preg_replace("/\s{2,}/", ' ', trim($value)));
  $size = sizeof($author);
  // No commas therefore something like Mark Grimshaw, Mark Nicholas Grimshaw, M N Grimshaw, Mark N. Grimshaw
  if ($size == 1) {
    // Is complete surname enclosed in {...}, unless the string starts with a backslash (\) because then it is
    // probably a special latex-sign..
    // 2006.02.11 DR: in the last case, any NESTED curly braces should also be taken into account! so second
    // clause rules out things such as author="a{\"{o}}"
    //
    if (preg_match("/(.*){([^\\\].*)}/", $value, $matches) && !(preg_match("/(.*){\\\.{.*}.*}/", $value, $matches2))) {
      $author = split(" ", $matches[1]);
      $surname = $matches[2];
    }
    else {
      $author = split(" ", $value);
      // last of array is surname (no prefix if entered correctly)
      $surname = array_pop($author);
    }
  }
  // Something like Grimshaw, Mark or Grimshaw, Mark Nicholas  or Grimshaw, M N or Grimshaw, Mark N.
  else
  if ($size == 2) {
    // first of array is surname (perhaps with prefix)
    list ($surname, $prefix) = _grabSurname(array_shift($author));
  }
  // If $size is 3, we're looking at something like Bush, Jr. III, George W
  else {
    // middle of array is 'Jr.', 'IV' etc.
    $appellation = join(' ', array_splice($author, 1, 1));
    // first of array is surname (perhaps with prefix)
    list ($surname, $prefix) = _grabSurname(array_shift($author));
  }
  $remainder = join(" ", $author);
  list ($firstname, $initials, $prefix2) = _grabFirstnameInitials($remainder);
  if (!empty ($prefix2))
  $prefix .= $prefix2;
  //var_dump($prefix);
  $surname = $surname . ' ' . $appellation;
  $author_array['firstname'] = trim($firstname);
  $author_array['initials'] = trim($initials);
  $author_array['lastname'] = trim($surname);
  $author_array['prefix'] = trim($prefix);
  $author_array['md5'] =  _md5sum($author_array);
  return;
}
/**
 * @param $creator
 * @return unknown_type
 */
function _md5sum($creator) {
  $string = $creator['firstname'] . $creator['initials'] . $creator['prefix'] .$creator['lastname'];
  $string = str_replace(' ', '', drupal_strtolower($string));
  return md5($string);
}
// grab firstname and initials which may be of form "A.B.C." or "A. B. C. " or " A B C " etc.
/**
 * @param $remainder
 * @return unknown_type
 */
function _grabFirstnameInitials($remainder) {
  $prefix = array();
  $firstname = $initials = '';
  $array = split(" ", $remainder);
  foreach ($array as $value) {
    $firstChar = drupal_substr($value, 0, 1);
    if ((ord($firstChar) >= 97) && (ord($firstChar) <= 122)){
    $prefix[] = $value;
    } else if (preg_match("/[a-zA-Z]{2,}/", trim($value))){
      $firstnameArray[] = trim($value);
    } else {
      $initialsArray[] = str_replace(".", " ", trim($value));
    }
  }
  if (isset ($initialsArray)) {
    foreach ($initialsArray as $initial)
    $initials .= ' ' . trim($initial);
  }
  if (isset ($firstnameArray)) {
    $firstname = join(" ", $firstnameArray);
  }
  if (!empty ($prefix)){
    $prefix = join(" ", $prefix);
  }
  return array($firstname,$initials,$prefix);
}
// surname may have title such as 'den', 'von', 'de la' etc. - characterised by first character lowercased.  Any
// uppercased part means lowercased parts following are part of the surname (e.g. Van den Bussche)
/**
 * @param $input
 * @return unknown_type
 */
function _grabSurname($input) {
  $surnameArray = split(" ", $input);
  $noPrefix = $surname = FALSE;
  foreach ($surnameArray as $value) {
    $firstChar = substr($value, 0, 1);
    if (!$noPrefix && (ord($firstChar) >= 97) && (ord($firstChar) <= 122))
    $prefix[] = $value;
    else {
      $surname[] = $value;
      $noPrefix = TRUE;
    }
  }
  if ($surname)
  $surname = join(" ", $surname);
  if (isset ($prefix)) {
    $prefix = join(" ", $prefix);
    return array(
    $surname,
    $prefix
    );
  }
  return array(
  $surname,
  FALSE
  );
}
/**
 * @param $type
 * @return unknown_type
 */
function _typeMap($type) {
  static $typeMap = array();
  if (empty ($typeMap)) {
    $result = db_query("SELECT * FROM {biblio_contributor_type} ;");
    while ($type = db_fetch_object($result)) {
      $typeMap[$type->type] = $type->ct_id;
    }
  }
  return $typeMap[$type];
}
/**
 * @return unknown_type
 */
function _loadMD5() {
  $md5 = array();
  $result = db_query('SELECT md5,cid	FROM {biblio_contributor_data} ');
  while ($row = db_fetch_array($result)) {
    $md5[$row['cid']] = $row['md5'];
  }
  return (count($md5)) ? $md5 : NULL;
}