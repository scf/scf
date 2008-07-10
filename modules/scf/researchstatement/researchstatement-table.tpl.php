<?php
// $Id: $

/**
 * @file researchstatement-table.tpl.php
 *
 * Theme implementation to display a researchstatement table
 *
 * - $nodes: 
 * - $title:
 * - $pager:
 */
?>

<?php if (count($nodes) > 0): ?>
<div id=id="researchstatements">
  <?php if (isset($title)): ?><h3><?php print $title ?></h3><?php endif; ?>
  <table>
    <thead>
      <tr>
        <th class="researchstatement-statement">Statement</th>
        <th class="researchstatement-id">PubMed ID</th>
        <th class="researchstatement-relationships bio-relationships" title="Relationships to reagents and genes">Relationships</th>
        <th class="researchstatement-comments bio-comments" title="Comments">Comments</th>
      </tr>
    </thead>
    <tbody>
  <?php foreach ($nodes as $node): ?>
      <tr>
        <td class="researchstatement-statement"><?php print l($node->title, 'node/' . $node->nid, array('attributes' => array('title' => t('Read more about this research statement...')))); ?></td>
        <td class="researchstatement-id"><?php print theme('researchstatement_pubmed_link', $node, FALSE) ?></td>
        <td class="researchstatement-relationships bio-relationships"><?php print $node->association_count ?></td>
        <td class="researchstatement-comments bio-comments"><?php print $node->comment_count ?></td>
      </tr>
  <?php endforeach; ?>
    </tbody>
  </table>
  <div class="pager"><?php print $pager ?></div>
</div>
<?php endif; ?>
