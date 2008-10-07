<?php
// $Id: node.tpl.php,v 1.4 2008/01/25 21:21:44 goba Exp $

/**
 * @file node.tpl.php
 *
 * Theme implementation to display a node.
 *
 * Available variables:
 * - $title: the (sanitized) title of the node.
 * - $content: Node body or teaser depending on $teaser flag.
 * - $picture: The authors picture of the node output from
 *   theme_user_picture().
 * - $date: Formatted creation date (use $created to reformat with
 *   format_date()).
 * - $links: Themed links like "Read more", "Add new comment", etc. output
 *   from theme_links().
 * - $name: Themed username of node author output from theme_user().
 * - $node_url: Direct url of the current node.
 * - $terms: the themed list of taxonomy term links output from theme_links().
 * - $submitted: themed submission information output from
 *   theme_node_submitted().
 * - $admin_links: Themed links like "Edit", "Add", etc. output from
 *   theme_links().
 *
 * Other variables:
 * - $node: Full node object. Contains data that may not be safe.
 * - $type: Node type, i.e. story, page, blog, etc.
 * - $comment_count: Number of comments attached to the node.
 * - $uid: User ID of the node author.
 * - $created: Time the node was published formatted in Unix timestamp.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the node. Increments each time it's output.
 *
 * Node status variables:
 * - $teaser: Flag for the teaser state.
 * - $page: Flag for the full page state.
 * - $promote: Flag for front page promotion state.
 * - $sticky: Flags for sticky post setting.
 * - $status: Flag for published status.
 * - $comment: State of comment settings for the node.
 * - $readmore: Flags true if the teaser content of the node cannot hold the
 *   main body content.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 */
?>
<div id="node-<?php print $node->nid; ?>_wrapper" class="node<?php print ' ' . $type; ?><?php print ($teaser ? ' teaser' : ' not-teaser') ?><?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?>">
  <div id="node-<?php print $node->nid; ?>" class="clear-block">
<?php print $picture ?>

<?php if (!$page): ?>
    <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
<?php endif; ?>
  
    <div class="content clear-block">
<?php if ($teaser): ?>
      <?php // @todo Fake the image until we have specific field for a main pubnode image ?>
  <?php if (preg_match('/.*<img .*/',$content)): ?>
      <?php print node_teaser($content, NULL, 240); ?>
  <?php else: ?>
      <img src="<?php print base_path() . path_to_theme() ?>/images/dummy-100x100.png" />
      <?php print node_teaser($node->content['body']['#value'], NULL, 240); ?>
  <?php endif; ?>
<?php else: ?>
      <?php print $content; ?>
<?php endif; ?>
    </div>

    <div class="clear-block"><?php print $admin_links; ?></div>
    <div class="clear-block"><?php print $links; ?></div>
  </div>
</div>
