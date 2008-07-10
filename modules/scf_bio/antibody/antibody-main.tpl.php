<?php
// $Id: $

/**
 * @file antibody-main.tpl.php
 *
 * Theme implementation to display a antibody node
 *
 * - $node 
 * - $clonality
 * - $body
 * - $source
 * - $host
 * - $reactivity
 * - $keywords
 * - $genes: list of genes
 * - $bio_ontologies: array of ont name => term list
 */
?>

<table id="antibody_details">
  <tbody>
    <tr class="antibody-clonality">
      <th>Clonality</th>
      <td><?php print $clonality ?></td>
    </tr>
    <tr class="antibody-source">
      <th>Source</th>
      <td><?php print $source ?></td>
    </tr>
    <tr class="antibody-host">
      <th>Host</th>
      <td><?php print $host ?></td>
    </tr>
    <tr class="antibody-reactivity">
      <th>Reactivity</th>
      <td><?php print $reactivity ?></td>
    </tr>
    <tr class="antibody-keywords">
      <th>Keywords</th>
      <td>
        <?php print $keywords ?>
      </td>
    </tr>
    <tr class="antibody-summary">
      <th>Summary</th>
      <td><?php print $body ?></td>
    </tr>
<?php foreach($bio_ontologies as $name => $terms): ?>
    <tr class="antibody-<?php print $name ?>">
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

