<?php
// $Id: $

/**
 * @file researchstatement-main.tpl.php
 *
 * Theme implementation to display a researchstatement node
 * 
 * - $node 
 * - $statement
 * - $genes
 * - $antibodies
 * - $modelorganisma
 */
?>

<table id="researchstatement_details">
  <tbody>
    <tr class="researchstatement-statement">
      <th>Statement</th>
      <td><?php print $statement ?></td>
    </tr>
    <tr class="researchstatement-id">
      <th>PubMed ID</th>
      <td><?php print $pubmed_link ?></td>
    </tr>
    <tr class="researchstatement-keywords">
      <th>Keywords</th>
      <td>
        <?php print $keywords ?>
      </td>
    </tr>
<?php foreach($bio_ontologies as $name => $terms): ?>
    <tr class="researchstatement-<?php print $name ?>">
      <th><?php print $name ?></th>
      <td>
        <?php print $terms ?>
      </td>
    </tr>
<?php endforeach; ?>
  </tbody>
</table>

<?php if (!empty($genes)): print $genes; endif;?>

<?php if (!empty($antibodies)): print $antibodies; endif;?>

<?php if (!empty($modelorganisms)): print $modelorganisms; endif;?>


