<?PHP
// $Id: biblio.pages.inc,v 1.1.2.30 2008/11/29 03:27:10 rjerome Exp $
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

/**
 * @return unknown_type
 */
function biblio_db_search() {

  $arg_list = func_get_args();
  // Drupal search? It returns an array of search results. We store the nids
  // of the result nodes and make them a "where n.nid in {..}" filter.
  // After installing the filter, we can go on as usual.

  // When called manually, search takes one parameter, i.e.
  // biblio_db_search('search', 'bla blu')
  $search = array_search('search', $arg_list);
  if ($search !== FALSE) {
    $keys = $arg_list[$search+1];
  }
  else {
    $keys = _get_biblio_search_filter();
  }

  if ($keys) {
    // Only search in biblio nodes. If we use a SESSION filter and have a list of
    // nids stored there, don't re-search. List is reset when submitting a new search.
    if ($search !== FALSE || !_get_biblio_search_filter('nodelist')) {
      if ($result = node_search('search',$keys.' type:biblio')) {
        $node_list = '';
        foreach ($result as $res) {
          $node_list .= $res['node']->nid.",";
        }
      }
      else {
        // No node search result. Make sure we find nothing, too. Node -1 does not exist.
        $node_list = '-1';
      }
      // Store as SESSION filter or argument list.
      // When called as function argument it takes only one parameter, i.e.
      // biblio_db_search('search', 'bla blu') and we must insert the node
      // list inbetween.
      if ($search !== FALSE) {
        array_splice($arg_list,$search+1,0,$node_list);
      }
      else {
        $_SESSION['biblio_filter'] = array(array('search',rtrim($node_list,','),$keys));
      }
    }
  }

  $inline = in_array('inline', $arg_list);
  $inline = in_array('profile',$arg_list)?'profile':$inline;
  $query_info = biblio_build_query($arg_list);
  if ($query_info['rss']['feed']){
  	 biblio_filter_feed($query_info['query'], $query_info['query_terms'], $query_info['rss']);
  }else{
    $count = db_result(db_query($query_info['count_query'],$query_info['query_terms']));
    $result = pager_query($query_info['query'], variable_get('biblio_rowsperpage', 25),0,$query_info['count_query'],$query_info['query_terms']);
    return biblio_show_results($result, $count, $query_info['sort_attrib'], $query_info['args'],$inline);
  }

}
/*
 * biblio_db_search builds the SQL query which will be used to
 * select and order "biblio" type nodes.  The query results are
 * then passed to biblio_show_results for output
 *
 *
 */
