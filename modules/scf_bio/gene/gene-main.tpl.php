<?php
// $Id: $

/**
 * @file gene-main.tpl.php
 *
 * Theme implementation to display a gene node
 *
 * - $node: 
 * - $egid:
 * - $symbol:
 * - $names:
 * - $organism:
 * - $keywords:
 * - $body:
 * - $phenotypes:
 * - $bio_ontologies: array of ont name => term list
 */
?>

<table id="gene_details">
  <tbody>
    <tr class="gene-id">
      <th>Entrez Gene ID</th>
      <td><?php print l($egid, "http://www.ncbi.nlm.nih.gov/sites/entrez?Db=gene&Cmd=DetailsSearch&Term=${egid}%5Buid%5D") ?></td>
    </tr>
    <tr class="gene-symbol">
      <th>Symbol</th>
      <td><?php print $symbol ?></td>
    </tr>
    <tr class="gene-names">
      <th>Names</th>
      <td><?php print $names ?></td>
    </tr>
    <tr class="gene-organism">
      <th>Organism</th>
      <td><?php print $organism ?></td>
    </tr>
    <tr class="gene-keywords">
      <th>Keywords</th>
      <td>
        <?php print $keywords ?>
      </td>
    </tr>
    <tr class="gene-summary">
      <th>Summary</th>
      <td><?php print $body ?></td>
    </tr>
    <tr class="gene-phenotypes">
      <th>Phenotypes</th>
      <td><?php print $phenotypes ?></td>
    </tr>
<?php foreach($bio_ontologies as $name => $terms): ?>
    <tr class="gene-<?php print $name ?>">
      <th><?php print $name ?></th>
      <td>
        <?php print $terms ?>
      </td>
    </tr>
<?php endforeach; ?>
  </tbody>
</table>

<?php if (!empty($antibodies)): print $antibodies; endif;?>

<?php if (!empty($modelorganisms)): print $modelorganisms; endif;?>

<?php if (!empty($researchstatements)): print $researchstatements; endif;?>
