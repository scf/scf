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
 * - $admin_links: List of links for administration actions
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 */
?>
<div id="node-<?php print $node->nid; ?>_wrapper" class="node<?php print ' ' . $type; ?><?php print ($teaser ? ' teaser' : ' not-teaser') ?><?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?> <?php print $zebra ?> clear-block">
  <div id="node-<?php print $node->nid; ?>">

<?php if (!$page): ?>
    <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
<?php endif; ?>

    <div class="content">
      <?php //print $content ?>
<?php if (isset($node->content['picture'])): ?>
      <?php print $node->content['picture']['#value'] ?>
<?php else: ?>
      <div class="member-picture"><img src="<?php print base_path() . path_to_theme() ?>/images/dummy-100x100.png" /></div>
<?php endif; ?>
<?php if (isset($node->content['jobtitle']) or isset($node->content['affiliation'])): ?>
      <div class="member-capacity">
  <?php if (isset($node->content['jobtitle'])): ?>
        <span class="member-jobtitle title"><?php print $node->content['jobtitle']['#value'] ?></span>
  <?php endif; ?>
  <?php if (isset($node->content['affiliation'])): ?>
        <span class="member-affiliation org"><?php print $node->content['affiliation']['#value'] ?></span>
  <?php endif; ?>
      </div>
<?php endif; ?>
<?php if (isset($node->content['contact'])): ?>
      <?php print $node->content['contact']['#value'] ?>
<?php endif; ?>
<?php if ($node->content['body']): ?>
      <div class="member-bio"><?php print $node->content['body']['#value'] ?></div>
<?php endif; ?>
<?php if (isset($node->content['account'])): ?>
      <div class="member-account-link"><?php print $node->content['account']['#value']; ?></div>
<?php endif; ?>
    </div>

<?php // @todo Tom still figuring this out... ?>
<?php if ($terms && $page): ?>
	<div id="member-research-areas" class="clear-block">
	  <h3>Research areas</h3>
    <?php print $terms; ?>
  </div>
<?php endif;?>

<?php if (isset($node->content['contributions'])): ?>
  <?php print $node->content['contributions']['#value'] ?>
<?php endif;?>

<?php if ($teaser): ?>
    <div class="clear-block"><?php print $admin_links; ?></div>
<?php endif; ?>

    <div class="clear-block"><?php print $links; ?></div>
  </div>
</div>