function biblio_build_query($arg_list) {
  global $user;
  static $bcc; //biblio_contributor (bc) count , increase for every invocation
  if ( !isset( $bcc ) ) $bcc = 0;
  $inline = $rss_info['feed'] = false;
  $join = array();
  $join[] = "left join {biblio} b  on n.vid=b.vid ";
  $join[] = "left join {biblio_types} bt  on b.biblio_type=bt.tid ";

  if (variable_get('biblio_view_only_own', 0) ) {
    $limit .= " AND n.uid=$user->uid ";
  }

  $sort_attrib = array("sort" => variable_get('biblio_sort', 'year'), 'order' => variable_get('biblio_order', 'DESC'));
  $limits = null;

  if (isset($_GET['order'])) {
    $sort_attrib['order'] = ($_GET['order'] == 'desc') ? 'desc' : 'asc';
  }
  if (isset($_GET['sort'])) {
    $sort_attrib['sort'] = check_plain($_GET['sort']);
  }

  switch ($sort_attrib['sort']) {
    case 'type':
      $sortby = "ORDER BY b.biblio_type %s, SUBSTRING(n.title,1,3) ASC ";
      break;
    case 'title':
      //$sortby = "ORDER BY SUBSTRING(n.title,1,3) %s, b.biblio_year DESC ";
      $sortby = 'ORDER BY
  					CASE
    					WHEN LEFT(n.title,1)="\""   THEN MID(title,2)
    					WHEN LEFT(n.title,1)="\'"   THEN MID(title,2)
    					WHEN LEFT(n.title,2)="A "   THEN MID(title,3)
    					WHEN LEFT(n.title,3)="An "  THEN MID(title,4)
    					WHEN LEFT(n.title,4)="The " THEN MID(title,5)
    					ELSE title
  					END  %s';
      break;
    case 'author':
      $sortby = "ORDER BY bcd.lastname %s ";
      $where['bc-rank'] = "bc.rank=0";
      //$where[] = "bc.ctid=1";
      $join['bc'] = '  JOIN {biblio_contributor} as bc on b.vid = bc.vid ';
      $join['bcd'] = '  JOIN {biblio_contributor_data} as bcd on bc.cid = bcd.cid ';
      break;
    case 'keyword': // added msh 070808
      $sortby = "ORDER BY SUBSTRING(LTRIM(b.biblio_keywords),1,4) %s ";
      break;
    case 'year':
    default:
      $sortby = "ORDER BY b.biblio_year %s,  SUBSTRING(n.title,1,3) ASC ";
  } //end switch
  if (!isset($_SESSION['biblio_filter']) || !is_array($_SESSION['biblio_filter'])) {
    $_SESSION['biblio_filter'] = array();
  }

  $session = &$_SESSION['biblio_filter'];

  foreach ($session as $filter) {
    $arg_list = array_merge($arg_list, $filter);
  }

  if (count($arg_list) ) {
    $args = array();
    while ($arg_list) {
      $type = $arg_list[0];
      array_shift($arg_list);
      $operator = ($operator)?$operator:" AND "; //defaults to AND
      switch ($type) {
        case 'and':
          $operator = " AND ";
          break;
        case 'or':
          $operator = " OR ";
          break;
        case 'inline':
          $inline = true;
          break;
        case 'rss.xml':
          $rss_info['feed'] = true;
          break;
        case 'profile':
          $inline = "profile";
          break;
        case 'cid':
        case 'aid':
          $bcc++;
          $term = explode("?",array_shift($arg_list));
          $join[] = "inner join {biblio_contributor} as bc". $bcc ." on n.vid = bc". $bcc .".vid";
          $where[] = "bc". $bcc .".cid = '%d' ";
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          break;
        case 'term':
        case 'term_id':
          $term = explode("?",array_shift($arg_list));
          $join[] = "inner join {term_node} as tn on n.vid = tn.vid";
          if ($type == 'term') {
            $join[] = "inner join  {term_data} as td on tn.tid= td.tid";
            $where[] = "td.name = '%s' ";
          }elseif ($type == 'term_id') {
            $where[] = "tn.tid = '%d' ";
          }
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          break;
        case 'tg':
          $term = explode("?",array_shift($arg_list));
          $where[] = ' CASE
    					WHEN LEFT(n.title,1)="\""   THEN SUBSTRING(n.title,2,1)
    					WHEN LEFT(n.title,1)="\'"   THEN SUBSTRING(n.title,2,1)
    					WHEN LEFT(n.title,2)="A "   THEN SUBSTRING(n.title,3,1)
    					WHEN LEFT(n.title,3)="An "  THEN SUBSTRING(n.title,4,1)
    					WHEN LEFT(n.title,4)="The " THEN SUBSTRING(n.title,5,1)
    					ELSE SUBSTRING(n.title,1,1)
  					  END
  					  RLIKE "%s" ';
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          $operator = NULL;
          break;
        case 'ag': //selects entries whoose authors firstname starts with the letter provided
          $term = explode("?",array_shift($arg_list));
          $where[] = 'substring(bcd.lastname,1,1) RLIKE "[[:<:]]%s[[:>:]]" ';
          $join['bc'] = '  JOIN {biblio_contributor} as bc on b.vid = bc.vid ';
          $join['bcd'] = '  JOIN {biblio_contributor_data} as bcd on bc.cid = bcd.cid ';
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          $operator = NULL;
          break;
        case 'author':
          $bcc++;
          $term = explode("?",array_shift($arg_list));
          $where[] = " bcd". $bcc .'.name RLIKE "[[:<:]]%s[[:>:]]" ';
          $join[] = " JOIN {biblio_contributor} as bc". $bcc ." on b.vid = bc". $bcc .".vid ";
          $join[] = " JOIN {biblio_contributor_data} as bcd". $bcc ." on bc". $bcc .".cid = bcd".$bcc .".cid ";
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          $operator = NULL;
          $rss_info['title'] = t("Publications by " . $term[0]);
          $rss_info['description'] = t("These publications by %author are part of the works listed at %sitename", array('%author' => $term[0], '%sitename' => variable_get('site_name', 'Drupal')));
          $rss_info['link'] = '/author/' . $term[0];
          break;
        case 'publisher':
          $term = explode("?",array_shift($arg_list));
          $where[] = "b.biblio_publisher RLIKE '%s' ";
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          $operator = NULL;
          break;
        case 'year':
          $term = db_escape_string(array_shift($arg_list));
          $where[] = "b.biblio_year=%d ";
          //$limit .= " AND b.biblio_year=%d ";
          $terms[] = (int)$term;
          array_push($args, $type, (int)$term);
          $operator = NULL;
          break;
        case 'uid':
          $term = db_escape_string(array_shift($arg_list));
          $where[] = "n.uid=%d ";
          //$limit .= " AND b.biblio_year=%d ";
          $terms[] = (int)$term;
          array_push($args, $type, (int)$term);
          $operator = NULL;
          break;
        case 'keyword':
          $term = explode("?",array_shift($arg_list));
          $where[] = "b.biblio_keywords RLIKE '%s' ";
          $terms[] = db_escape_string($term[0]);
          array_push($args, $type, $term[0]);
          $operator = NULL;
          break;
        case 'type':
          $term = db_escape_string(array_shift($arg_list));
          $limit .= $operator. "b.biblio_type=%d ";
          $terms[] = (int)$term;
          array_push($args, $type, (int)$term);
          $operator = NULL;
          break;
        case 'order':
          $term = (db_escape_string(array_shift($arg_list)) == 'desc')?'desc':'asc';
          $sort_attrib['order'] = $term;
          array_push($args, $type, $term);
          break;
        case 'sort':
          $term = db_escape_string(array_shift($arg_list));
          $sort_attrib['sort'] = $term;
          array_push($args, $type, $term);
          switch ($term) {
            case 'type':
              $sortby = "ORDER BY b.biblio_type %s, b.biblio_year DESC, SUBSTRING(n.title,1,1) ASC ";
              break;
            case 'title':
              $sortby = "ORDER BY SUBSTRING(n.title,1,1) %s, b.biblio_year DESC ";
              break;
            case 'year':
            default:
              $sortby = "ORDER BY b.biblio_year %s, SUBSTRING(n.title,1,1) ASC, b.biblio_type ASC ";
          } //end switch
          break;
            case 'search':
              $term = explode("?",array_shift($arg_list));
              // Search filter has a third component, the keywords, here useless. Drop it.
              array_shift($arg_list);
              $result_nids = split(',', $term[0]);
              $where[] = "n.nid in (".db_placeholders($result_nids).")";
              foreach ($result_nids as $result_nid) {
                $terms[] = db_escape_string($result_nid);
                array_push($args, $type, $result_nid);
              }
              $operator = NULL;
              break;
            default:
              $fields = biblio_get_db_fields();
              $term = explode("?",array_shift($arg_list));
              if (in_array("biblio_$type",$fields))
              {
                $where[] = "b.biblio_$type RLIKE '%s' ";
                $terms[] = db_escape_string($term[0]);
                array_push($args, $type, $term[0]);
                $operator = NULL;
              }
              break;
      }
    }
  }
  $where[] = "n.type='biblio' ";
  if ($user->uid != 1 ) {
    $where[] = 'n.status = 1 ';
  }//show only published entries to everyone except admin

  $joins = implode(' ', $join);

  $where_clause = count($where) > 1 ? '('. implode(') AND (', $where) .')': $where[0];

  $query = db_rewrite_sql("SELECT DISTINCT n.*, b.*, bt.name as biblio_type_name FROM {node} n $joins  WHERE $where_clause $published $limit $sortby ");
  $count_query = db_rewrite_sql("SELECT COUNT(DISTINCT(n.nid)) FROM {node} n $joins  WHERE $where_clause $published $limit ");
  $_SESSION['last_biblio_query'] = $query;
  $terms[] = $sort_attrib['order']; // this is either asc or desc to be inserted into the first term of the ORDER clause
  $_SESSION['last_biblio_query_terms'] = $terms;

  return (array('query'      => $query,
               'query_terms' => $terms,
               'count_query' => $count_query,
               'args'        => $args,
               'sort_attrib' => $sort_attrib,
               'rss'         => $rss_info
  ));

}

