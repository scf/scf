<div class="item-list">
<ul id="association-list-<?=$name?>">
<?php foreach ($items as $key => $item) { ?>
<li><?=$item ?>
<? if ($haslink) { 
	$aid = $associations[$key]->nid;
?>
&nbsp;<input type="image" title="Remove term" src="<?php print drupal_get_path('module', 'scf') ?>/images/icon-x.gif" onclick="$.post('/association/ajax/delete/<?=$nid?>/<?=$aid?>', {}, function() { $('a[href$=/node/<?=$aid?>]').parent().hide('slow'); });" />
<? } ?>
</li>
<?php } ?>
</ul>
<? if ($haslink) { ?>
	<div>
		<a href="#" onclick="$('#association-addtermdiv-<?=$name?>').slideToggle('slow');return false;">Add new item</a>
	</div>
	<div style="display:none;" id="association-addtermdiv-<?=$name?>">
		<!-- form goes here -->
		<?=$form?>
	</div>
	<script>$('#association-<?php print $name ?>-text-wrapper').css('display', 'inline');</script>
<? } ?>
</div>