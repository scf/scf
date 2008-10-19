<?php
/**
 * Add to the general preprocessing for this theme
 */
function phptemplate_preprocess(&$variables, $hook) {
  
  switch ($hook) {
  
    case 'page':
      
      // Provide invoking URL for page templates
      // - could do in scf_base_preprocess_page - but you seen the size of that sucker!4
      $variables['url_path'] = $_GET['q'];
      $variables['url_path_token'] = preg_replace('/\//', '-', $_GET['q']);
      $variables['url_path_alias'] = drupal_get_path_alias($_GET['q']);
      $variables['url_path_alias_token'] = preg_replace('/\//', '-',$variables['url_path_alias']);
      
      // Replace out the Reaserch statement title 
      // @todo This is better placed in the researchstatement.module
      if (isset($variables['node']) && $variables['node']->type) {
        $node = $variables['node'];
        if ($node->type == 'researchstatement') {
          // Could drupal_set_title too but want search engines to identify the statements
          $variables['title'] = 'Research statement';
        } else if (isset($node->display_title)) {
          $variables['display_title'] = $node->display_title;
          //if ($node->type == 'pubnode') {
	  // $variables['display_title'] .= '<a href="#editedinfo">*</a>';
          //}
        }
      }
      
      // Rather than use Conditional Comments, we use this...
      $user_agent = ( isset( $_SERVER['HTTP_USER_AGENT'] ) ) ? strtolower( $_SERVER['HTTP_USER_AGENT'] ) : '';
      switch (true) {
        case strchr($user_agent, 'msie 5.0'):
        case strchr($user_agent, 'msie 5.5'):
        case strchr($user_agent, 'msie 6.0'):
          $ie_style = path_to_theme() .'/css/ie.css';
          $ie_js = path_to_theme() .'/jscript/ie.js';
          if (file_exists($ie_style)) {
            drupal_add_css($ie_style, 'theme', 'all', FALSE);
            $variables['styles'] = drupal_get_css();
          }
          if (file_exists($ie_js)) {
            drupal_add_js($ie_js, 'theme');
            $variables['scripts'] = drupal_get_js();
          }
      }
      break;
  }
}

/**
 * Add to the variables for node.tpl.php
 *
 * Most themes utilize their own copy of node.tpl.php. The default is located
 * inside "modules/node/node.tpl.php". Look in there for the full list of
 * variables.
 *
 * The $variables array contains the following arguments:
 * - $node
 * - $teaser
 * - $page
 *
 * @see node.tpl.php
 */
