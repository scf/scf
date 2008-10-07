<?php
// $Id: views-ui-edit-item.tpl.php,v 1.7 2008/05/09 19:32:12 merlinofchaos Exp $
/**
 * @file views-ui-edit-item.tpl.php
 *
 * This template handles the printing of fields/filters/sort criteria/arguments or relationships.
 */
?>
<?php print $rearrange; ?>
<?php print $add; ?>
<div class="views-category-title<?php if ($overridden) { print ' overridden'; }?>">
  <?php print $item_help_icon; ?>
  <?php print $title; ?>
</div>

<div class="views-category-content">
  <?php if (empty($fields)): ?>
    <div><?php print t('None defined'); ?></div>
  <?php else: ?>
    <?php foreach ($fields as $pid => $field): ?>
      <?php if (!empty($field['links'])): ?>
        <?php print $field['links']; ?>
      <?php endif; ?>
      <div class="<?php print $field['class']; if (!empty($field['changed'])) { print ' changed'; } ?>">
        <?php print $field['title']; ?>
        <?php print $field['info']; ?>
      </div>
    <?php endforeach; ?>
  <?php endif; ?>
</div>