/**
 * biblio_show_results takes the query results from biblio_db_search and
 * adds some controls to the page then loops through the results applying
 * the selected style to each entry
 *
 * @param $result
 * @param $count
 * @param $attrib
 * @param $args
 * @param $inline
 * @return unknown_type
 */
function biblio_show_results($result, $count = 0, $attrib = array("sort" => 'year', 'order' => 'DESC'), $args = null,$inline=false) {
  global $pager_total_items;
  
  module_load_include('inc','biblio','biblio.contributors');
  
  $base =  variable_get('biblio_base', 'biblio');
  $style = variable_get('biblio_style', 'cse');
  if (module_exists('popups')){
     popups_add_popups();
  }
  if (variable_get('biblio_rss', 0)) {
    drupal_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.variable_get('site_name', 'Drupal').' RSS" href="'.url("$base/rss.xml").'" />');
  }
  $pub_types = db_query('SELECT t.* FROM {biblio_types} as t WHERE t.tid>0');
  while ($option = db_fetch_object($pub_types)) {
    $pub_type["$option->tid"] = $option->name;
  }
  if (!$inline) {
    // Search box. Has same permissions as the filter tab.
    if (variable_get('biblio_search',0) && user_access('show filter tab')) {
      $content .= drupal_get_form('biblio_search_form');
    }
    // Add some links to the top of the page to change the sorting/ordering...
    $order = ($attrib['order'] == "desc" || $attrib['order'] == "DESC")?"asc":"desc";
    if ( biblio_access('export')) {
//      $content .= '<div class="biblio-export">'.t('Export @count results', array('@count' => $pager_total_items[0])).':['.l("Tagged","$base/export/tagged"). ']['.l("XML","$base/export/xml"). ']['.l("CSV","$base/export/csv").']['.l("BibTex","$base/export/bibtex").']</div>';
      $content .= '<div class="biblio-export">'.t('Export @count results', array('@count' => $pager_total_items[0])).':' . theme('biblio_export_links'). '</div>';
    }else{
      $content .= '<div class="biblio-export">'.t('Found @count results', array('@count' => $pager_total_items[0])) .'</div><br />';
    }
    if (user_access('show sort links')) {
      $options = array('attributes'=>array("title"=>t("Click a second time to reverse the sort order")));

      $sort_links =  variable_get('biblio_sort_tabs', array('author'=>'author','title'=>'title','type'=>'type','year'=>'year'));

      $content .= '<div class="biblio-sort">';
      foreach($sort_links as $key => $link) {
        if ($key === $link) {
          $content .= t('Sort by').':';
          break;
        }
      }

      if ($sort_links['year']) {
        $options['query'] = "sort=year&order=$order";
        $content .= "[".l(t("Year"),$_GET['q'],$options)."]";
      }
      if ($sort_links['title']) {
        $options['query'] = "sort=title&order=$order";
        $content .= "[".l(t("Title"),$_GET['q'],$options)."]";
      }
      if ($sort_links['type']) {
        $options['query'] = "sort=type&order=$order";
        $content .= "[".l(t("Type"),$_GET['q'],$options)."]";
      }
      if ($sort_links['author']) {
        $options['query'] = "sort=author&order=$order";
        $content .= "[".l(t("Author"),$_GET['q'],$options)."]";
      }
      if ($sort_links['keyword']) {
        $options['query'] = "sort=keyword&order=$order";
        $content .= "[".l(t("Keyword"),$_GET['q'],$options)."]";
      }
      $content .= "</div> ";
    }

    $session = &$_SESSION['biblio_filter'];
    // if there are any filters in place, print them at the top of the list
    if (count($args)) {
      $i = 0;
      while ($args) {
        $type = $args[0];
        array_shift($args);
        $value = db_escape_string($args[0]);
        if ($type == 'search') {
          $search_content = '<em>'._get_biblio_search_filter().'</em>';
          array_shift($args);
          continue;
        }
        if ($type == 'term_id') {
          $term = taxonomy_get_term($value);
          $value = $term->name;
          $type = t("Taxonomy Term");
        }
        if ($type == 'aid') {
          $author = biblio_get_contributor($value);
          $value = $author->name;
          $type = t("Author");
        }
        if ($type == 'type') $value = $pub_type["$value"];
        array_shift($args);
        $params = array('%a' =>  $type , '%b' =>  $value );
        $filtercontent .= ($i++ ? t('<em> and</em> <strong>%a</strong> is <strong>%b</strong>', $params) : t('<strong>%a</strong> is <strong>%b</strong>', $params)) ;
      }
      if ($search_content) {
        $content .= '<div class="biblio-current-filters"><b>'.t('Search results for ').'</b>';
        $content .= $search_content;
        if ($filtercontent) {
          $content .= '<br><b>'.t('Filters').': </b>';
        }
      }
      else {
        $content .= '<div class="biblio-current-filters"><b>'.t('Filters').': </b>';
      }
      $content .= $filtercontent;
      $link_options = array();
      if (isset($_GET['sort'])) {
        $link_options['query']  .= "sort=" . $_GET['sort'];
      }
  	   if (isset($_GET['order'])) {
  	     $link_options['query']   .= $options['query'] ? "&" : "" ;
  	     $link_options['query']   .= "order=" . $_GET['order'];
  	   }

      if ($search_content) {
        $content .= '&nbsp;&nbsp;'.l('['.t('Reset Search').']',"$base/filter/clear", $link_options);
      } else {
        $content .= '&nbsp;&nbsp;'.l('['.t('Clear All Filters').']',"$base/filter/clear", $link_options);
      }
      $content .= '</div>';
    }
  }
  if ($inline === true) print '<div class="biblio-inline">';
  $_char = $_type = $_year = $name = "";
  $content .= theme(biblio_alpha_line);
  while ($node = db_fetch_object($result)) {

    $node->biblio_contributors = biblio_load_contributors($node->vid);

    if ( module_exists("upload") ) {
      $node->files = upload_load($node);
    }

    if (isset($node->biblio_year) && ($node->biblio_year == 9998))  $node->biblio_year = t("In Press") ;
    if (isset($node->biblio_year) && ($node->biblio_year == 9999))  $node->biblio_year = t("Submitted") ;
    switch ($attrib['sort']) {
      case 'title':
        $first = drupal_substr(drupal_ucfirst(ltrim($node->title)), 0, 1);
        if (drupal_substr(drupal_ucfirst(ltrim($node->title)), 0, 1) == '"' ) $first = drupal_ucfirst(drupal_substr(ltrim($node->title), 1, 1));
        if (drupal_substr(drupal_ucfirst(ltrim($node->title)), 0, 1) == "'" ) $first = drupal_ucfirst(drupal_substr(ltrim($node->title), 1, 1));
        if (drupal_substr(drupal_ucfirst(ltrim($node->title)), 0, 2) == "A " ) $first = drupal_ucfirst(drupal_substr(ltrim($node->title), 2, 1));
        if (drupal_substr(drupal_ucfirst(ltrim($node->title)), 0, 3) == "An " ) $first = drupal_ucfirst(drupal_substr(ltrim($node->title), 3, 1));
        if (drupal_substr(drupal_ucfirst(ltrim($node->title)), 0, 4) == "The " ) $first = drupal_ucfirst(drupal_substr(ltrim($node->title), 4, 1));
        if ( $first  != $_char) {
          $_char =  $first ;
          $content .= '<div class="biblio-year">' . $_char . "</div>\n";
        }
        break;
      case 'author':
        if ( drupal_substr(drupal_ucfirst(ltrim($node->biblio_contributors[0]['lastname'])), 0, 1) != $_char) {
          $_char = drupal_substr(drupal_ucfirst(ltrim($node->biblio_contributors[0]['lastname'])), 0, 1) ;
          $content .= '<div class="biblio-year">' . $_char . "</div>\n";
        }
        break;
      case 'type':
        if ($node->biblio_type_name != $_type) {
          $_type = $node->biblio_type_name;
          //      $name = db_result(db_query("SELECT name FROM {biblio_types} as t where t.tid=%d", $node->biblio_type)) ;
          $content .= '<div class="biblio-year">' . $_type . "</div>\n";
        }
        break;
      case 'keyword': 	// added msh 08 aug 07
        $tok = drupal_ucfirst(strtok($node->biblio_keywords, variable_get('biblio_keyword_sep', ',')));
        if (empty($tok)) {
          $tok = t("No Keywords");
        }
        if ($tok != $name) {
          $name = $tok;
          if ($name != '') {
            $content .= '<div class="biblio-year">' . $name . "</div>\n";
          }
        }
        break;
      case 'year':
      default:
        if ($node->biblio_year != $_year) {
          $_year = $node->biblio_year;
          $content .= '<div class="biblio-year">' .$_year. "</div>\n";
        }
    } //end switch
    $inline_links = ($inline && variable_get('inlinemode_in_links',0)) ? true : false;
    $content .= theme('biblio_entry', $node,$base,$style,$inline);
  } //end while
  $content .= theme('pager', 0, variable_get('biblio_rowsperpage', 25));
  if ($count == 0) {
    $content .= "<h3>".t("No items found")."</h3>";
    if (strstr($content, "Filters:")) {
      $content .= t('!modify_link or !remove_link your filters and try again.', array('!modify_link' => l(t('Modify'),"$base/filter"), '!remove_link' => l(t('remove'),"$base/filter/clear")));
    }
  }

  if ($inline === true)       print $content . "</div>";
  if ($inline === false)      print theme('page', $content);
  if ($inline === "profile")  return $content;

}


