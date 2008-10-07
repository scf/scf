<?php
// $Id: views-view-rss.tpl.php,v 1.1 2008/04/12 02:47:18 merlinofchaos Exp $
/**
 * @file views-view-rss.tpl.php
 * Default simple view template to display a list of rows.
 *
 * @ingroup views_templates
 */
?>
<?php print "<?xml"; ?> version="1.0" encoding="utf-8" <?php print "?>"; ?>
<rss version="2.0" xml:base="<?php print $link; ?>"<?php print $namespaces; ?>>
  <?php print $channel; ?>
</rss>