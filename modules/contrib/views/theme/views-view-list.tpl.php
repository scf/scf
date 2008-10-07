<?php
// $Id: views-view-list.tpl.php,v 1.1 2008/02/15 04:11:48 merlinofchaos Exp $
/**
 * @file views-view-list.tpl.php
 * Default simple view template to display a list of rows.
 *
 * - $options['type'] will either be ul or ol.
 * @ingroup views_templates
 */
?>
<div class="item-list">
  <<?php print $options['type']; ?>>
    <?php foreach ($rows as $row): ?>
      <li><?php print $row ?></li>
    <?php endforeach; ?>
  </<?php print $options['type']; ?>>
</div>