/**
 * Add a search field on the main biblio page.
 */
/**
 * @param $form_state
 * @return unknown_type
 */
function biblio_search_form(&$form_state) {
  $form = array(
    '#attributes' => array('class' => 'biblio-search'),
  );
  $form['module'] = array('#type' => 'value', '#value' => $type);

  // Uncomment to collapse the search box.
  //$form['basic'] = array(
  //  '#attributes' => array('class' => 'biblio-fieldset'),
  //  '#type' => 'fieldset',
  //  '#collapsible' => TRUE,
  //  '#collapsed' => TRUE,
  //  '#title' => t('Quick Search'),
  //  '#description' => '',
  //  );

  $form['basic']['inline'] = array('#prefix' => '<div class="biblio-inline">', '#suffix' => '</div>');
  $form['basic']['inline']['keys'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => '',
    '#size' => 25,
    '#maxlength' => 255,
  );
  $button_text = strlen(variable_get('biblio_search_button_text', ''))?variable_get('biblio_search_button_text', t('Biblio search')):t('Biblio search');
  $form['basic']['inline']['submit'] = array('#type' => 'submit', '#value' => $button_text);
  return $form;
}

/**
 * We must validate the keyword length before calling node_search. Otherwise
 * we would store a keyword of e.g. length 1, but do_search would complain about
 * it later and resend us to the input form. But since the keyword was stored already
 * we could never again display the page without errors

 * @param $form
 * @param $form_state
 * @return unknown_type
 */
