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
      <? if ($has_addlink) { ?>
      	<div><a href="/ontology/suggest/<?=$vid?>/<?=$nid?>">Suggest terms for this article</a></div>
      <? } ?>
    <ul id="taxonomy-extra-termlist-<?=$vid; ?>">
      <?php foreach ($terms as $term) { ?>
        <li><?=$term; ?></li>
      <?php } ?>
    </ul>
    <?php /* div added by MAG on 5/27/2008 */ if ($has_addlink) { ?>
    <div>
	    <a href="#" onclick="$('#taxonomy-extra-addtermdiv-<?=$vid ?>').slideToggle('slow');return false;">Add new <? if ($vid==6 or $vid==7 or $vid==8) { ?>GO <? } ?>term</a>
	    <div id="taxonomy-extra-addtermdiv-<?=$vid; ?>" style="display: none;">
	    	<?=drupal_get_form('taxonomy_extra_form_addterm', $nid, $vid); ?>
	    </div>
    </div>
    <?php } ?>
  </div>
  <script>
  	<?php /* HACK (MAG, 5/28/2008): drupal wraps the textfield in a div, which bumps it down an extra line.
  	Changing the div property to inline removes this */ ?>
  	$('#taxonomy-extra-text-<?=$vid ?>-wrapper').css('display', 'inline');
  	/*$('#taxonomy-extra-termlist-<?=$vid ?> li a').mouseover(function() {
      $('input', this.parentNode).attr('src', '/sites/all/themes/scf_base/images/icon-x-hover.gif');
    }).mouseout(function() {
      $('input', this.parentNode).attr('src', '/sites/all/themes/scf_base/images/icon-x.gif');
    });
   
    $('#taxonomy-extra-termlist-<?=$vid ?> li input').mouseover(function() {
      this.src = '/sites/all/themes/scf_base/images/icon-x-hover.gif';
    }).mouseout(function() {
      this.src = '/sites/all/themes/scf_base/images/icon-x.gif';
    });*/
  </script>
<?php } ?>