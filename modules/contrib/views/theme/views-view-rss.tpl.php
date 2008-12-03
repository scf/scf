<?php
// $Id: views-view-rss.tpl.php,v 1.2 2008/07/21 20:56:49 merlinofchaos Exp $
/**
 * @file views-view-rss.tpl.php
 * Default template for feed displays that use the RSS style.
 *
 * @ingroup views_templates
 */
?>
<?php print "<?xml"; ?> version="1.0" encoding="utf-8" <?php print "?>"; ?>
<rss version="2.0" xml:base="<?php print $link; ?>"<?php print $namespaces; ?>>
  <?php print $channel; ?>
</rss>