function biblio_search_form_validate($form, &$form_state) {
  // Empty searchbox finds everything.
  $keys = $form_state['values']['keys'];
  if ($keys != '')  {
    $query = search_parse_query($form_state['values']['keys']);
    if ($query[2] == '') {
      form_set_error('keys', t('You must include at least one positive keyword with @count characters or more.', array('@count' => variable_get('minimum_word_size', 3))));
    }
  }
}


/**
 * When we submit a search, we revoke all current filters since search
 * and filtering are considered two different concepts things* conceptually.
 *
 * But we store the results as a filter (which is just a list of node ids that
 * matched the search request) so that we can reorder or export the search
 * results like with any other filter.  The filter has three components:
 * ('search',<list of node ids>,<search keywords>).
 * The second component, the filter value, is empty when submitting keywords.
 * In biblio_db_search we fill the second component with the list of nids
 * matching our keywords, as returned by node_search.  We store the keywords
 * only for showing them in "Search results for <keywords>".
 */
/**
 * @param $form
 * @param $form_state
 * @return unknown_type
 */
function biblio_search_form_submit($form, &$form_state) {
  $keys = $form_state['values']['keys'];
  if ($keys != '')  {
    $_SESSION['biblio_filter'] = array(array('search','',$keys));
    $base =  variable_get('biblio_base', 'biblio');
    $form_state['redirect'] = $base;
  }
  else {
    // No keywords. Remove former search keys if any. Leaves other filters intact.
    if (_get_biblio_search_filter()){
      $_SESSION['biblio_filter'] = array();
    }
  }
}