function phptemplate_preprocess_node(&$variables) {
  $links = array();
  
  $node = $variables['node'];
  $type = $node->type;
  $nid = $node->nid;
  $sticky = $node->sticky;
  $promoted = $node->promote;
  $published = $node->status;
  
  $edit_perm = node_access('update', $node);
  $add_perm = node_access('create', $node);
  $del_perm = node_access('delete', $node);
  $admin_perm = user_access('administer nodes');
  
  if ($edit_perm) {
    $links['node_edit'] = array(
      'title' => t('Edit'), 
      'href' => 'node/' . $nid . '/edit', 
      'attributes' => array('title' => t('Edit this !type', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($add_perm) {
    $links['node_add'] = array(
      'title' => t('Add'), 
      'href' => "node/add/$type", 
      'attributes' => array('title' => t('Add a new !type', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($del_perm) {
    $links['node_del'] = array(
      'title' => t('Delete'), 
      'href' => 'node/' . $nid . '/delete', 
      'attributes' => array('title' => t('Delete this !type', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($admin_perm && $published) {
    $links['node_unstatus'] = array(
      'title' => t('Un-publish'), 
      'href' => 'node/' . $nid . '/publish/off', 
      'attributes' => array('title' => t('Un-publish this !type', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($admin_perm && !$promoted) {
    $links['node_promote'] = array(
      'title' => t('Promote'), 
      'href' => 'node/' . $nid . '/promote/on', 
      'attributes' => array('title' => t('Promote this !type to the front page', array('!type' => $type))), 
      'query' => 'destination=' . variable_get('site_frontpage','node'));
  }
  if ($admin_perm && $promoted) {
    $links['node_unpromote'] = array(
      'title' => t('Demote'), 
      'href' => 'node/' . $nid . '/promote/off', 
      'attributes' => array('title' => t('Demote this !type from the front page', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($admin_perm && !$sticky) {
    $links['node_sticky'] = array(
      'title' => t('Feature'), 
      'href' => 'node/' . $nid . '/feature/on', 
      'attributes' => array('title' => t('Feature this !type at the top of this list', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($admin_perm && $sticky) {
    $links['node_unsticky'] = array(
      'title' => t('De-feature'), 
      'href' => 'node/' . $nid . '/feature/off', 
      'attributes' => array('title' => t('Return this !type to normal list sort order', array('!type' => $type))), 
      'query' => drupal_get_destination());
  }
  if ($admin_perm && !$sticky && !$promoted) {
    $links['node_promotefeature'] = array(
      'title' => t('Feature front'), 
      'href' => 'node/' . $nid . '/promotefeature', 
      'attributes' => array('title' => t('Feature this !type on the front page', array('!type' => $type))), 
      'query' => 'destination=' . variable_get('site_frontpage','node'));
  }

  // @todo Be nice to hook this into a view of unpublished nodes of type X
  //if ($admin_perm) {
  //  $links['node_status'] = array(
  //    'title' => t('Un-published !type content', array('!type' => $type)), 
  //    'href' => 'admin/content/node', 
  //    'query' => "status=0&type=$type");
  //}
  
  $variables['admin_links'] = theme('links', $links, array('class' => 'action_links links inline'));
}

/**
 * Add to variables for block.tpl.php
 *
 * Prepare the values passed to the theme_block function to be passed
 * into a pluggable template engine. Uses block properties to generate a
 * series of template file suggestions. If none are found, the default
 * block.tpl.php is used.
 *
 * Most themes utilize their own copy of block.tpl.php. The default is located
 * inside "modules/system/block.tpl.php". Look in there for the full list of
 * variables.
 *
 * The $variables array contains the following arguments:
 * - $block
 *
 * @see block.tpl.php
 */
function phptemplate_preprocess_block(&$variables) {
  $links = array();
  
  if (user_access('administer blocks')) {
    $links['block_edit'] = array(
      'title' => t('Edit'), 
      'href' => 'admin/build/block/configure/' . $variables['block']->module .'/'. $variables['block']->delta, 
      'attributes' => array('title' => t('Edit this block')), 
      'query' => drupal_get_destination());
  }
  
  $variables['admin_links'] = theme('links', $links, array('class' => 'action_links links inline'));
  
}

/**
 * Add to the variables for forums.tpl.php
 *
 * The $variables array contains the following arguments:
 * - $forums
 * - $topics
 * - $parents
 * - $tid
 * - $sortby
 * - $forum_per_page
 *
 * @see forums.tpl.php
 */
function phptemplate_preprocess_forums(&$variables) {

  // Clean out some SCF types to simplify the user interface
  foreach ($variables['links'] as $type => $link) {
    switch ($type) {
      case 'antibody' :
      case 'interview' :
      case 'modelorganism' :
      case 'newsarticle' :
      case 'pubnode' :
      case 'pubgroup' :
        unset($variables['links'][$type]);
    }
  }

  if (user_access('administer forums')) {
    $variables['links']['forum_addcontainer'] = array(
      'title' => t('Add container'), 
      'href' => 'admin/content/forum/add/container', 
      'attributes' => array('title' => t('Add a new container')), 
      'query' => drupal_get_destination());
    $variables['links']['forum_addforum'] = array(
      'title' => t('Add forum'), 
      'href' => 'admin/content/forum/add/forum', 
      'attributes' => array('title' => t('Add a new forum')), 
      'query' => drupal_get_destination());
  }
  // Attach these admin links to the existing links variable
  $variables['links'] = theme('links', $variables['links'], array('class' => 'links inline'));
}

/**
 * Add to the variables to format a forum listing.
 *
 * $variables contains the following information:
 * - $forums
 * - $parents
 * - $tid
 *
 * @see forum-list.tpl.php
 * @see theme_forum_list()
 */
function phptemplate_preprocess_forum_list(&$variables) {
  foreach ($variables['forums'] as $id => $forum) {
    $links = array();
    $type = $variables['forums'][$id]->is_container ? 'container' : 'forum';
    if (user_access('administer forums')) {
      $links['forum_edit'] = array(
        'title' => t('Edit'), 
        'href' => "admin/content/forum/edit/$type/$id", 
        'attributes' => array('title' => t('Edit this !type', array('!type' => $type))), 
        'query' => drupal_get_destination());
    }
    $variables['forums'][$id]->admin_links = theme('links', $links, array('class' => 'action_links links inline'));
  }
}

/**
 * Preprocess variables to format the topic listing.
 *
 * $variables contains the following data:
 * - $tid
 * - $topics
 * - $sortby
 * - $forum_per_page
 *
 * @see forum-topic-list.tpl.php
 * @see theme_forum_topic_list()
 */
function phptemplate_preprocess_forum_topic_list(&$variables) {
  foreach ($variables['topics'] as $id => $topic) {
    $links = array();
    if (user_access('administer forums')) {
      $links['forum_edit'] = array(
        'title' => t('Edit'), 
        'href' => 'node/' . $topic->nid . '/edit', 
        'attributes' => array('title' => t('Edit this topic')), 
        'query' => drupal_get_destination());
    }
    $variables['topics'][$id]->admin_links = theme('links', $links, array('class' => 'action_links links inline'));
  }
}

/**
 * Process variables to format submission info for display in the forum list and topic list.
 *
 * $variables will contain: $topic
 *
 * @see forum-submitted.tpl.php
 * @see theme_forum_submitted()
 */
function phptemplate_preprocess_forum_submitted(&$variables) {
  $variables['author'] = '';
  if (isset($variables['topic']->uid)) {
    if ($author = _member_get_node($variables['topic']->uid)) {
      $variables['author'] = theme('member_link', $author);
    }
    else {
      $variables['author'] = theme('username', $variables['topic']);
    }
  }
  $variables['time'] = isset($variables['topic']->timestamp) ? format_interval(time() - $variables['topic']->timestamp) : '';
}

/**
 * Preprocess variables to format the next/previous forum topic navigation links.
 *
 * $variables contains $node.
 *
 * @see forum-topic-navigation.tpl.php
 * @see theme_forum_topic_navigation()
 */
function phptemplate_preprocess_forum_topic_navigation(&$variables) {
  $forum = taxonomy_get_term($variables['node']->tid);
  $variables['forum'] = $variables['node']->tid;
  $variables['forum_title'] = phptemplate_truncate_by_word_count($forum->name, 12);
  $variables['forum_url'] = url('forum/' . $variables['node']->tid);
  
  if (isset($variables['prev_title'])) {
    $variables['prev_title'] = phptemplate_truncate_by_word_count($variables['prev_title'], 12);
  }
  if (isset($variables['next_title'])) {
    $variables['next_title'] = phptemplate_truncate_by_word_count($variables['next_title'], 12);
  }
}

/**
 * Override the logintoboggan block
 */
function phptemplate_lt_loggedinblock() {
  global $user;

  // Tom to provide member_user hook to attache this infor to the user object
  $usernode = _member_get_node($user);
  $content = 'Logged in as <span class="fn">' . ($usernode ? check_plain($usernode->firstname) : $user->name) . '</span> ';
  $content .= '<span class="link">' . l(t('Profile'), ($usernode ? 'node/' . check_plain($usernode->nid) : 'user/' . $user->uid), array('title' => 'View this use profile')) . '</span> ';
  $content .= '<span class="link">' . l(t('Log out'), 'logout') . '</span>';
  
  return $content;
}

/**
 *  Override the login link block text.
 */
function phptemplate_lt_login_link() {
  return t('Log in or Register');
}

/**
 *  Override the 'more' link rendering
 */
function phptemplate_more_link($url, $title) {
  return '<div class="more-link">'. t('<a href="@link" title="@title">Read more</a>', array('@link' => check_url($url), '@title' => $title)) .'</div>';
}


/**
 * Override the message rendering
 */
function phptemplate_status_messages($display = NULL) {
  $output = '';
  foreach (drupal_get_messages($display) as $type => $messages) {
    $output .= "<div class=\"messages $type\">\n";
    $output .= "<h3>$type</h3>";
    if (count($messages) > 1) {
      $output .= " <ul>\n";
      foreach ($messages as $message) {
        $output .= '  <li>'. $message ."</li>\n";
      }
      $output .= " </ul>\n";
    }
    else {
      $output .= $messages[0];
    }
    $output .= "</div>\n";
  }
  return $output;
}

/**
 * Override a formatted list of recent comments to be displayed in the comment block.
 */
function phptemplate_comment_block() {
  $items = array();
  foreach (phptemplate_comment_get_recent() as $comment) {
    $link = l($comment->subject, 'node/'. $comment->nid, array('fragment' => 'comment-'. $comment->cid));
    $date = t('@time ago', array('@time' => format_interval(time() - $comment->timestamp)));
    if ($author = _member_get_node($comment->uid)) {
      $capacity = ($author->jobtitle === '' ? '' : $author->jobtitle . ', ') . ($author->affiliation === '' ? '' : $author->affiliation);
      $author = theme('member_link', $author);
    }
    else {
      $author = theme_username(user_load(array('uid' => $comment->uid)));
      $capacity = '';
    }
    $content = '<div class="title clear-block">' . $link . ' <span class="date">' . $date . '</span></div>';
    $content .= '<div class="comment clear-block">' . phptemplate_truncate_by_word_count($comment->comment, 30) . '</div>';
    $content .= '<div class="byline vcard clear-block">By <span class="fn">' . $author . '</span>';
    $content .= ($capacity === '' ? '' : ', <span class="title">' . $capacity . '</span>') . '</div>';
    $items[] = $content;

  }
  if ($items) {
    return theme('item_list', $items);
  }
}

/**
 * Override of comment_get_recent.
 * 
 * Find a number of recent comments. This is done in two steps.
 *   1. Find the n (specified by $number) nodes that have the most recent
 *      comments.  This is done by querying node_comment_statistics which has
 *      an index on last_comment_timestamp, and is thus a fast query.
 *   2. Loading the information from the comments table based on the nids found
 *      in step 1.
 *
 * @param $number
 *   (optional) The maximum number of comments to find.
 * @return
 *   An array of comment objects each containing a nid,
 *   subject, cid, and timestamp, or an empty array if there are no recent
 *   comments visible to the current user.
 * 
 * @todo Should this be a preprocess function?
 */
function phptemplate_comment_get_recent($number = 10) {
  // Select the $number nodes (visible to the current user) with the most
  // recent comments. This is efficient due to the index on
  // last_comment_timestamp.
  $result = db_query_range(db_rewrite_sql("SELECT nc.nid FROM {node_comment_statistics} nc WHERE nc.comment_count > 0 ORDER BY nc.last_comment_timestamp DESC", 'nc'), 0, $number);

  $nids = array();
  while ($row = db_fetch_object($result)) {
    $nids[] = $row->nid;
  }

  $comments = array();
  if (!empty($nids)) {
    // From among the comments on the nodes selected in the first query,
    // find the $number most recent comments.
    $result = db_query_range('SELECT c.nid, c.uid, c.subject, c.comment, c.cid, c.timestamp FROM {comments} c INNER JOIN {node} n ON n.nid = c.nid WHERE c.nid IN ('. implode(',', $nids) .') AND n.status = 1 AND c.status = %d ORDER BY c.cid DESC', COMMENT_PUBLISHED, 0, $number);
    while ($comment = db_fetch_object($result)) {
      $comments[] = $comment;
    }
  }
  return $comments;
}
