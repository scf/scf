<?php
// $Id: $

/**
 * @file modelorganism-table.tpl.php
 *
 * Theme implementation to display a modelorganism table
 *
 * - $nodes: 
 * - $title:
 * - $pager:
 */
?>

<?php if (count($nodes) > 0): ?>
<div id="modelorganisms">
  <?php if (isset($title)): ?><h3><?php print $title ?></h3><?php endif; ?>
  <table>
    <thead>
      <tr>
        <th class="modelorganism-title">Model Organism</th>
        <th class="modelorganism-source">Source</th>
        <th class="modelorganism-species">Species</th>
        <th class="modelorganism-strain">Strain</th>
        <th class="modelorganism-relationships bio-relationships" title="Relationships to antibodies, genes and research statements">Relationships</th>
        <th class="modelorganism-comments bio-comments" title="Comments">Comments</th>
      </tr>
    </thead>
    <tbody>
<?php foreach ($nodes as $node): ?>
      <tr>
        <td class="modelorganism-title"><?php print l($node->title, 'node/' . $node->nid, array('attributes' => array('title' => t('Read more about this model organism...')))) ?></td>
        <td class="modelorganism-source"><?php print $node->source ?></td>
        <td class="modelorganism-species"><?php print $node->species ?></td>
        <td class="modelorganism-strain"><?php print $node->strain ?></td>
        <td class="modelorganism-relationships bio-relationships"><?php print $node->association_count ?></td>
        <td class="modelorganism-comments bio-comments"><?php print $node->comment_count ?></td>
      </tr>
  <?php endforeach; ?>
    </tbody>
  </table>
  <div class="pager"><?php print $pager ?></div>
</div>
<?php endif; ?>