/**
 * @param $arg
 * @return unknown_type
 */
function _get_biblio_search_filter($arg = 'keys') {
  if (variable_get('biblio_search',0) &&
    is_array($_SESSION['biblio_filter']) &&
    is_array($_SESSION['biblio_filter'][0]) &&
    in_array('search',$_SESSION['biblio_filter'][0])
  ){
    switch ($arg) {
      case 'keys': return $_SESSION['biblio_filter'][0][2]; break;
      case 'nodelist': return $_SESSION['biblio_filter'][0][1]; break;
    }
  }
}


function _get_biblio_filters() {

  $fields = " b.biblio_year, t.name , t.tid ";
  $order = " b.biblio_year DESC";
  $taxo_fields = "td.name as termname,td.tid as taxid, v.name as vocab_name";
  $taxo_order = "vocab_name ASC, termname ASC";
  $table = "{node} as n  inner join {biblio} as b on n.vid=b.vid ";
  $join = "left join {biblio_types} as t on b.biblio_type = t.tid";
  $taxo_join = array("inner join {term_node} as tn on n.vid = tn.vid",
                     "left join  {term_data} as td on tn.tid= td.tid",
                     "left join  {vocabulary} as v on v.vid= td.vid");

  $taxo_joins = implode(' ',$taxo_join);

  $result = db_query("SELECT $fields FROM $table $join ORDER BY $order");
  $authors = db_query("SELECT firstname, lastname, cid FROM {biblio_contributor_data} ORDER BY lastname ASC");
  $taxoresult = db_query("SELECT $taxo_fields FROM $table $taxo_joins ORDER BY $taxo_order");

  while ($option = db_fetch_object($result)) {
    if ($option->biblio_year == 9998) { $option->biblio_year = t("In Press"); }
    if ($option->biblio_year == 9999) { $option->biblio_year = t("Submitted"); }

    $pub_years["$option->biblio_year"] = $option->biblio_year;
    $pub_type["$option->tid"] = $option->name;

  }
  while($auth = db_fetch_object($authors)) {
    $pub_authors[$auth->cid] = $auth->lastname .', '. $auth->firstname;
  }
  while ($tax = db_fetch_object($taxoresult)) {
    $pub_taxo["$tax->taxid"] = "$tax->vocab_name - $tax->termname";
  }
  $author_select = isset($pub_authors) ? array('title' => t('Author'), 'options' => $pub_authors) : null;
  $years_select  = isset($pub_years) ? array('title' => t('Year'), 'options' => array_unique($pub_years)) : null;
  $type_select   = isset($pub_type)  ? array('title' => t('Type'), 'options' => array_unique($pub_type))  : null;
  $tax_select    = isset($pub_taxo)  ? array('title' => t('Term'),'options' => array_unique($pub_taxo))  : null;

  $filters = array(
    'aid'     => $author_select,
    'type'    => $type_select,
    'term_id' => $tax_select,
    'year'    => $years_select
  );

  return $filters;
}

