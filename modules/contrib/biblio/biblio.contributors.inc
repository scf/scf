<?php
//$Id: biblio.contributors.inc,v 1.1.2.29 2009/09/08 00:30:31 rjerome Exp $
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
  $contributors = array();
  $query = "SELECT * FROM {biblio_contributor} bc
			INNER JOIN {biblio_contributor_data} bcd ON bc.cid=bcd.cid
			WHERE bc.vid=%d
			ORDER BY bc.rank ASC"; // do not change order of presentation

  $result = db_query($query, $vid);
  while ($creator = db_fetch_array($result)) {
    $contributors[$creator['auth_category']][] = $creator;
  }
  return $contributors;
}
/**
 * Add separate author named "et al" to the end of the author array
 * 
 * @param $authors - author array to augment
 * @param $type - auth_type
 * @return true if author was added, false if "etal" was already there
 */
function biblio_authors_add_etal (&$authors, $type) {
  $etal = "et al"; $max_rank = 0;
  foreach ($authors as $author) { // et al author should be added only once per type
    if ($author['auth_type'] != $type) continue;
    if ($author['name'] == $etal) return false;
    $max_rank = max($max_rank, $author['rank']);
  }
  $authors[] = biblio_parse_author(array('name' => $etal, 'auth_type' => $type, 'lastname' => $etal, 'rank' => $max_rank + 1));
  return true;
}
/**
 * Parse initial contributor array and augment with additional info
 * @param $contributors initial contributor array
 * @return augmented contributor array
 */
function biblio_parse_contributors($contributors) {
  $result = array();
  foreach ($contributors as $cat => $authors) {
    $etal = array();   
    foreach ($authors as $author) {
      // remove any form of "et al" from name field, because it confuses biblio_parse_author
      $author_cleaned = preg_replace("/et\.?\s+al\.?/",'',$author['name']);
      if ($author_cleaned != $author['name']) { // if "et al" was present:
        $author['name'] = $author_cleaned;  // store cleaned name 
        $etal[$author['auth_type']] = true; // mark it as "to be added" in $etal array 
      }
      $author['name'] = trim($author['name']);
      if (strlen($author['name'])) {
        $result[$cat][] = biblio_parse_author($author, $cat);
      }
    }
    // add "et al" authors for all neccessary author types
    foreach ($etal as $type => $dummy) {
      if (isset($result[$cat])) { // add "et al" only if plain authors exists
        biblio_authors_add_etal ($result[$cat], $type);
      }
    }
  }
  return $result;
}
/**
 * Save $node->biblio_contributors to database
 * @param $node
 * @param $update
 * @return success of database operations
 */
function biblio_save_contributors($node, $update = FALSE) {
  if (empty ($node->biblio_contributors)) return true;
  return _save_contributors($node->biblio_contributors, $node->nid, $node->vid, $update);
}
/**
 * Save contributors to the database
 * @param $authors
 * @param $nid
 * @param $vid
 * @param $update
 * @return success of database operations
 */
function _save_contributors($contributors, $nid, $vid, $update = FALSE) {
  $md5 = _loadMD5();
  db_query('DELETE FROM {biblio_contributor} WHERE nid = %d AND vid = %d', array($nid, $vid));
  foreach ($contributors as $cat => $authors) { 
    foreach ($authors as $key => $author) {
      if ($update && !empty($author['cid']))  $author['cid'] = null; // null out the cid so we don't do a global change
      if (empty ($author['cid']) && !empty ($md5))
        $author['cid'] = array_search($author['md5'], $md5);
      if (empty ($author['cid'])) {
        if (!drupal_write_record('biblio_contributor_data', $author)) return false;
        $cid = db_last_insert_id('biblio_contributor_data', 'cid');
      } else {
        $cid = $author['cid'];
      }
      $link_array = array(
        'nid' => $nid,
        'vid' => $vid,
        'cid' => $cid,
        'rank' => (isset($author['rank']) && is_numeric($author['rank'])) ? $author['rank'] : $key,
        'auth_type' => $author['auth_type'],
        'auth_category' => $cat,
      );
  
      if (!drupal_write_record('biblio_contributor', $link_array)) return false;
    }
  }
  db_query("UPDATE {biblio_contributor_data} SET aka = cid WHERE aka = 0 OR aka IS NULL");
  return true; // successfully saved all contributors
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
function biblio_parse_author($author_array, $cat = 0) {
  if ($cat == 5){
    $author_array['firstname'] = '';
    $author_array['initials'] = '';
    $author_array['lastname'] = trim($author_array['name']);
    $author_array['prefix'] = '';
  }
  else {
    $value = trim($author_array['name']);
    $appellation = $prefix = $surname = $firstname = $initials = '';
    $prefix = "";
    $value = preg_replace("/\s{2,}/", ' ', $value); // replace multiple white space by single space
    $author = explode(",", $value);
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
  }
  $author_array['md5'] =  _md5sum($author_array);
  return $author_array;
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
      $initialsArray[] = trim(str_replace(".", " ", trim($value)));
    }
  }
  if (isset ($initialsArray)) {
    $initials = join(" ", $initialsArray);
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
 * @return unknown_type
 */
function _loadMD5() {
  static $md5 = array();
  static $count =0;
  $db_count = db_result(db_query("SELECT COUNT(*) FROM {biblio_contributor_data"));
  if ($db_count > $count){
    $count = $db_count;
    $result = db_query('SELECT md5,cid	FROM {biblio_contributor_data} ');
    while ($row = db_fetch_array($result)) {
      $md5[$row['cid']] = $row['md5'];
    }
  }
  return (count($md5)) ? $md5 : NULL;
}