<?php
// $Id: $

/**
 * @file modelorganism-main.tpl.php
 *
 * Theme implementation to display a modelorganism node
 *
 * - $node 
 * - $species
 * - $strain
 * - $source
 * - $keywords
 * - $body
 * - $genes
 * - $bio_ontologies: array of ont name => term list
 */
?>

<table id="modelorganism_details">
  <tbody>
    <tr class="modelorganism-species">
      <th>Species</th>
      <td><?php print $species ?></td>
    </tr>
    <tr class="modelorganism-strain">
      <th>Strain</th>
      <td><?php print $strain ?></td>
    </tr>
    <tr class="modelorganism-source">
      <th>Source</th>
      <td><?php print $source ?></td>
    </tr>
    <tr class="modelorganism-summary">
      <th>Summary</th>
      <td><?php print $body ?></td>
    </tr>
    <tr class="modelorganism-keywords">
      <th>Keywords</th>
      <td>
        <?php print $keywords ?>
      </td>
    </tr>
<?php foreach($bio_ontologies as $name => $terms): ?>
    <tr class="modelorganism-<?php print $name ?>">
      <th><?php print $name ?></th>
      <td>
        <?php print $terms ?>
      </td>
    </tr>
<?php endforeach; ?>
  </tbody>
</table>

<?php if (!empty($genes)): print $genes; endif;?>

<?php if (!empty($researchstatements)): print $researchstatements; endif;?>


