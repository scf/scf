<?php
// $Id: $

/**
 * @file antibody-table.tpl.php
 *
 * Theme implementation to display a antibody table
 *
 * - $nodes: 
 * - $title:
 * - $pager:
 */
?>

<?php if (count($nodes) > 0): ?>
<div id="antibodies">
  <?php if (isset($title)): ?><h3><?php print $title ?></h3><?php endif; ?>
  <table>
    <thead>
      <tr>
        <th class="antibody-title">Antibody</th>
        <th class="antibody-source">Source</th>
        <th class="antibody-host">Host</th>
        <th class="antibody-reactivity">Reactivity</th>
        <th class="antibody-clonality">Clonality</th>
        <th class="antibody-relationships bio-relationships" title="Relationships to model organisms, genes and research statements">Relationships</th>
        <th class="antibody-comments bio-comments" title="Comments">Comments</th>
      </tr>
    </thead>
    <tbody>
  <?php foreach ($nodes as $node): ?>
      <tr>
        <td class="antibody-title"><?php print l($node->title, 'node/' . $node->nid, array('attributes' => array('title' => t('Read more about this antibody...')))) ?></td>
        <td class="antibody-source"><?php print $node->source ?></td>
        <td class="antibody-host"><?php print $node->host ?></td>
        <td class="antibody-reactivity"><?php print $node->reactivity ?></td>
        <td class="antibody-clonality"><?php print $node->clonality ?></td>
        <td class="antibody-relationships bio-relationships"><?php print $node->association_count ?></td>
        <td class="antibody-comments bio-comments"><?php print $node->comment_count ?></td>
      </tr>
  <?php endforeach; ?>
    </tbody>
  </table>
  <div class="pager"><?php print $pager ?></div>
</div>
<?php endif; ?>
