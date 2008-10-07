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

$link = '';
if (preg_match('/.*href.*/',$author_link)) {
  $link = preg_replace('/^.*href="(.+?)".*$/','$1',$author_link);
  $link = preg_replace('/^\/(.+)$/','$1',$link);
}
?>
<?php if ($author_link) : ?>
  <div class="byline vcard clear">
    <?php if (isset($author_edit_link)): ?>
      <div class="contributor-edit">[<?php print $author_edit_link ?>]</div>
    <?php endif;?>
    <span>By </span>
    <span class="fn"><?php print $author_link ?>, </span>
    <span class="title"><?php print $author_capacity ?></span>
    <?php if ($link != '') : ?>
      <?php print l('<span>'.t('View member page.').'</span>', $link, array('attributes' => array('class' => 'link'), 'html' => 1)); ?>
    <?php endif; ?>
  </div>
<?php endif;?>