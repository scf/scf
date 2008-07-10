<?php
// $Id: contributor-embed.tpl.php,v 1.3 2007/08/28 11:35:33 goba Exp $

/**
 * @file contributor-embed.tpl.php
 *
 * Theme implementation to display a contributor in an embedded context (e.g. in an interview).
 * Has all node variables (see node.tpl.php) available, and also the following:
 *
 * - $capacity: 
 * - $capacity_dir_url: url of directory listing by position
 * - $affiliation:
 * - $affiliation_link: url of directory listing by affiliation
 */
?>
<div id="contributor-<?php print $node->nid; ?>" class="node contributor-embed contributor clear-block">

  <?php print $picture; ?>

  <div class="contributor-title">
    <span class="contributor-name"><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></span>
<?php if ($capacity): ?>
    <span class="contributor-capacity"><?php print $capacity; ?></span><?php if ($affiliation): ?>, <?php endif; ?>
<?php endif; ?>
<?php if ($affiliation_link): ?>
    <span class="contributor-affiliation"><?php print $affiliation_link ?></span>
<?php endif;?>
  </div>

  <div class="contributor-content">
    <?php print $content ?>
  </div>

  <?php if ($terms): ?>
    <div class="contributor-research-interests">
      <b>Research interests:</b> <span class="terms"><?php print $terms ?></span>
    </div>
  <?php endif;?>

  <?php if (isset($edit_link)): ?>
  <div class="contributor-edit">[<?php print $edit_link ?>]</div>
  <?php endif;?>
</div>