<?php
// Original File:       ./cite/styles/cite_MLA.php
// Original Repository: https://refbase.svn.sourceforge.net/svnroot/refbase/trunk/cite/styles/cite_MLA.php $
// Original Author(s):  Richard Karnesky <mailto:karnesky@gmail.com> and
//                      Matthias Steffens <mailto:refbase@extracts.de>
//
// Modified for use in biblio by Ron Jerome
//
// $Id: biblio_style_mla.inc,v 1.7.2.10 2009/09/30 15:48:25 rjerome Exp $
/**
 * Get the style information
 *
 * @return
 *   The name of the style
 */
function biblio_style_mla_info() {
  return array (
    'mla' => 'Modern Language Association (MLA)'
    );
}
function biblio_style_mla_author_options() {
  $author_options = array(
    'BetweenAuthorsDelimStandard' =>', ',         //4
    'BetweenAuthorsDelimLastAuthor' => ', and ',  //5
    'AuthorsInitialsDelimFirstAuthor' => ', ',     //7
    'AuthorsInitialsDelimStandard'=> ' ',         //8
    'betweenInitialsDelim' => '. ',               //9
    'initialsBeforeAuthorFirstAuthor' => false,   //10
    'initialsBeforeAuthorStandard' => true,       //11
    'shortenGivenNames' => FALSE,                 //12
    'numberOfAuthorsTriggeringEtAl' => 3,         //13
    'includeNumberOfAuthors' => 1,                //14
    'customStringAfterFirstAuthors' => ', et al.',//15
    'encodeHTML' => true
  );

  return $author_options;
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
function biblio_style_mla($node, $base = 'biblio', $inline = false) {

  $author_options = biblio_style_mla_author_options();
  $authors = theme('biblio_format_authors', $node->biblio_contributors[1], $author_options, $inline);
  //if (empty($authors)) $authors = theme('biblio_authors', $node->biblio_contributors[5], 'mla', 5, $inline);  // if no authors substitute corp author if available
  //if (empty($authors)) $authors = '[' . t('Anonymous') . ']';  // use anonymous if we still have nothing.
  //$output .= '<span class="biblio-authors">' . $authors . "</span>.&nbsp; \n";
  if (!empty ($node->biblio_citekey)&&(variable_get('biblio_display_citation_key',0))) {
    $output .= '[' . check_plain($node->biblio_citekey) . '] ';
  }

  switch ($node->biblio_type) {
    case 102: //Journal Article
    case 105: //Newspaper Article
    case 106: //Magazine Article
      if(!empty($authors)) {
        if (!ereg("\. *$", $authors))
        $output .= $authors . ".";
        else
        $output .= $authors;
      }

      if (!empty($node->title))      // title
      {
        if (!empty($authors)) $output .= " ";
        $output .= '"' ;
        $output .= '<span class="biblio-title">';
        $url = biblio_get_title_url_info($node);
        $output .= l($node->title, $url['link'], $url['options']);
        $output .= ". </span> \n";
        if (!ereg("[?!.]$", $node->title)) $output .= ".";
        $output .= '"';
      }

      // From here on we'll assume that at least either the 'author' or the 'title' field did contain some contents
      // if this is not the case, the output string will begin with a space. However, any preceding/trailing whitespace will be removed at the cleanup stage (see below)

      if (!empty($node->biblio_alternate_title)) {      // abbreviated journal name
        $output .= " " . '<i>' . $node->biblio_alternate_title . '</i>';

        // if there's no abbreviated journal name, we'll use the full journal name
      }
      elseif (!empty($node->biblio_secondary_title)) {      // publication (= journal) name
        $output .= " " . '<i>' . $node->biblio_secondary_title . '</i>';
      }
      if (!empty($node->biblio_volume)) {     // volume
        if (!empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)) {
          $output .= ".";
        }
        $output .= " " . $node->biblio_volume;
      }
      if (!empty($node->biblio_issue)) {     // issue
        $output .=  "." . $node->biblio_issue;
      }
      if (!empty($node->biblio_year))      // year
      {
        $output .= " (".$node->biblio_year . ")";
      }
      // FIXME do something with the online pubs section
      if ($node->online_publication == "yes") // this record refers to an online article
      {
        // instead of any pages info (which normally doesn't exist for online publications) we append
        // an optional string (given in 'online_citation') plus the current date and the DOI (or URL):

        $today = date("j M. Y");

        if (!empty($node->online_citation))      // online_citation
        {
          if (!empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)) // only add ":" if either volume, issue, abbrev_journal or publication isn't empty
          $output .= ":";

          $output .= " " . $node->online_citation;
        }

        if (!empty($node->doi))      // doi
        {
          if (!empty($node->online_citation) OR (empty($node->online_citation) AND (!empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)))) // only add "." if online_citation isn't empty, or else if either volume, issue, abbrev_journal or publication isn't empty
          $output .= ".";

          if ($encodeHTML)
          $output .= " " . $today . encodeHTML(" <http://dx.doi.org/" . $node->doi . ">");
          else
          $output .= " " . $today . " <http://dx.doi.org/" . $node->doi . ">";
        }
        elseif (!empty($node->url))      // url
        {
          if (!empty($node->online_citation) OR (empty($node->online_citation) AND (!empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)))) // only add "." if online_citation isn't empty, or else if either volume, issue, abbrev_journal or publication isn't empty
          $output .= ".";

          if ($encodeHTML)
          $output .= " " . $today . encodeHTML(" <" . $node->url . ">");
          else
          $output .= " " . $today . " <" . $node->url . ">";
        }

      }
      else // $node->online_publication == "no" -> this record refers to a printed article, so we append any pages info instead:
      {
        if (!empty($node->biblio_pages))      // pages
        {
          if (!empty($node->biblio_year) ||
          !empty($node->biblio_volume) ||
          !empty($node->biblio_issue) ||
          !empty($$node->biblio_alternate_title) ||
          !empty($node->biblio_secondary_title)) {  // only add ": " if either volume, issue, abbrev_journal or publication isn't empty
            $output .= ": ";
          }
          // TODO: FIXME $output .= formatPageInfo($node->biblio_pages, $markupPatternsArray["endash"]); // function 'formatPageInfo()' is defined in 'cite.inc.php'
          $output .= theme_biblio_page_number($node->biblio_pages);
        }
      }

      if (!ereg("\. *$", $output))
      $output .= ".";

      break;
    case 101: //Book Chapter
    case 103: //Conference Paper
      if(!empty($authors)) {
        if (!ereg("\. *$", $authors)) {
          $output .= $authors . ".";
        }
        else {
          $output .= $authors;
        }

        if (!empty($node->title))      // title
        {
          if (!empty($authors))  $output .= " ";

          $output .= '"<i>' ;
          $output .= '<span class="biblio-title">';
          $url = biblio_get_title_url_info($node);
          $output .= l($node->title, $url['link'], $url['options']);
          $output .= ". </span> \n";
          $output .= '</i>';
          if (!ereg("[?!.]$", $node->title)) $output .= ".";
          $output .= '"';
        }

        $publication = ereg_replace("[ \r\n]*\(Eds?:[^\)\r\n]*\)", "", $node->biblio_secondary_title);
        if (!empty($publication))      // publication
        $output .= " <i>$publication</i>";


        // From here on we'll assume that at least either the 'author' or the 'title' field did contain some contents
        // if this is not the case, the output string will begin with a space. However, any preceding/trailing whitespace will be removed at the cleanup stage (see below)

        if (!empty($node->biblio_contributors[2]))      // editor
        {
          $editor_options = array(
            'BetweenAuthorsDelimStandard' =>', ',         //4
            'BetweenAuthorsDelimLastAuthor' => ', and ',  //5
            'AuthorsInitialsDelimFirstAuthor' => ' ',     //7
            'AuthorsInitialsDelimStandard'=> ' ',         //8
            'betweenInitialsDelim' => '. ',               //9
            'initialsBeforeAuthorFirstAuthor' => true,   //10
            'initialsBeforeAuthorStandard' => true,       //11
            'shortenGivenNames' => false,                 //12
            'numberOfAuthorsTriggeringEtAl' => 3,         //13
            'includeNumberOfAuthors' => 1,                //14
            'customStringAfterFirstAuthors' => ', et al.',//15
            'encodeHTML' => true
          );
          $editor = theme('biblio_format_authors', $node->biblio_contributors[2], $editor_options, $inline);

          if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";

          if (count($node->biblio_contributors[2]) > 1) { // there are at least two editors (separated by ';')
            $output .= " Eds. " . $editor;
          }
          else { // there's only one editor (or the editor field is malformed with multiple editors but missing ';' separator[s])
            $output .= " Ed. " . $editor;
          }
        }

        if (!empty($node->biblio_edition) && !preg_match("/^(1|1st|first|one)( ed\.?| edition)?$/i", $node->biblio_edition))      // edition
        {
          if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";

          if (preg_match("/^\d{1,3}$/", $node->biblio_edition)) // if the edition field contains a number of up to three digits, we assume it's an edition number (such as "2nd ed.")
          {
            if ($node->biblio_edition == "2") {
              $editionSuffix = "nd";
            }
            elseif ($node->biblio_edition == "3") {
              $editionSuffix = "rd";
            }
            else {
              $editionSuffix = "th";
            }
          }
          else {
            $editionSuffix = "";
          }

          if (preg_match("/^(Rev\.?|Revised)( ed\.?| edition)?$/i", $node->biblio_edition)) {
            $node->biblio_edition = "Rev.";
          }
          elseif (preg_match("/^(Abr\.?|Abridged)( ed\.?| edition)?$/i", $node->biblio_edition)) {
            $node->biblio_edition = "Abr.";
          }
          if (!preg_match("/( ed\.?| edition)$/i", $node->biblio_edition)) {
            $editionSuffix .= " ed.";
          }
          $output .= " " . $node->biblio_edition . $editionSuffix;
        }

        if (!empty($node->biblio_volume))      // volume
        {
          if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";
          $output .= " Vol. " . $node->biblio_volume;
        }

        if (!empty($node->biblio_alternate_title) OR !empty($node->biblio_tertiary_title)) // if there's either a full or an abbreviated series title
        {
          if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";
          $output .= " ";

          if (!empty($node->biblio_alternate_title)) {
            $output .= $node->biblio_alternate_title;      // abbreviated series title
          }
          // if there's no abbreviated series title, we'll use the full series title instead:
          elseif (!empty($node->biblio_tertiary_title)) {
            $output .= $node->biblio_tertiary_title;      // full series title
          }
          if (!empty($node->biblio_volume)||!empty($node->biblio_issue)) {
            $output .= ", ";
          }
          if (!empty($node->biblio_volume)) {      // series volume
            $output .= $node->biblio_volume;
          }
          if (!empty($node->biblio_issue)) {      // series issue (I'm not really sure if -- for this cite style -- the series issue should be rather omitted here)
            $output .= "." . $node->biblio_issue; // is it correct to format series issues similar to journal article issues?
          }
        }

        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";

        if (!empty($node->biblio_place_published)) {     // place
          $output .= " " . $node->biblio_place_published;
        }
        if (!empty($node->biblio_publisher))      // publisher
        {
          if (!empty($node->biblio_place_published))  $output .= ":";
          $output .= " " . $node->biblio_publisher;
        }

        if (!empty($node->biblio_year))      // year
        {
          $output .= ", " . $node->biblio_year;
        }

        if (!empty($node->biblio_pages)) {     // pages
          $output .= theme_biblio_page_number($node->biblio_pages);
        }
        if (!ereg("\. *$", $output))  $output .= ".";
      }
      break;

    default : // all other types

      if (!empty($authors))      // author
      {
        if (!ereg("\. *$", $authors))
        $output .= $authors . ".";
        else
        $output .= $authors;
      }

      if (!empty($node->title))      // title
      {
        $url = biblio_get_title_url_info($node);
        if (!empty($authors))
        $output .= " ";

        if (!empty($node->thesis))      // thesis
        {
          $output .= '<span class="biblio-title">';
          $output .= '"' . l($node->title, $url['link'], $url['options']);
          $output .= '</span>';
          if (!ereg("[?!.]$", $node->title))
          $output .= ".";
          $output .= '"';
        }
        else // not a thesis
        $output .= '<i>';
        $output .= '<span class="biblio-title">';
        $output .= l($node->title, $url['link'], $url['options']);
        $output .= '</span>';
        $output .= '</i>';
      }

      if (count($node->biblio_contributors[2]))      // editor (if different from author, see note above regarding the check for ' (ed)' or ' (eds)')
      {

        $editor_options = array(
          'BetweenAuthorsDelimStandard' =>', ',         //4
          'BetweenAuthorsDelimLastAuthor' => ', and ',  //5
          'AuthorsInitialsDelimFirstAuthor' => ' ',     //7
          'AuthorsInitialsDelimStandard'=> ' ',         //8
          'betweenInitialsDelim' => '. ',               //9
          'initialsBeforeAuthorFirstAuthor' => true,   //10
          'initialsBeforeAuthorStandard' => true,       //11
          'shortenGivenNames' => false,                 //12
          'numberOfAuthorsTriggeringEtAl' => 3,         //13
          'includeNumberOfAuthors' => 1,                //14
          'customStringAfterFirstAuthors' => ', et al.',//15
          'encodeHTML' => true
        );
        $editor = theme('biblio_format_authors', $node->biblio_contributors[2], $editor_options, $inline);
        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";

        if (count($node->biblio_contributors[2] > 1)) {// there are at least two editors (separated by ';')
          $output .= " Eds. " . $editor;
        }
        else {// there's only one editor (or the editor field is malformed with multiple editors but missing ';' separator[s])
          $output .= " Ed. " . $editor;
        }
      }

      if (!empty($node->biblio_edition) && !preg_match("/^(1|1st|first|one)( ed\.?| edition)?$/i", $node->biblio_edition))      // edition
      {
        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";

        if (preg_match("/^\d{1,3}$/", $node->biblio_edition)) // if the edition field contains a number of up to three digits, we assume it's an edition number (such as "2nd ed.")
        {
          if ($node->biblio_edition == "2") {
            $editionSuffix = "nd";
          }
          elseif ($node->biblio_edition == "3") {
            $editionSuffix = "rd";
          }
          else {
            $editionSuffix = "th";
          }
        }
        else {
          $editionSuffix = "";
        }

        if (preg_match("/^(Rev\.?|Revised)( ed\.?| edition)?$/i", $node->biblio_edition)) {
          $node->biblio_edition = "Rev.";
        }
        elseif (preg_match("/^(Abr\.?|Abridged)( ed\.?| edition)?$/i", $node->biblio_edition)) {
          $node->biblio_edition = "Abr.";
        }
        if (!preg_match("/( ed\.?| edition)$/i", $node->biblio_edition)) {
          $editionSuffix .= " ed.";
        }
        $output .= " " . $node->biblio_edition . $editionSuffix;
      }

      if (!empty($node->biblio_volume))      // volume
      {
        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output)) $output .= ".";
        $output .= " Vol. " . $node->biblio_volume;
      }

      if (!empty($node->biblio_alternate_title) OR !empty($node->biblio_secondary_title)) // if there's either a full or an abbreviated series title
      {
        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output))  $output .= ".";
        $output .= " ";

        if (!empty($node->biblio_alternate_title)) {
          $output .= $node->biblio_alternate_title;      // abbreviated series title
        }
        // if there's no abbreviated series title, we'll use the full series title instead:
        elseif (!empty($node->biblio_secondary_title)) {
          $output .= $node->biblio_secondary_title;      // full series title
        }
        if (!empty($node->biblio_volume)||!empty($node->biblio_issue)) {
          $output .= ", ";
        }
        if (!empty($node->biblio_volume)) {     // series volume
          $output .= $node->biblio_volume;
        }
        if (!empty($node->biblio_issue)) {     // series issue (I'm not really sure if -- for this cite style -- the series issue should be rather omitted here)
          $output .= "." . $node->biblio_issue; // is it correct to format series issues similar to journal article issues?
        }
      }

      if (!empty($node->thesis))      // thesis (unpublished dissertation)
      {
        // TODO: a published dissertation needs to be formatted differently!
        //       see e.g. example at: <http://web.csustan.edu/english/reuben/pal/append/AXI.HTML>

        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output))
        $output .= ".";

        // TODO: I've also seen MLA examples that separate thesis name, name of institution and year by dots. ?:-|
        //       Also, do we need to use the abbreviation "Diss." instead of "Ph.D. thesis"? What about other thesis types then?
        //       see e.g. <http://www.english.uiuc.edu/cws/wworkshop/writer_resources/citation_styles/mla/unpublished_diss.htm>
        $output .= " " . $node->thesis;
        $output .= ", " . $node->biblio_publisher;
      }
      else // not a thesis
      {
        if (!ereg("[?!.][ \"" . '</i>' . "]*$", $output))  $output .= ".";

        if (!empty($node->biblio_place_published)) {     // place
          $output .= " " . $node->biblio_place_published;
        }
        if (!empty($node->biblio_publisher))      // publisher
        {
          if (!empty($node->biblio_place_published))  $output .= ":";
          $output .= " " . $node->biblio_publisher;
        }
      }

      if (!empty($node->biblio_year)) {     // year
        $output .= ", ".$node->biblio_year;
      }

      if ($node->online_publication == "yes") // this record refers to an online article
      {
        $today = date("j M. Y");

        if (!empty($node->online_citation))      // online_citation
        {
          if (!ereg("\. *$", $output)) $output .= ".";

          $output .= " " . $node->online_citation;
        }

        if (!empty($node->doi))      // doi
        {
          if (!ereg("\. *$", $output)) $output .= ".";

          if ($encodeHTML) {
            $output .= " " . $today . encodeHTML(" <http://dx.doi.org/" . $node->doi . ">");
          }
          else {
            $output .= " " . $today . " <http://dx.doi.org/" . $node->doi . ">";
          }
        }
        elseif (!empty($node->url))      // url
        {
          if (!ereg("\. *$", $output)) $output .= ".";

          if ($encodeHTML) {
            $output .= " " . $today . encodeHTML(" <" . $node->url . ">");
          }
          else {
            $output .= " " . $today . " <" . $node->url . ">";
          }
        }
      }

      if (!ereg("\. *$", $output)) $output .= ".";
      break;
  }

  return filter_xss($output, biblio_get_allowed_tags());
}
