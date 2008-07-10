<?php
// $Id: member-picture.tpl.php,v 1.2 2007/08/07 08:39:36 goba Exp $

/**
 * @file member-picture.tpl.php
 * Default theme implementation to present an picture configured for the
 * member.
 *
 * Available variables:
 * - $picture: Image set by the member or the site's default. Will be linked
 *   depending on the viewer's permission to view the members profile page.
 * - $member: The member node.
 *
 * @see template_preprocess_member_picture()
 */
?>
<div class="member-picture">
  <?php print $picture; ?>
</div>
