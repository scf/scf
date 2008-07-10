<?php
// $Id: $

/**
 * @file related-terms-block-list.tpl.php
 *
 * - $title: title of list
 * - $terms: array of term links
 * - $addlink link to add new terms or ''
 */

if (!empty($terms) || $has_addlink) {
?>
  <div class="resource-list clear-block">
    <h3>
      <?php print $title ?>
    </h3>
    <ul id="taxonomy-extra-termlist-<?php print $vid; ?>">
      <?php foreach ($terms as $term) { ?>
        <li><?php print $term; ?></li>
      <?php } ?>
    </ul>
    <?php /* div added by MAG on 5/27/2008 */ if ($has_addlink) { ?>
    <div>
	    <a href="#" onclick="$('#taxonomy-extra-addtermdiv-<?=$vid ?>').slideToggle('slow');return false;">Add new term</a>
	    <div id="taxonomy-extra-addtermdiv-<?php print $vid; ?>" style="display: none;">
	    	<?php print drupal_get_form('taxonomy_extra_form_addterm', $nid, $vid); ?>
	    </div>
    </div>
    <?php } ?>
  </div>
  <script>
  	<?php /* HACK (MAG, 5/28/2008): drupal wraps the textfield in a div, which bumps it down an extra line.
  	Changing the div property to inline removes this */ ?>
  	$('#taxonomy-extra-text-<?php print $vid ?>-wrapper').css('display', 'inline');
  </script>
<?php } ?>