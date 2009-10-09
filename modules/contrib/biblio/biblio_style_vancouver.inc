<?php
  // Original File:       ./cite/styles/cite_Vancouver.php
  // Original Author(s):  Matthias Steffens <mailto:refbase@extracts.de>

  // This Vancouver style was modeled after these resources:
  // <http://www.library.uq.edu.au/training/citation/vancouv.pdf>
  // <http://library.curtin.edu.au/research_and_information_skills/referencing/vancouver.pdf>
  // <http://www.icmje.org/index.html> citing: <http://www.nlm.nih.gov/citingmedicine/>
  // <http://www.nlm.nih.gov/bsd/uniform_requirements.html>
  // <http://library.sun.ac.za//eng/help/infolit2002/bibvancouver.htm>

  // based on 'cite_AMA.php'

  // NOTES: - In the Vancouver style, the reference list is arranged numerically in the order in which references are cited in the text.
  //          This isn't currently handled by this style
  //        - For conference proceedings, you'll currently need to add the place & date of the conference in the proceedings title field
  //          (e.g. "Proceedings of the 5th Germ Cell Tumour Conference; 2001 Sep 13-15; Leeds, UK").

  // TODO: - abstracts, newspaper/magazine articles, patents & reports?
  //       - arrange references numerically
  //       - for newspaper articles, only the beginning page number of an article should be included (see: <http://www.ncbi.nlm.nih.gov/books/bv.fcgi?rid=citmed.section.41496#41607>)
  //       - where to put (and how to format) editors of whole books that also have an author?
  //       - see also inline comments labeled with TODO (and NOTE)


// Modified for use in biblio by Ron Jerome
//
// $Id: biblio_style_vancouver.inc,v 1.1.2.10 2009/09/30 15:48:25 rjerome Exp $
/**
 * Get the style information
 *
 * @return
 *   The name of the style
 */
function biblio_style_vancouver_info() {
  return array (
    'vancouver' => 'Vancouver'
    );
}
function biblio_style_vancouver_author_options() {
  $author_options = array(
    'BetweenAuthorsDelimStandard'       => ', ',
    'BetweenAuthorsDelimLastAuthor'     => ', ',
    'AuthorsInitialsDelimFirstAuthor'   => ' ',
    'AuthorsInitialsDelimStandard'      => ' ',
    'betweenInitialsDelim'              => '',
    'initialsBeforeAuthorFirstAuthor'   => FALSE,
    'initialsBeforeAuthorStandard'      => FALSE,
    'numberOfInitialsToKeep'            => 2,
    'shortenGivenNames'                 => TRUE,
    'numberOfAuthorsTriggeringEtAl'     => 6,
    'includeNumberOfAuthors'            => 6,
    'customStringAfterFirstAuthors'     => ', et al.',
    'encodeHTML'                        => true
  );

  return $author_options;
}

