<?php
// $Id: member-embed.tpl.php,v 1.3 2007/08/28 11:35:33 goba Exp $

/**
 * @file member-embed.tpl.php
 *
 * Theme implementation to display a member in an embedded context (e.g. in an interview).
 * Has all node variables (see node.tpl.php) available, and also the following:
 *
 * - $title: 
 * - $contribs:
 */
?>
<div class="member-contributions">
<h3><?php print $title; ?></h3>
<ul>
<?php foreach ($contribs as $contrib) { ?>
  <li><?php print $contrib; ?></li>
<?php } ?>
</ul>
</div>
