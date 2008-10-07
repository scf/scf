<?php
// $Id: views-view-unformatted.tpl.php,v 1.2 2008/02/15 04:11:48 merlinofchaos Exp $
/**
 * @file views-view-unformatted.tpl.php
 * Default simple view template to display a list of rows.
 *
 * @ingroup views_templates
 */
?>
<?php foreach ($rows as $row): ?>
  <?php print $row ?>
<?php endforeach; ?>