function biblio_style_vancouver($node, $base = 'biblio', $inline = false) {
  $markupPatternsArray = array("italic-prefix" => '<i>',
                               "italic-suffix" => '</i>',
                               "endash" => '-');
  $author_options = biblio_style_vancouver_author_options();
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
        $output .= '<span class="biblio-authors">';
        if (!ereg("\. *$", $authors)) {
          $output .= $authors . ".";
        }
        else {
          $output .= $authors;
        }
        $output .=  "</span>";
      }

      if (!empty($node->title))      // title
      {
        $output .= '<span class="biblio-title">';
        if (!empty($authors)) $output .= " ";
        $output .= '<span class="biblio-title">';
        $url = biblio_get_title_url_info($node);
        $output .= l($node->title, $url['link'], $url['options']);
        $output .= ". </span> \n";
        if (!ereg("[?!.]$", $node->title)) $output .= ".";
        $output .= '</span>';
      }

      // From here on we'll assume that at least either the 'author' or the 'title' field did contain some contents
      // if this is not the case, the output string will begin with a space. However, any preceding/trailing whitespace will be removed at the cleanup stage (see below)

      if (!empty($node->biblio_alternate_title))      // abbreviated journal name
        $output .= " " . preg_replace("/\./", "", $node->biblio_alternate_title); // no punctuation marks are used in the abbreviated journal name, just spaces (TODO: smarten regex pattern)

      // if there's no abbreviated journal name, we'll use the full journal name instead:
      elseif (!empty($node->biblio_secondary_title))      // publication (= journal) name
        $output .= " " . $node->biblio_secondary_title;

      if ($row['online_publication'] == "yes") // this record refers to an online publication
        $output .= " [Internet]"; // NOTE: some of the above mentioned resources use "[serial online]", "[serial on the Internet]" or just "[online]" instead

      // NOTE: the formatting of year/volume/issue is meant for journal articles (TODO: newspaper/magazine articles)
      if (!empty($node->biblio_year))      // year
        $output .= ". " . $node->biblio_year;

      if ($row['online_publication'] == "yes") // append the current date if this record refers to an online publication
        $output .= " [cited " . date("Y M j") . "]";

      if (!empty($node->biblio_volume) || !empty($node->biblio_issue))
        $output .= ";";

      if (!empty($node->biblio_volume))      // volume (=month)
        $output .= $node->biblio_volume;

      if (!empty($node->biblio_issue))      // issue (=day)
        $output .=  "(" . $node->biblio_issue . ")";

      if (!empty($node->biblio_pages))      // pages
      {
        if (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)) // only add ": " if either year, volume, issue, abbrev_journal or publication isn't empty
          $output .= ":";

        $output .= theme_biblio_page_number($node->biblio_pages, $markupPatternsArray["endash"], "", "", "", "", "", "", true); // function 'formatPageInfo()' is defined in 'cite.inc.php'
      }

      if ($row['online_publication'] == "yes") // this record refers to an online publication
      {
        // append an optional string (given in 'online_citation') plus the DOI (or URL):

        if (!empty($row['online_citation']))      // online_citation
        {
          if (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)) // only add ":" or "," if either year, volume, issue, abbrev_journal or publication isn't empty
          {
            if (empty($node->biblio_pages))
              $output .= ":"; // print instead of pages
            else
              $output .= ";"; // append to pages (TODO: not sure whether this is correct)
          }

          $output .= $row['online_citation'];
        }

        if (!empty($row['doi']) || !empty($row['url']))      // doi OR url
        {
          if (!empty($row['online_citation']) OR (empty($row['online_citation']) AND (!empty($node->biblio_year) || !empty($node->biblio_volume) || !empty($node->biblio_issue) || !empty($node->biblio_alternate_title) || !empty($node->biblio_secondary_title)))) // only add "." if online_citation isn't empty, or else if either year, volume, issue, abbrev_journal or publication isn't empty
            $output .= ".";

          $output .= " Available from: " . $markupPatternsArray["underline-prefix"]; // NOTE: some of the above mentioned resources use "Available from: URL:http://..." instead

          if (!empty($row['doi']))      // doi
            $uri = "http://dx.doi.org/" . $row['doi'];
          else      // url
            $uri = $row['url'];

          if ($encodeHTML)
            $output .= encodeHTML($uri);
          else
            $output .= $uri;

          $output .= $markupPatternsArray["underline-suffix"];
        }
      }

      if (!ereg("\. *$", $output) AND ($row['online_publication'] != "yes"))
        $output .= "."; // NOTE: the examples in the above mentioned resources differ wildly w.r.t. whether the closing period should be omitted for online publications
      break;
    case 101: //Book Chapter
    case 103: //Conference Paper
      if(!empty($authors)) {
        $output .= '<span class="biblio-authors">';
        if (!ereg("\. *$", $authors)) {
          $output .= $authors . ".";
        }
        else {
          $output .= $authors;
        }
        $output .= '</span>';
      }
      if (!empty($node->title))      // title
      {
        if (!empty($authors))  $output .= " ";
        $output .= '<span class="biblio-title">';
        $url = biblio_get_title_url_info($node);
        $output .= l($node->title, $url['link'], $url['options']);
        $output .= ". </span> \n";
        if (!ereg("[?!.]$", $node->title)) $output .= ".";
      }


      if (!empty($node->biblio_contributors[2]))      // editor
      {
        $editor_options = array(
          'BetweenAuthorsDelimStandard'       => ', ',
          'BetweenAuthorsDelimLastAuthor'     => ', ',
          'AuthorsInitialsDelimFirstAuthor'   => ' ',
          'AuthorsInitialsDelimStandard'      => ' ',
          'betweenInitialsDelim'              => '',
          'initialsBeforeAuthorFirstAuthor'   => FALSE,
          'initialsBeforeAuthorStandard'      => FALSE,
          'shortenGivenNames'                 => TRUE,
          'numberOfAuthorsTriggeringEtAl'     => 6,
          'includeNumberOfAuthors'            => 6,
          'customStringAfterFirstAuthors'     => ' et al.',
          'encodeHTML'                        => true
        );

        $editor = theme('biblio_format_authors', $node->biblio_contributors[2], $editor_options, $inline);

        $output .= " In: " . $editor . ", ";
        if (count ($node->biblio_contributors[2]) > 1) // there are at least two editors (separated by ';')
        $output .= "editors";
        else // there's only one editor (or the editor field is malformed with multiple editors but missing ';' separator[s])
        $output .= "editor";
      }

      $publication = ereg_replace("[ \r\n]*\(Eds?:[^\)\r\n]*\)", "", $node->biblio_secondary_title);
      if (!empty($publication))      // publication
      {
        if (!ereg("[?!.] *$", $output))
          $output .= ".";

        if (empty($editor))
          $output .= " In:";

        $output .= " " . $publication;
      }

      if (!empty($node->biblio_volume))      // volume
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
          $output .= ".";

        $output .= " Vol " . $node->biblio_volume; // TODO: not sure whether this is correct
      }

      if (!empty($node->biblio_edition) && !preg_match("/^(1|1st|first|one)( ed\.?| edition)?$/i", $node->biblio_edition))      // edition
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
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

      if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
        $output .= ".";

      if (!empty($node->biblio_place_published))      // place
      {
//        // for places in the USA, format any two-letter postal code for the state (i.e. ensure upper case & wrap in parens, eg. "Boca Raton (FL)"):
//        if (preg_match("/(.+?)[$punct$space]+($uspsStateAbbreviations)[$punct$space]*$/i$patternModifiers", $node->biblio_place_published))
//          $output .= " " . preg_replace("/(.+?)[$punct$space]+($uspsStateAbbreviations)[$punct$space]*$/ie$patternModifiers", "'\\1 ('.strtoupper('\\2').')'", $node->biblio_place_published);
//        else
          $output .= " " . $node->biblio_place_published;
      }

      if (!empty($node->biblio_publisher))      // publisher
      {
        if (!empty($node->biblio_place_published))
          $output .= ":";

        $output .= " " . $node->biblio_publisher;
      }

      if (!empty($node->biblio_year))      // year
        $output .= "; " . $node->biblio_year;

      if (!empty($node->biblio_pages))      // pages
        $output .= ". " . theme_biblio_page_number($node->biblio_pages, $markupPatternsArray["endash"], "p. ", "p. ", "", "", "", "", true); // function 'formatPageInfo()' is defined in 'cite.inc.php'

      if (!empty($node->biblio_alternate_title) OR !empty($node->biblio_tertiary_title)) // if there's either a full or an abbreviated series title
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
          $output .= ".";

        $output .= " (";

        if (!empty($node->biblio_alternate_title))      // abbreviated series title
          $output .= preg_replace("/\./", "", $node->biblio_alternate_title); // no punctuation marks are used in the abbreviated series title, just spaces (TODO: smarten regex pattern)

        // if there's no abbreviated series title, we'll use the full series title instead:
        elseif (!empty($node->biblio_tertiary_title))      // full series title
          $output .= $node->biblio_tertiary_title;

        if (!empty($node->biblio_volume)||!empty($node->biblio_issue))
          $output .= "; ";

        if (!empty($node->biblio_volume))      // series volume
          $output .= "vol " . $node->biblio_volume;

        if (!empty($node->biblio_volume) && !empty($node->biblio_issue))
          $output .= "; "; // TODO: not sure whether this is correct

        if (!empty($node->biblio_issue))      // series issue (I'm not really sure if -- for this cite style -- the series issue should be rather omitted here)
          $output .= "no " . $node->biblio_issue; // since a series volume should be prefixed with "vol", is it correct to prefix series issues with "no"?

        $output .= ")";
      }

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
        $output .= '<span class="biblio-authors">';
        if (!ereg("\. *$", $authors))
        $output .= $authors . ".";
        else
        $output .= $authors;
        $output .= '</span>';
      }

      if (!empty($node->title))      // title
      {
        if (!empty($authors))
        $output .= " ";

        // TODO: book/volume/report/etc titles should be formatted in heading caps, however, this doesn't yet work correctly if the publication title contains HTML entities
          $output .= '<span class="biblio-title">';
          $url = biblio_get_title_url_info($node);
          $output .= l($node->title, $url['link'], $url['options']);
          $output .= ". </span> \n";
       }
      if ($row['type'] == "Software") // for software, add software label
        $output .= " [computer program]";

      if (($row['online_publication'] == "yes") AND empty($row['thesis'])) // this record refers to an online publication (online theses will be handled further down below)
        $output .= " [Internet]"; // NOTE: some of the above mentioned resources use "[monograph online]", "[monograph on the Internet]" or just "[online]" instead

      if (!empty($node->biblio_volume) AND ($row['type'] != "Software"))      // volume
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
          $output .= ".";

        $output .= " Vol " . $node->biblio_volume; // TODO: not sure whether this is correct
      }

      if (!empty($node->biblio_edition))      // edition
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
          $output .= ".";

        if ($row['type'] == "Software")      // software edition (=version)
        {
          $output .= " Version " . $node->biblio_edition;
        }
        elseif (!preg_match("/^(1|1st|first|one)( ed\.?| edition)?$/i", $node->biblio_edition))      // edition
        {
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
      }

      if (count($node->biblio_contributors[2]))      // editor (if different from author, see note above regarding the check for ' (ed)' or ' (eds)')
      {

        $editor_options = array(
          'BetweenAuthorsDelimStandard'     => ', ',
          'BetweenAuthorsDelimLastAuthor'   => ', ',
          'AuthorsInitialsDelimFirstAuthor' => ' ',
          'AuthorsInitialsDelimStandard'    => ' ',
          'betweenInitialsDelim'            => '',
          'initialsBeforeAuthorFirstAuthor' => FALSE,
          'initialsBeforeAuthorStandard'    => FALSE,
          'shortenGivenNames'               => TRUE,
          'numberOfAuthorsTriggeringEtAl'   => 6,
          'includeNumberOfAuthors'          => 3,
          'customStringAfterFirstAuthors'   => ' et al.',
          'encodeHTML'                      => true
        );

        $editor = theme('biblio_format_authors', $node->biblio_contributors[2], $editor_options, $inline);
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
        $output .= ".";

        $output .= " " . $editor;
        if (count($node->biblio_contributors[2]) > 1) // there are at least two editors (separated by ';')
        $output .= ", editors";
        else // there's only one editor (or the editor field is malformed with multiple editors but missing ';' separator[s])
        $output .= ", editor";
      }

      if (!empty($row['thesis']))      // thesis
      {
        // TODO: do we need to use the term "[dissertation]" instead of "[Ph.D. thesis]", etc? What about other thesis types then?
        $output .= " [" . $row['thesis'];

        if ($row['online_publication'] == "yes") // this record refers to an online thesis
          $output .= " on the Internet]";
        else
          $output .= "]";
      }

      if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
        $output .= ".";

      if (!empty($node->biblio_place_published))      // place
      {
//        // for places in the USA, format any two-letter postal code for the state (i.e. ensure upper case & wrap in parentheses, eg. "Boca Raton (FL)"):
//        if (preg_match("/(.+?)[$punct$space]+($uspsStateAbbreviations)[$punct$space]*$/i$patternModifiers", $node->biblio_place_published))
//          $output .= " " . preg_replace("/(.+?)[$punct$space]+($uspsStateAbbreviations)[$punct$space]*$/ie$patternModifiers", "'\\1 ('.strtoupper('\\2').')'", $node->biblio_place_published);
//        else
          $output .= " " . $node->biblio_place_published;
      }

      if (!empty($node->biblio_publisher))      // publisher
      {
        if (!empty($node->biblio_place_published))
          $output .= ":";

        $output .= " " . $node->biblio_publisher;
      }

      $output .= ";";

      if (!empty($node->biblio_year))      // year
        $output .= " " . $node->biblio_year;

      if ($row['type'] == "Software")      // for software, volume (=month) and issue (=day) information is printed after the year (TODO: not sure whether this is correct)
      {
        if (!empty($node->biblio_volume))      // volume (=month)
          $output .= " " . $node->biblio_volume;

        if (!empty($node->biblio_issue))      // issue (=day)
          $output .= " " . $node->biblio_issue;
      }

      if ($row['online_publication'] == "yes") // append the current date if this record refers to an online publication
        $output .= " [cited " . date("Y M j") . "]";

      if (!empty($node->biblio_alternate_title) OR !empty($node->biblio_tertiary_title)) // if there's either a full or an abbreviated series title
      {
        if (!ereg("[?!.][ \"" . $markupPatternsArray["italic-suffix"] . "]*$", $output))
          $output .= ".";

        $output .= " (";

        if (!empty($node->biblio_alternate_title))      // abbreviated series title
          $output .= preg_replace("/\./", "", $node->biblio_alternate_title); // no punctuation marks are used in the abbreviated series title, just spaces (TODO: smarten regex pattern)

        // if there's no abbreviated series title, we'll use the full series title instead:
        elseif (!empty($node->biblio_tertiary_title))      // full series title
          $output .= $node->biblio_tertiary_title;

        if (!empty($node->biblio_volume)||!empty($node->biblio_issue))
          $output .= "; ";

        if (!empty($node->biblio_volume))      // series volume
          $output .= "vol " . $node->biblio_volume;

        if (!empty($node->biblio_volume) && !empty($node->biblio_issue))
          $output .= "; "; // TODO: not sure whether this is correct

        if (!empty($node->biblio_issue))      // series issue (I'm not really sure if -- for this cite style -- the series issue should be rather omitted here)
          $output .= "no " . $node->biblio_issue; // since a series volume should be prefixed with "vol", is it correct to prefix series issues with "no"?

        $output .= ")";
      }

      if ($row['online_publication'] == "yes" || $row['type'] == "Software") // this record refers to an online publication, or a computer program/software
      {
        // append an optional string (given in 'online_citation') plus the DOI (or URL):

        if (!empty($row['online_citation']))      // online_citation
        {
          if (!ereg("\. *$", $output))
            $output .= ".";

          $output .= $row['online_citation'];
        }

        if (!empty($row['doi']) || !empty($row['url']))      // doi OR url
        {
          if (!ereg("\. *$", $output))
            $output .= ".";

          $output .= " Available from: " . $markupPatternsArray["underline-prefix"]; // NOTE: some of the above mentioned resources use "Available from: URL:http://..." instead

          if (!empty($row['doi']))      // doi
            $uri = "http://dx.doi.org/" . $row['doi'];
          else      // url
            $uri = $row['url'];

          if ($encodeHTML)
            $output .= encodeHTML($uri);
          else
            $output .= $uri;

          $output .= $markupPatternsArray["underline-suffix"];
        }
      }

      if (!ereg("\. *$", $output) AND ($row['online_publication'] != "yes") AND ($row['type'] != "Software"))
        $output .= "."; // NOTE: the examples in the above mentioned resources differ wildly w.r.t. whether the closing period should be omitted for online publications
      break;
  }

  return filter_xss($output, biblio_get_allowed_tags());
}