/**
 * @return unknown_type
 */
function biblio_form_filter() {
  // No longer use &$_SESSION so that we can alter $session in case of the search filter.
  $session = $_SESSION['biblio_filter'];
  $session = is_array($session) ? $session : array();
  $filters = _get_biblio_filters();

  $i = 0;
  $form['filters'] = array('#type' => 'fieldset',
    '#title' => t('Show only items where'),
    '#theme' => 'biblio_filters',
  );
  foreach ($session as $filter) {
    list($type, $value) = $filter;
    // Don't show the search filter. Reset $session because of the $count(session) below.
    if ($type == 'search') {
      $session = array ();
      break;
    }
    if ($type == 'category') {
      // Load term name from DB rather than search and parse options array.
      $value = module_invoke('taxonomy', 'get_term', $value);
      $value = $value->name;
    }else if ($type == 'keyword') {
      $filters[$type]['title'] = 'Keyword';
    }else {
      $value = $filters[$type]['options'][$value];
    }
    $string = ($i++ ? '<em>and</em> where <strong>%a</strong> is <strong>%b</strong>' : '<strong>%a</strong> is <strong>%b</strong>');
    $form['filters']['current'][] = array('#value' => t($string, array('%a' => $filters[$type]['title'] , '%b' => $value)));
  }

  foreach ($filters as $key => $filter) {
    if (count ($filter['options'])) {
      $names[$key] = $filter['title'];
      $form['filters']['status'][$key] = array('#type' => 'select', '#options' => $filter['options']);
    }
  }
  $form['filters']['status']['keyword']=array('#type' => 'textfield','#size' => 25, '#maxlength' => 255);
  $names['keyword'] = "Keyword";

  $form['filters']['filter'] = array('#type' => 'radios', '#options' => $names, '#default_value' => 'aid');
  $form['filters']['buttons']['submit'] = array('#type' => 'submit', '#value' => (count($session) ? t('Refine') : t('Filter')));
  if (count($session) && $type != 'search') {
    $form['filters']['buttons']['undo'] = array('#type' => 'submit', '#value' => t('Undo'));
    $form['filters']['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  return  $form;
}

/**
 * @param $form
 * @param $form_state
 * @return unknown_type
 */
function biblio_form_filter_submit($form, &$form_state) {
  // If the search filter was set, remove it now.
  if(_get_biblio_search_filter()){
    $_SESSION['biblio_filter'] = array ();
  }
  $op = $form_state['values']['op'];
  $filters = _get_biblio_filters();
  switch ($op) {
    case t('Filter'):
    case t('Refine'):
      if (isset($form_state['values']['filter'])) {
        $filter = $form_state['values']['filter'];

        // Flatten the options array to accommodate hierarchical/nested options.
        if (isset($filters[$filter]['options'])) {
          $flat_options = form_options_flatten($filters[$filter]['options']);
        }

        if (isset($flat_options[$form_state['values'][$filter]]) || $filter == 'keyword') {
          $_SESSION['biblio_filter'][] = array($filter, $form_state['values'][$filter]);
          $base =  variable_get('biblio_base', 'biblio');
          drupal_goto($base);

        }
      }
      break;
    case t('Undo'):
      array_pop($_SESSION['biblio_filter']);
      break;
    case t('Reset'):
      $_SESSION['biblio_filter'] = array();
      break;
  }
}

/**
 * @param $user
 * @param $profile
 * @return unknown_type
 */
function biblio_get_user_pubs($user,$profile=''){
  switch ((isset($user->biblio_profile_uid) ) ? $user->biblio_profile_uid : variable_get('biblio_profile_uid', 'uid')){
    case 'lastname':
      $pubs = biblio_db_search('author', $user->biblio_lastname, $profile);
      break;
    case 'uid_lastname':
      $pubs = biblio_db_search('uid', $user->uid, 'author', $user->biblio_lastname, $profile);
      break;
    case 'uid':
    default:
      $pubs = biblio_db_search('uid', $user->uid, $profile);
  }
  return $pubs;

}

/**
 * @param $node
 * @return unknown_type
 */
function biblio_view_inline(&$node) {
  $style = variable_get('biblio_style', 'classic');
  $base =  variable_get('biblio_base', 'biblio');

  $output  = '<div class="biblio-inline">';
  $output .= theme('biblio_long', $node, $base, $style);
  $output .= '</div>';
  return $output;
}

/**
 * @return unknown_type
 */
function biblio_citekey_view() {
  $citekey = arg(2);
  $nid = db_fetch_object(db_query("SELECT nid FROM {biblio} WHERE biblio_citekey = '%s' ORDER BY vid DESC", $citekey));
  if ($nid->nid > 0) {
    $node = node_load($nid->nid);
    return node_page_view($node);
  } else {
    return t("Sorry, citekey @cite not found", array('@cite'=>$citekey));
  }

}


/**
 * @param $author
 * @param $aid
 * @param $base
 * @param $inline
 * @return unknown_type
 */
function _biblio_author_link($author, $aid, $base='biblio', $inline = FALSE) {
  $options = array();
  $inline = $inline ? "/inline" : "";

  if (isset($_GET['sort'])) {
    $options['query']  .= "sort=" . $_GET['sort'];
  }
  if (isset($_GET['order'])) {
    $options['query']   .= $options['query'] ? "&" : "" ;
    $options['query']   .= "order=" . $_GET['order'];
  }

  $html = l(trim($author), "$base/aid/$aid".$inline,$options );

  return $html;
}

/**
 * @param $keywords
 * @param $base
 * @return unknown_type
 */
function _biblio_keyword_links($keywords,$base='biblio') {
  $options = array();
  if (isset($_GET['sort'])) {
    $options['query']  .= "sort=" . $_GET['sort'];
  }
  if (isset($_GET['order'])) {
    $options['query']  .= $options['query'] ? "&" : "";
    $options['query']  .= "order=" . $_GET['order'];
  }
  $html = "";
  $sep = variable_get('biblio_keyword_sep', ',');
  $keyword_array = explode($sep, $keywords);

  foreach($keyword_array as $kw) {
    $html .= (empty($html))? "" : "; ";
    $html .= l(trim($kw), "$base/keyword/" . trim($kw) , $options);
  }
  return $html;
}





