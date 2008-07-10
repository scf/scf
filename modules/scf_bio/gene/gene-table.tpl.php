<?php
// $Id: $

/**
 * @file gene-table.tpl.php
 *
 * Theme implementation to display a gene table
 *
 * - $nodes: 
 * - $title:
 * - $pager:
 */
?>

<?php if (count($nodes) > 0): ?>
<div id="genes">
  <?php if (isset($title)): ?><h3><?php print $title ?></h3><?php endif; ?>
  <table>
    <thead>
      <tr>
        <th class="gene-title">Gene</th>
        <th class="gene-alias">Alias</th>
        <th class="gene-organism">Organism</th>
        <th class="gene-keywords">Keywords</th>
        <th class="gene-relationships bio-relationships" title="Relationships to reagents and research statements">Relationships</th>
        <!-- <th class="gene-comments bio-comments" title="Comments">Comments</th> -->
      </tr>
    </thead>
    <tbody>
  <?php foreach ($nodes as $node): ?>
      <tr>
        <td class="gene-title"><?php print l($node->title, 'node/' . $node->nid, array('attributes' => array('title' => t('Read more about this gene...')))) ?></td>
        <td class="gene-alias"><?php print $node->names ?></td>
        <td class="gene-organism"><?php print $node->organism ?></td>
        <td class="gene-keywords"><?php print $node->keywords ?></td>
        <td class="gene-relationships bio-relationships"><?php print $node->association_count ?></td>
        <!--<td class="gene-comments bio-comments"><?php print $node->comment_count ?></td>-->
      </tr>
  <?php endforeach; ?>
    </tbody>
  </table>
  <div class="pager"><?php print $pager ?></div>
</div>
<?php endif; ?>