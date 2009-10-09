<?php
// Original File:       ./cite/styles/cite_Chicago.php
// Original Author(s):  Matthias Steffens <mailto:refbase@extracts.de> and
//                      Richard Karnesky <mailto:karnesky@gmail.com>
//

// This is a citation style file (which must reside within the 'cite/styles/' sub-directory of your refbase root directory). It contains a
// version of the 'citeRecord()' function that outputs a reference list from selected records according to the citation style documented
// in the "Chicago Manual of Style" (2003), and Kate Turabian's "Manual for Writer's of Term Papers, Theses, and Dissertations" (1996)

// Modified for use in biblio by Ron Jerome
//
// $Id: biblio_style_chicago.inc,v 1.4.2.9 2009/09/30 15:48:25 rjerome Exp $
/**
 * Get the style information
 *
 * @return
 *   The name of the style
 */
function biblio_style_chicago_info() {
  return array (
    'chicago' => 'Chicago'
    );
}
function biblio_style_chicago_author_options() {
  $author_options = array(
    'BetweenAuthorsDelimStandard' =>', ',         //4
    'BetweenAuthorsDelimLastAuthor' => ', and ',  //5
    'AuthorsInitialsDelimFirstAuthor' => ', ',     //7
    'AuthorsInitialsDelimStandard'=> ' ',         //8
    'betweenInitialsDelim' => '. ',               //9
    'initialsBeforeAuthorFirstAuthor' => false,   //10
    'initialsBeforeAuthorStandard' => true,       //11
    'shortenGivenNames' => FALSE,                 //12
    'numberOfAuthorsTriggeringEtAl' => 10,         //13
    'includeNumberOfAuthors' => 10,                //14
    'customStringAfterFirstAuthors' => ' et al.',//15
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
function biblio_style_chicago($node, $base = 'biblio', $inline = false) {

  $author_options = biblio_style_chicago_author_options();
  // $authors = theme('biblio_authors', $node->biblio_contributors[1], 'mla', 1, $inline);

  $authors = theme('biblio_format_authors', $node->biblio_contributors[1], $author_options, $inline);
  //$editors = theme('biblio_format_authors', $node->biblio_contributors[2], $author_options, $inline);
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

      if (!empty($node->biblio_secondary_title))      // publication (= journal) name
      $output .= " <i>$node->biblio_secondary_title</i>";

      // if there's no full journal name, we'll use the abbreviated journal name
      elseif (!empty($node->biblio_alternate_title))      // abbreviated journal name
      $output .= " <i>$node->biblio_alternate_title</i>";

      if (!empty($node->biblio_volume))      // volume
      $output .= " " . $node->biblio_volume;

      if (!empty($node->biblio_issue))      // issue
      $output .=  ", no. " . $node->biblio_issue;

      if (!empty($node->biblio_year))      // year
      $output .= " (" . $node->biblio_year . ")";

      if (!empty($node->biblio_pages))      // pages
      {
        if (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)) // only add ": " if either year, volume, issue, abbrev_journal or publication isn't empty
        $output .= ": ";

        $output .= theme_biblio_page_number($node->biblio_pages); // function 'formatPageInfo()' is defined in 'cite.inc.php'
      }

      if ($node->online_publication == "yes") // this record refers to an online article
      {
        // append an optional string (given in 'online_citation') plus the current date and the DOI (or URL):

        $today = date("F j, Y");

        if (!empty($node->online_citation))      // online_citation
        {
          if (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)) // only add "," if either year, volume, issue, abbrev_journal or publication isn't empty
          {
            if (empty($node->biblio_pages))
            $output .= ":"; // print instead of pages
            else
            $output .= ","; // append to pages
          }

          $output .= " " . $node->online_citation;
        }

        if (!empty($node->doi))      // doi
        {
          if (!empty($node->online_citation) OR (empty($node->online_citation) AND (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)))) // only add "." if online_citation isn't empty, or else if either year, volume, issue, abbrev_journal or publication isn't empty
          $output .= "."; // NOTE: some Chicago examples (e.g. <http://www.lib.berkeley.edu/instruct/guides/chicago-turabianstyle.pdf>) use a comma here (not sure what's correct)

          if ($encodeHTML)
          $output .= " " . encodeHTML("http://dx.doi.org/" . $node->doi) . " (accessed " . $today . ")";
          else
          $output .= " " . "http://dx.doi.org/" . $node->doi . " (accessed " . $today . ")";
        }
        elseif (!empty($node->biblio_url))      // url
        {
          if (!empty($node->online_citation) OR (empty($node->online_citation) AND (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)))) // only add "." if online_citation isn't empty, or else if either year, volume, issue, abbrev_journal or publication isn't empty
          $output .= "."; // see note for doi

          if ($encodeHTML)
          $output .= " " . encodeHTML($node->biblio_url) . " (accessed " . $today . ")";
          else
          $output .= " " . $node->biblio_url . " (accessed " . $today . ")";
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
      $output .= " In <i>$publication</i>";


      // From here on we'll assume that at least either the 'author' or the 'title' field did contain some contents
      // if this is not the case, the output string will begin with a space. However, any preceding/trailing whitespace will be removed at the cleanup stage (see below)

      if (!empty($node->biblio_contributors[2]))      // editor
      {
        $editor_options = array(
          'BetweenAuthorsDelimStandard'     => ', ',
          'BetweenAuthorsDelimLastAuthor'   => ' and ',
          'AuthorsInitialsDelimFirstAuthor' => ' ',
          'AuthorsInitialsDelimStandard'    => ' ',
          'betweenInitialsDelim'            => '. ',
          'initialsBeforeAuthorFirstAuthor' => TRUE,
          'initialsBeforeAuthorStandard'    => TRUE,
          'shortenGivenNames'               => FALSE,
          'numberOfAuthorsTriggeringEtAl'   => 10,
          'includeNumberOfAuthors'          => 10,
          'customStringAfterFirstAuthors'   => ' et al.',
          'encodeHTML'                      => true
        );

        $editor = theme('biblio_format_authors', $node->biblio_contributors[2], $editor_options, $inline);

        $output .= ", edited by " . $editor;
      }

      if (!empty($node->biblio_pages))      // pages
      $output .= ", " . theme_biblio_page_number($node->biblio_pages); // function 'formatPageInfo()' is defined in 'cite.inc.php'

      if (!empty($node->biblio_edition) && !preg_match("/^(1|1st|first|one)( ed\.?| edition)?$/i", $node->biblio_edition))      // edition
      {
        if (!ereg("[?!.][ \"<i>]*$", $output))
        $output .= ".";

        if (preg_match("/^\d{1,3}$/", $node->biblio_edition)) // if the edition field contains a number of up to three digits, we assume it's an edition number (such as "2nd ed.")
        {
          if ($node->biblio_edition == "2")
          $editionSuffix = "nd";
          elseif ($node->biblio_edition == "3")
          $editionSuffix = "rd";
          else
          $editionSuffix = "th";
        }
        else
        $editionSuffix = "";

        if (!preg_match("/( ed\.?| edition)$/i", $node->biblio_edition))
        $editionSuffix .= " ed.";

        $output .= " " . $node->biblio_edition . $editionSuffix;
      }

      if (!empty($node->biblio_volume))      // volume
      {
        if (!ereg("[?!.][ \"<i>]*$", $output))
        $output .= ".";

        $output .= " Vol. " . $node->biblio_volume;
      }

      if (!empty($node->biblio_alternate_title) OR !empty($node->biblio_tertiary_title)) // if there's either a full or an abbreviated series title
      {
        if (!ereg("[?!.][ \"<i>]*$", $output))
        $output .= ".";

        $output .= " ";

        if (!empty($node->biblio_alternate_title))
        $output .= $node->biblio_alternate_title;      // abbreviated series title

        // if there's no abbreviated series title, we'll use the full series title instead:
        elseif (!empty($node->biblio_tertiary_title))
        $output .= $node->biblio_tertiary_title;      // full series title

        if (!empty($node->biblio_volume)||!empty($node->biblio_issue))
        $output .= " ";

        if (!empty($node->biblio_volume))      // series volume
        $output .= $node->biblio_volume;

        if (!empty($node->biblio_issue))      // series issue (I'm not really sure if -- for this cite style -- the series issue should be rather omitted here)
        $output .= ", no. " . $node->biblio_issue; // is it correct to format series issues similar to journal article issues?
      }

      if (!ereg("[?!.][ \"<i>]*$", $output))
      $output .= ".";

      if (!empty($node->biblio_place_published))      // place
      $output .= " " . $node->biblio_place_published;

      if (!empty($node->biblio_publisher))      // publisher
      {
        if (!empty($node->biblio_place_published))
        $output .= ":";

        $output .= " " . $node->biblio_publisher;
      }

      if (!empty($node->biblio_year))      // year
      $output .= ", " . $node->biblio_year;

      if (!ereg("\. *$", $output))
      $output .= ".";

      break;

    default : // all other types
      //TODO
      //        if (ereg("[ \r\n]*\(ed\)", $node->author)) // single editor
      //          $author = $author . ", ed";
      //        elseif (ereg("[ \r\n]*\(eds\)", $node->author)) // multiple editors
      //          $author = $author . ", eds";

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
          $output .= ". </span> \n";
          if (!ereg("[?!.]$", $node->title))
          $output .= ".";
          $output .= '"';
        }
        else // not a thesis
        $output .= '<i>';
        $output .= '<span class="biblio-title">';
        $output .= l($node->title, $url['link'], $url['options']);
        $output .= ". </span> \n";
        $output .= '</i>';
      }
      $publication = ereg_replace("[ \r\n]*\(Eds?:[^\)\r\n]*\)", "", $node->biblio_secondary_title);
      if (!empty($publication))      // publication
      $output .= " In <i>$publication</i>";

      if (count($node->biblio_contributors[2]))      // editor (if different from author, see note above regarding the check for ' (ed)' or ' (eds)')
      {

        $editor_options = array(
          'BetweenAuthorsDelimStandard'     => ', ',
          'BetweenAuthorsDelimLastAuthor'   => ' and ',
          'AuthorsInitialsDelimFirstAuthor' => ' ',
          'AuthorsInitialsDelimStandard'    => ' ',
          'betweenInitialsDelim'            => '. ',
          'initialsBeforeAuthorFirstAuthor' => TRUE,
          'initialsBeforeAuthorStandard'    => TRUE,
          'shortenGivenNames'               => FALSE,
          'numberOfAuthorsTriggeringEtAl'   => 10,
          'includeNumberOfAuthors'          => 10,
          'customStringAfterFirstAuthors'   => ' et al.',
          'encodeHTML'                      => true
        );

        $editor = theme('biblio_format_authors', $node->biblio_contributors[2], $editor_options, $inline);
        $output .= ", Edited by " . $editor;
      }

      if (!empty($node->biblio_edition) && !preg_match("/^(1|1st|first|one)( ed\.?| edition)?$/i", $node->biblio_edition))      // edition
      {
        if (!ereg("[?!.][ \"<i>]*$", $output))
        $output .= ".";

        if (preg_match("/^\d{1,3}$/", $node->biblio_edition)) // if the edition field contains a number of up to three digits, we assume it's an edition number (such as "2nd ed.")
        {
          if ($node->biblio_edition == "2")
          $editionSuffix = "nd";
          elseif ($node->biblio_edition == "3")
          $editionSuffix = "rd";
          else
          $editionSuffix = "th";
        }
        else
        $editionSuffix = "";

        if (!preg_match("/( ed\.?| edition)$/i", $node->biblio_edition))
        $editionSuffix .= " ed.";

        $output .= " " . $node->biblio_edition . $editionSuffix;
      }

      if (!empty($node->biblio_volume))      // volume
      {
        if (!ereg("[?!.][ \"<i>]*$", $output))
        $output .= ".";

        $output .= " Vol. " . $node->biblio_volume;
      }

      if (!empty($node->biblio_alternate_title) OR !empty($node->biblio_tertiary_title)) // if there's either a full or an abbreviated series title
      {
        if (!ereg("[?!.][ \"<i>]*$", $output))
        $output .= ".";

        $output .= " ";

        if (!empty($node->biblio_alternate_title))
        $output .= $node->biblio_alternate_title;      // abbreviated series title

        // if there's no abbreviated series title, we'll use the full series title instead:
        elseif (!empty($node->biblio_tertiary_title))
        $output .= $node->biblio_tertiary_title;      // full series title

        if (!empty($node->biblio_volume)||!empty($node->biblio_issue))
        $output .= " ";

        if (!empty($node->biblio_volume))      // series volume
        $output .= $node->biblio_volume;

        if (!empty($node->biblio_issue))      // series issue (I'm not really sure if -- for this cite style -- the series issue should be rather omitted here)
        $output .= ", no. " . $node->biblio_issue; // is it correct to format series issues similar to journal article issues?
      }

      if (!empty($node->thesis))      // thesis
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
        $output .= ".";

        $output .= " " . $node->thesis;
        $output .= ", " . $node->biblio_publisher;
      }
      else // not a thesis
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
        $output .= ".";

        if (!empty($node->biblio_place_published))      // place
        $output .= " " . $node->biblio_place_published;

        if (!empty($node->biblio_publisher))      // publisher
        {
          if (!empty($node->biblio_place_published))
          $output .= ":";

          $output .= " " . $node->biblio_publisher;
        }
      }

      if (!empty($node->biblio_year))      // year
      $output .= ", ".$node->biblio_year;

      if ($node->online_publication == "yes") // this record refers to an online article
      {
        // append an optional string (given in 'online_citation') plus the current date and the DOI (or URL):

        $today = date("F j, Y");

        if (!empty($node->online_citation))      // online_citation
        {
          if (!ereg("\. *$", $output))
          $output .= ".";

          $output .= " " . $node->online_citation;
        }

        if (!empty($node->doi))      // doi
        {
          if (!ereg("\. *$", $output))
          $output .= ".";

          if ($encodeHTML)
          $output .= " " . encodeHTML("http://dx.doi.org/" . $node->doi) . " (accessed " . $today . ")";
          else
          $output .= " " . "http://dx.doi.org/" . $node->doi . " (accessed " . $today . ")";
        }
        elseif (!empty($node->biblio_url))      // url
        {
          if (!ereg("\. *$", $output))
          $output .= ".";

          if ($encodeHTML)
          $output .= " " . encodeHTML($node->biblio_url) . " (accessed " . $today . ")";
          else
          $output .= " " . $node->biblio_url . " (accessed " . $today . ")";
        }

      }

      if (!ereg("\. *$", $output))
      $output .= ".";
      break;
  }

  return filter_xss($output, biblio_get_allowed_tags());
}

