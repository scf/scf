<?php
// $Id: $

/**
 * @file interview-byline.tpl.php
 *
 * Theme implementation to display the byline of an interview
 *
 * - $node: 
 * - $author_link: 
 * - $author_capacity: 
 */
?>
<?php if ($author_link): ?>
  <?php if (isset($author_edit_link)): ?>
  <div class="contributor-edit">[<?php print $author_edit_link ?>]</div>
  <?php endif;?>
  <div class="interview-byline">By <?php print $author_link ?>, <?php print $author_capacity ?></div>
<?php endif;?>
