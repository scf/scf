<?php
// $Id: $

/**
 * @file bio-resources.tpl.php
 *
 * - $nodes: raw array(<modulename> => <node array>);
 * - $tables: array(<modulename> => html) 
 */
foreach ($tables as $module => $table) {
  print $table;
  print "\n<br/>";
  if (isset($search[$module])) {
    print '<div class="search_link">';
    print $search[$module];
    print "</div>\n<br/>";
  }
}
?>