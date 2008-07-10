<?php

// $Id: scf.pages.inc $

/**
 * will go away if using views for stembook page?
 */
function scf_pub_dir() {
  $pub_modules = array('interview', 'pubgroup');

  $out = '';

  foreach ($pub_modules as $mod) {
    if (module_exists($mod)) {
      $out .= module_invoke($mod, 'list_view');
    }
  }

  return $out;
}

/**
 * TODO: this bio stuff doesn't belong in SCF proper
 */
function scf_resources_dir() {
  $res_modules = array('gene', 'antibody', 'modelorganism', 'researchstatement');

  $nodes = array();
  $pagers = array();

  $elt = 0;
  foreach ($res_modules as $mod) {
    if (module_exists($mod)) {
      list($n, $pager) = module_invoke($mod, 'load_recent_nodes', 5, $elt++);
      $nodes[$mod] = $n;
      $pagers[$mod] = $pager;
    }
  }

  return theme('bio_resources', $nodes, $pagers);
}

/**
 * @param $op determines what node field to change
 * @param $value "on", "off" or "toggle"
 */
function scf_set_node_publishing_options ($node, $op, $value = 'on') {
  if (node_access('update', $node)) {
    $field = NULL;
    switch ($op) {
      case 'publish':
        $field = 'status';
        break;
      case 'promote':
        $field = 'promote';
        break;
      case 'feature':
        $field = 'sticky';
        break;
      case 'promotefeature':
        $field = array('sticky', 'promote');
        break;
    }
    if (!empty($field)) {
      // hack for "promotefeature" op
      if (is_array($field)) {
        foreach ($field as $f) {
          $node->$f = 1;
        }
      } else {
        $cur = isset($node->$field) ? $node->$field : 0;
        if ($value == 'on' || $value == 'off') {
          $node->$field = ($value == 'on' ? 1 : 0);
        } else if ($value == 'toggle') {
          $node->$field = $cur ? 0 : 1;
        }
      }
      node_save($node);
    }
  } else {
    drupal_set_message('You do not have permission to update this content item.', 'error');
  }
  // use destination from request url
  drupal_goto();
}
