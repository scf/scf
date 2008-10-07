<?php
// $Id: $

/**
 * @file newsarticle-byline.tpl.php
 *
 * Theme implementation to display the byline of an newsarticle
 *
 * - $node: 
 * - $author_name: 
 * - $media_source: 
 * - $pubdate: 
 */
?>
<div class="byline newsarticle">
  <?php if (!empty($author_name)): ?><span class="name"><?php print $author_name; ?></span>,<?php endif;?>
  <span class="source"><?php print $media_source ?> </span>
  <span class="date"><?php print $pubdate ?></span>
</div>