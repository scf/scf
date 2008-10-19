<?php

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
    $content .= '<div class="comment clear-block">' . scf_au_truncate_by_word_count($comment->comment, 30) . '</div>';
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
