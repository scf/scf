<?php
// $Id: member-embed.tpl.php,v 1.3 2007/08/28 11:35:33 goba Exp $

/**
 * @file member-embed.tpl.php
 *
 * Theme implementation to display a member in an embedded context (e.g. in an interview).
 * Has all node variables (see node.tpl.php) available, and also the following:
 *
 * - $jobtitle: 
 * - $jobtitle_dir_url: url of directory listing by position
 * - $affiliation:
 * - $affiliation_dir_url: url of directory listing by affiliation
 */
?>
<div id="member-<?php print $node->nid; ?>" class="node member-embed member clear-block">

  <?php print $picture ?>

  <div class="member-title">
    <span class="member-name"><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></span>
<?php if ($jobtitle): ?>
    <span class="member-jobtitle"><a href="<?php print $jobtitle_dir_url ?>"><?php print $jobtitle ?></a></span><?php if ($affiliation): ?>, <?php endif;?>
<?php endif;?>
<?php if ($affiliation): ?>
    <span class="member-affiliation"><a href="<?php print $affiliation_dir_url ?>"><?php print $affiliation ?></a></span>
<?php endif;?>
  </div>

  <div class="member-content">
    <?php print $content ?>
  </div>

  <?php if ($terms): ?>
    <div class="member-research-interests">
      <b>Research interests:</b> <span class="terms"><?php print $terms ?></span>
    </div>
  <?php endif;?>

</div>