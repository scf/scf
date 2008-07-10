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
<div class="newsarticle-byline">
  <div><?php if (!empty($author_name)): ?><?php print $author_name; ?>, <?php endif;?><?php print $media_source; ?></div>
  <div><?php print $pubdate; ?></div>
</div>
