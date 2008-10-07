<?php
// $Id: comment.tpl.php,v 1.4 2008/01/04 19:24:23 goba Exp $

/**
 * @file comment.tpl.php
 * Default theme implementation for comments.
 *
 * Available variables:
 * - $author: Comment author. Can be link or plain text.
 * - $content: Body of the post.
 * - $date: Date and time of posting.
 * - $links: Various operational links.
 * - $new: New comment marker.
 * - $picture: Authors picture.
 * - $signature: Authors signature.
 * - $status: Comment status. Possible values are:
 *   comment-unpublished, comment-published or comment-review.
 * - $submitted: By line with date and time.
 * - $title: Linked title.
 *
 * These two variables are provided for context.
 * - $comment: Full comment object.
 * - $node: Node object the comments are attached to.
 *
 * @see template_preprocess_comment()
 * @see theme_comment()
 */
$date = t('@time ago', array('@time' => format_interval(time() - $comment->timestamp)));
if ($author = _member_get_node($comment->uid)) {
  $capacity = ($author->jobtitle === '' ? '' : $author->jobtitle . ', ') . ($author->affiliation === '' ? '' : $author->affiliation);
  $author = theme('member_link', $author);
}
else {
  $author = theme_username(user_load(array('uid' => $comment->uid)));
  $capacity = '';
}
?>
<div class="comment<?php print ($comment->new) ? ' comment-new' : ''; print ' '. $status ?> clear-block">
  <h3 class="clear-block">
    <?php print $title ?>
      <span class="date"> Added <?php print $date ?></span>
<?php if ($new): ?>
      <span class="new">New comment</span>
<?php endif; ?>
  </h3>
  <div class="content clear-block"><?php print $content ?></div>
  <div class="byline vcard clear-block">By 
    <span class="fn"><?php print $author ?></span>
    <?php print ($capacity === '' ? '' : ', <span class="title">' . $capacity . '</span>') ?>
    <?php print $links ?>
  </div>
</div>