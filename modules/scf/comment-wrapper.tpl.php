<?php
// $Id: comment-wrapper.tpl.php,v 1.2 2007/08/07 08:39:35 goba Exp $

/**
 * @file comment-wrapper.tpl.php
 * Default theme implementation to wrap comments.
 *
 * Available variables:
 * - $content: All comments for a given page. Also contains sorting controls
 *   and comment forms if the site is configured for it.
 *
 * The following variables are provided for contextual information.
 * - $node: Node object the comments are attached to.
 * The constants below the variables show the possible values and should be
 * used for comparison.
 * - $display_mode
 *   - COMMENT_MODE_FLAT_COLLAPSED
 *   - COMMENT_MODE_FLAT_EXPANDED
 *   - COMMENT_MODE_THREADED_COLLAPSED
 *   - COMMENT_MODE_THREADED_EXPANDED
 * - $display_order
 *   - COMMENT_ORDER_NEWEST_FIRST
 *   - COMMENT_ORDER_OLDEST_FIRST
 * - $comment_controls_state
 *   - COMMENT_CONTROLS_ABOVE
 *   - COMMENT_CONTROLS_BELOW
 *   - COMMENT_CONTROLS_ABOVE_BELOW
 *   - COMMENT_CONTROLS_HIDDEN
 *
 * @see template_preprocess_comment_wrapper()
 * @see theme_comment_wrapper()
 */
$title = t('Recent discussions about this ');
switch($node->type) {
  case 'newsarticle':
    $title .= t('news article');
    break;
  case 'pubnode':
    $title .= t('publication');
    break;
  case 'pubgroup':
    $title .= t('publication group');
    break;
  case 'interview':
    $title .= t('interview');
    break;
  default:
    $title = t('Comments');
}
?>
<div id="comments">
<?php if ($content && $node->type != 'forum'): ?>
  <h2 class="comments"><?php print $title; ?></h2>
<?php endif; ?>
  <?php print $content; ?>
</div>
