<?php
// $Id: taxonomy.views.inc,v 1.29 2008/05/27 23:27:39 merlinofchaos Exp $
/**
 * @file
 *
 * Provide views data and handlers for taxonomy.module
 */

/**
 * @defgroup views_taxonomy_module taxonomy.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function taxonomy_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // vocabulary table

  $data['vocabulary']['table']['group']  = t('Taxonomy');

  $data['vocabulary']['table']['join'] = array(
    // vocabulary links to term_data directly via vid.
    'term_data' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to node through term_data via vid
    'node' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to node_revision via term_data
    'node_revision' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to users via term_data
    'users' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );

  // vocabulary name
  $data['vocabulary']['name'] = array(
    'title' => t('Vocabulary name'), // The item it appears as on the UI,
    'field' => array(
      'help' => t('Name of the vocabulary a term is a member of. This will be the vocabulary that whichever term the "Taxonomy: Term" field is; and can similarly cause duplicates.'),
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );
  $data['vocabulary']['vid'] = array(
    'title' => t('Vocabulary ID'), // The item it appears as on the UI,
    'help' => t('The taxonomy vocabulary ID'),
    'argument' => array(
      'handler' => 'views_handler_argument_vocabulary_vid',
      'name field' => 'name',
    ),
  );

  // ----------------------------------------------------------------------
  // term_data table

  $data['term_data']['table']['group']  = t('Taxonomy');
  $data['term_data']['table']['base'] = array(
    'field' => 'tid',
    'title' => t('Term'),
    'help' => t('Taxonomy terms are attached to nodes.'),
  );


  // The term data table
  $data['term_data']['table']['join'] = array(
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revision' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'users' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    // This is provided for many_to_one argument
    'term_node' => array(
      'field' => 'tid',
      'left_field' => 'tid',
    ),
  );

  // Term name field
  $data['term_data']['name'] = array(
    'title' => t('Term'),
    'help' => t('Taxonomy terms. Note that using this can cause duplicate nodes to appear in views; you must add filters to reduce the resultset.'),
    'field' => array(
      'handler' => 'views_handler_field_taxonomy',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'help' => t('Taxonomy term name'),
      'many to one' => TRUE,
      'empty name field' => t('Uncategorized'),
    ),
  );

  // taxonomy weight
  $data['term_data']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The term weight field'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Term description
  $data['term_data']['description'] = array(
    'title' => t('Term description'), // The item it appears as on the UI,
    'help' => t('The description associated with a taxonomy term.'),
    'field' => array(
      'field' => 'description', // the real field
      'group' => t('Taxonomy'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_markup',
      'format' => FILTER_FORMAT_DEFAULT,
    ),
  );

  // Term vocabulary
  $data['term_data']['vid'] = array(
    'title' => t('Vocabulary'),
    'help' => t('Filter the results of "Taxonomy: Term" to a particular vocabulary.'),
    'filter' => array(
      'handler' => 'views_handler_filter_vocabulary_vid',
    ),
  );

  // ----------------------------------------------------------------------
  // term_node table

  $data['term_node']['table']['group']  = t('Taxonomy');

  $data['term_node']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links directly to node via vid
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      // links directly to node_revisions via vid
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'users' => array(
      // Links to users via node.nid
      'left_table' => 'node',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'term_hierarchy' => array(
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  // tid field
  $data['term_node']['tid'] = array(
    'title' => t('Term ID'),
    'help' => t('The taxonomy term ID'),
    'field' => array(
      'title' => t('All terms'),
      'help' => t('Display all taxonomy terms associated with a node.'),
      'handler' => 'views_handler_field_term_node_tid',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid',
      'name table' => 'term_data',
      'name field' => 'name',
      'empty name field' => t('Uncategorized'),
      'numeric' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_term_node_tid',
      'hierarchy table' => 'term_hierarchy',
      'numeric' => TRUE,
    ),
  );

  // @todo: term_relation

  // ----------------------------------------------------------------------
  // term_hierarchy table

  $data['term_hierarchy']['table']['group']  = t('Taxonomy');

  $data['term_hierarchy']['table']['join'] = array(
    'term_hierarchy' => array(
      // links to self through left.parent = right.tid (going down in depth)
      'left_field' => 'tid',
      'field' => 'parent',
    ),
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links to node thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revisions' => array(
      // links to node_revisions thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'users' => array(
      // links to users thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  // ----------------------------------------------------------------------
  // term_synonym table

  $data['term_synonym']['table']['group']  = t('Taxonomy');

  $data['term_synonym']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revisions' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'users' => array(
      // Links to users via node
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['term_synonym']['name'] = array(
    'title' => t('Term synonym'),
    'help' => t('Term synonyms may be used to find terms by alternate names.'),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'many to one' => TRUE,
      'empty name field' => t('Uncategorized'),
    ),
  );
  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function taxonomy_views_data_alter(&$data) {
  $data['node']['term_node_tid_depth'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Term ID (with depth)'),
    'help' => t('The depth filter is more complex, so provides fewer options.'),
    'real field' => 'nid',
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid_depth',
      'accept depth modifier' => TRUE,
    ),
  );

  $data['node']['term_node_tid_depth_modifier'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Term ID depth modifier'),
    'help' => t('Allows the "depth" for Taxonomy: Term ID (with depth) to be modified via an additional argument.'),
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid_depth_modifier',
    ),
  );
}

/**
 * Argument handler for taxonomy terms with depth.
 *
 * This handler is actually part of the node table and has some restrictions,
 * because it uses a subquery to find nodes with
 */
class views_handler_argument_term_node_tid_depth extends views_handler_argument {
  function options(&$options) {
    parent::options($options);
    $options['depth'] = 0;
    $options['break_phrase'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['depth'] = array(
      '#type' => 'weight',
      '#title' => t('Depth'),
      '#default_value' => $this->options['depth'],
      '#description' => t('The depth will match nodes tagged with terms in the hierarchy. For example, if you have the term "fruit" and a child term "apple", with a depth of 1 (or higher) then filtering for the term "fruit" will get nodes that are tagged with "apple" as well as "fruit". If negative, the reverse is true; searching for "apple" will also pick up nodes tagged with "fruit" if depth is -1 (or lower).'),
    );

    $form['break_phrase'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow multiple terms per argument'),
      '#description' => t('If selected, users can enter multiple arguments in the form of 1+2+3. Due to the number of JOINs it would require, AND will be treated as OR with this argument.'),
      '#default_value' => !empty($this->options['break_phrase']),
    );

    $form['set_breadcrumb'] = array(
      '#type' => 'checkbox',
      '#title' => t("Set the breadcrumb for the term parents"),
      '#description' => t('If selected, the breadcrumb trail will include all parent terms, each one linking to this view. Note that this only works if just one term was received.'),
      '#default_value' => !empty($this->options['set_breadcrumb']),
    );
  }

  function set_breadcrumb(&$breadcrumb) {
    if (empty($this->options['set_breadcrumb']) || !is_numeric($this->argument)) {
      return;
    }

    return views_taxonomy_set_breadcrumb($breadcrumb, $this);
  }

  /**
   * Override default_actions() to remove summary actions.
   */
  function default_actions($which = NULL) {
    if ($which) {
      if (in_array($which, array('ignore', 'not found', 'empty', 'default'))) {
        return parent::default_actions($which);
      }
      return;
    }
    $actions = parent::default_actions();
    unset($actions['summary asc']);
    unset($actions['summary desc']);
    return $actions;
  }

  function query() {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      $tids = new stdClass();
      $tids->value = $this->argument;
      $tids = views_break_phrase($this->argument, $tids);
      if ($tids->value == -1) {
        return FALSE;
      }

      if (count($tids->value) > 1) {
        $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($tids->value), '%d')) . ")";
      }
      else {
        $placeholder = " = %d";
      }

      $tids = $tids->value;
    }
    else {
      $placeholder = "= %d";
      $tids = array($this->argument);
    }
    $subquery = "\n  SELECT tn.nid FROM {term_node} tn\n";
    $where = "  WHERE tn.tid $placeholder\n";
    $args = $tids;
    $last = "tn";

    if ($this->options['depth'] > 0) {
      $subquery .= "    LEFT JOIN {term_hierarchy} th ON th.tid = tn.tid\n";
      $last = "th";
      foreach (range(1, abs($this->options['depth'])) as $count) {
        $subquery .= "    LEFT JOIN {term_hierarchy} th$count ON $last.parent = th$count.tid\n";
        $where .= "    OR th$count.tid $placeholder\n";
        $args = array_merge($args, $tids);
        $last = "th$count";
      }
    }
    else if ($this->options['depth'] < 0) {
      foreach (range(1, abs($this->options['depth'])) as $count) {
        $subquery .= "    LEFT JOIN {term_hierarchy} th$count ON $last.tid = th$count.parent\n";
        $where .= "    OR th$count.tid $placeholder\n";
        $args = array_merge($args, $tids);
        $last = "th$count";
      }
    }

    $this->query->add_where(0, "$this->table_alias.$this->real_field IN ($subquery$where  )", $args);
  }

  function title() {
    $term = taxonomy_get_term($this->argument);
    if (!empty($term)) {
      return check_plain($term->name);
    }
    // TODO review text
    return t('No name');
  }
}

function views_taxonomy_set_breadcrumb(&$breadcrumb, &$argument) {
  if (empty($argument->options['set_breadcrumb'])) {
    return;
  }

  $args = $argument->view->args;
  $parents = taxonomy_get_parents_all($argument->argument);
  foreach (array_reverse($parents) as $parent) {
    // Unfortunately parents includes the current argument. Skip.
    if ($parent->tid == $argument->argument) {
      continue;
    }
    $args[$argument->position] = $parent->tid;
    $path = $argument->view->get_url($args);
    $breadcrumb[$path] = check_plain($parent->name);
  }
}


/**
 * Argument handler for to modify depth for a previous term.
 *
 * This handler is actually part of the node table and has some restrictions,
 * because it uses a subquery to find nodes with
 */
class views_handler_argument_term_node_tid_depth_modifier extends views_handler_argument {
  function options_form(&$form, &$form_state) { }
  function query() { }
  function pre_query() {
    // We don't know our argument yet, but it's based upon our position:
    $argument = isset($this->view->args[$this->position]) ? $this->view->args[$this->position] : NULL;
    if (!is_numeric($argument)) {
      return;
    }

    if ($argument > 10) {
      $argument = 10;
    }

    if ($argument < -10) {
      $argument = -10;
    }

    // figure out which argument preceded us.
    $keys = array_reverse(array_keys($this->view->argument));
    $skip = TRUE;
    foreach ($keys as $key) {
      if ($key == $this->options['id']) {
        $skip = FALSE;
        continue;
      }

      if ($skip) {
        continue;
      }

      if (!is_object($this->view->argument[$key]['handler'])) {
        continue;
      }

      if (isset($handler)) {
        unset($handler);
      }

      $handler = &$this->view->argument[$key]['handler'];
      if (empty($handler->definition['accept depth modifier'])) {
        continue;
      }

      // Finally!
      $handler->options['depth'] = $argument;
    }
  }
}

/**
 * Field handler for terms.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_term_node_tid extends views_handler_field_prerender_list {
  function init(&$view, $options) {
    parent::init($view, $options);
    if ($view->base_table == 'node_revisions') {
      $this->additional_fields['vid'] = array('table' => 'node_revisions', 'field' => 'vid');
    }
    else {
      $this->additional_fields['vid'] = array('table' => 'node', 'field' => 'vid');
    }
  }

  /**
   * Provide meaningful defaults
   */
  function options(&$options) {
    parent::options($options);
    $options['link_to_taxonomy'] = TRUE;
    $options['limit'] = FALSE;
    $options['vids'] = array();
  }

  /**
   * Provide "link to term" option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_taxonomy'] = array(
      '#title' => t('Link this field to its term page'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_taxonomy']),
    );

    $form['limit'] = array(
      '#type' => 'checkbox',
      '#title' => t('Limit terms by vocabulary'),
      '#default_value'=> $this->options['limit'],
    );

    $options = array();
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = $voc->name;
    }

    $form['vids'] = array(
      '#prefix' => '<div><div id="edit-options-vids">',
      '#suffix' => '</div></div>',
      '#type' => 'checkboxes',
      '#title' => t('Vocabularies'),
      '#options' => $options,
      '#default_value' => $this->options['vids'],
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-limit' => array(TRUE)),
    );
  }

  /**
   * Add this term to the query
   */
  function query() {
    $this->add_additional_fields();
  }

  function pre_render($values) {
    $this->field_alias = $this->aliases['vid'];
    $vids = array();
    foreach ($values as $result) {
      $vids[] = $result->{$this->aliases['vid']};
    }

    if ($vids) {
      $voc = '';
      if (!empty($this->options['limit']) && !empty($this->options['vids'])) {
        $voc = " AND td.vid IN (" . implode(', ', array_keys(array_filter($this->options['vids']))) . ")";
      }

      $result = db_query("SELECT tn.vid AS node_vid, td.* FROM {term_data} td INNER JOIN {term_node} tn ON td.tid = tn.tid WHERE tn.vid IN (" . implode(', ', $vids) . ")$voc ORDER BY td.weight, td.name");

      while ($term = db_fetch_object($result)) {
        if (empty($this->options['link_to_taxonomy'])) {
          $this->items[$term->node_vid][$term->tid] = check_plain($term->name);
        }
        else {
          $this->items[$term->node_vid][$term->tid] = l($term->name, taxonomy_term_path($term));
        }
      }
    }
  }
}

/**
 * Field handler to provide simple renderer that allows linking to a taxonomy
 * term.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_taxonomy extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   *
   * This constructer assumes the term_data table. If using another
   * table, we'll need to be more specific.
   */
  function construct() {
    parent::construct();
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['tid'] = 'tid';
  }

  /**
   * Provide link to taxonomy option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_taxonomy'] = array(
      '#title' => t('Link this field to its taxonomy term page'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_taxonomy']),
    );
  }

  /**
   * Render whatever the data is as a link to the taxonomy.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_taxonomy']) && !empty($values->{$this->aliases['tid']})) {
      $term = new stdClass();
      $term->tid = $values->{$this->aliases['tid']};
      $term->vid = $values->{$this->aliases['vid']};

      return l($data, taxonomy_term_path($term), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

class views_handler_argument_taxonomy extends views_handler_argument {

  /**
   * Override the behavior of title(). Get the title of the node.
   */
  function title() {
    $term = taxonomy_get_term($this->argument);
    if (!empty($term)) {
      return check_plain($term->name);
    }
    // TODO review text
    return t('No name');
  }
}

/**
 * Filter by vocabulary id
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_vocabulary_vid extends views_handler_filter_in_operator {
  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }

    $this->value_options = array();
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $this->value_options[$voc->vid] = $voc->name;
    }
  }
}

/**
 * Argument handler to accept a user id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_vocabulary_vid extends views_handler_argument {
  /**
   * Override the behavior of title(). Get the name of the user.
   */
  function title() {
    $title = db_result(db_query("SELECT v.name FROM {vocabulary} v WHERE v.vid = %d", $this->argument));

    if (empty($title)) {
      return t('No vocabulary');
    }

    return check_plain($title);
  }
}

/**
 * Filter by term id
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_term_node_tid extends views_handler_filter_many_to_one {
  function has_extra_options() { return TRUE; }

  function get_value_options() { /* don't overwrite the value options */ }

  function options(&$options) {
    parent::options($options);
    $options['type'] = 'textfield';
    $options['vid'] = db_result(db_query('SELECT MIN(vid) FROM {vocabulary} v'));
  }

  function extra_options_form(&$form, &$form_state) {
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = $voc->name;
    }

    $form['vid'] = array(
      '#prefix' => '<div class="views-left-40">',
      '#suffix' => '</div>',
      '#type' => 'radios',
      '#title' => t('Vocabulary'),
      '#options' => $options,
      '#description' => t('Select which vocabulary to show terms for in the regular options.'),
      '#default_value' => $this->options['vid'],
    );

    $form['markup_start'] = array(
      '#value' => '<div class="views-left-40">',
    );

    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Selection type'),
      '#options' => array('select' => t('Dropdown'), 'textfield' => t('Autocomplete')),
      '#default_value' => $this->options['type'],
    );

    $form['hierarchy'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show hierarchy in dropdown'),
      '#default_value' => !empty($this->options['hierarchy']),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('radio:options[type]' => array('select')),
    );

    $form['markup_end'] = array(
      '#value' => '</div>',
    );
  }

  function value_form(&$form, &$form_state) {
    $vocabulary = taxonomy_vocabulary_load($this->options['vid']);
    if (!$vocabulary) {
      $form['markup'] = array(
        '#prefix' => '<div class="form-item">',
        '#suffix' => '</div>',
        '#value' => t('An invalid vocabulary is selected. Please change it in the options.'),
      );
      return;
    }

    if ($this->options['type'] == 'textfield') {
      $default = '';
      if ($this->value) {
        $result = db_query("SELECT * FROM {term_data} td WHERE td.tid IN (" . implode(', ', $this->value) . ')');
        while ($term = db_fetch_object($result)) {
          if ($default) {
            $default .= ', ';
          }
          $default .= $term->name;
        }
      }

      $form['value'] = array(
        '#title' => t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name)),
        '#type' => 'textfield',
        '#autocomplete_path' => 'taxonomy/autocomplete/' . $vocabulary->vid,
        '#default_value' => $default,
      );
    }
    else {
      if (!empty($this->options['hierarchy'])) {
        $tree = taxonomy_get_tree($vocabulary->vid);
        $options = array();

        if ($tree) {
          foreach ($tree as $term) {
            $choice = new stdClass();
            $choice->option = array($term->tid => str_repeat('-', $term->depth) . $term->name);
            $options[] = $choice;
          }
        }
      }
      else {
        $options = array();
        $result = db_query("SELECT * FROM {term_data} WHERE vid = %d ORDER BY weight, name", $vocabulary->vid);
        while ($term = db_fetch_object($result)) {
          $options[$term->tid] = $term->name;
        }
      }

      // Since we're overriding, allow the list of options to be reduced.
      if (!empty($form_state['exposed']) && !empty($this->options['expose']['reduce'])) {
        $options = $this->reduce_value_options($options);
      }

      $form['value'] = array(
        '#type' => 'select',
        '#title' => t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name)),
        '#multiple' => TRUE,
        '#options' => $options,
        '#size' => min(9, count($options)),
      );
    }

    if (empty($form_state['exposed'])) {
      // Retain the helper option
      $this->helper->options_form($form, $form_state);
    }
  }

  function value_validate(&$form, &$form_state) {
    // We only validate if they've chosen the text field style.
    if ($this->options['type'] != 'textfield') {
      return;
    }

    $values = drupal_explode_tags($form_state['values']['options']['value']);
    $tids = $this->validate_term_strings($form['value'], $values);

    if ($tids) {
      $form_state['values']['options']['value'] = $tids;
    }
  }

  function exposed_validate(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }

    $identifier = $this->options['expose']['identifier'];

    // We only validate if they've chosen the text field style.
    if ($this->options['type'] != 'textfield') {
      if (!empty($this->options['expose']['single']) && $form_state['values'][$identifier] != 'All')  {
        $form_state['values'][$identifier] = array($form_state['values'][$identifier]);
      }
      return;
    }

    if (empty($this->options['expose']['identifier'])) {
      return;
    }

    $values = drupal_explode_tags($form_state['values'][$identifier]);

    $tids = $this->validate_term_strings($form[$identifier], $values);

    if ($uids) {
      $form_state['values'][$identifier] = $tids;
    }
  }

  /**
   * Validate the user string. Since this can come from either the form
   * or the exposed filter, this is abstracted out a bit so it can
   * handle the multiple input sources.
   */
  function validate_term_strings(&$form, $values) {
    if (empty($values)) {
      return array();
    }

    $tids = array();
    $placeholders = array();
    $args = array();
    $results = array();
    foreach ($values as $value) {
      $missing[strtolower($value)] = TRUE;
      $args[] = $value;
      $placeholders[] = "'%s'";
    }

    if (!$args) {
      return;
    }

    $result = db_query("SELECT * FROM {term_data} WHERE name IN (" . implode(', ', $placeholders) . ")", $args);
    while ($term = db_fetch_object($result)) {
      unset($missing[strtolower($term->name)]);
      $tids[] = $term->tid;
    }

    if ($missing) {
      form_error($form, format_plural(count($missing), 'Unable to find term: @terms', 'Unable to find terms: @terms', array('@terms' => implode(', ', array_keys($missing)))));
    }

    return $tids;
  }

  function value_submit($form, &$form_state) {
    // prevent array_filter from messing up our arrays in parent submit.
  }

  function admin_summary() {
    // set up $this->value_options for the parent summary
    $this->value_options = array();

    if ($this->value) {
      $result = db_query("SELECT * FROM {term_data} td WHERE td.tid IN ("  . implode(', ', $this->value) . ")");

      while ($term = db_fetch_object($result)) {
        $this->value_options[$term->tid] = $term->name;
      }
    }
    return parent::admin_summary();
  }
}

/**
 * Allow taxonomy term ID(s) as argument
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_term_node_tid extends views_handler_argument_many_to_one {
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['set_breadcrumb'] = array(
      '#type' => 'checkbox',
      '#title' => t("Set the breadcrumb for the term parents"),
      '#description' => t('If selected, the breadcrumb trail will include all parent terms, each one linking to this view. Note that this only works if just one term was received.'),
      '#default_value' => !empty($this->options['set_breadcrumb']),
    );
  }

  function set_breadcrumb(&$breadcrumb) {
    if (empty($this->options['set_breadcrumb']) || !is_numeric($this->argument)) {
      return;
    }

    return views_taxonomy_set_breadcrumb($breadcrumb, $this);
  }

  function title_query() {
    $titles = array();
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));

    $result = db_query("SELECT name FROM {term_data} WHERE tid IN ($placeholders)", $this->value);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->name);
    }
    return $titles;
  }
}

/**
 * Implementation of hook_views_plugins
 */
function taxonomy_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'argument validator' => array(
      'taxonomy_term' => array(
        'title' => t('Taxonomy term'),
        'handler' => 'views_plugin_argument_validate_taxonomy_term',
      ),
    ),
  );
}

/**
 * Validate whether an argument is an acceptable node.
 */
class views_plugin_argument_validate_taxonomy_term extends views_plugin_argument_validate {
  function validate_form(&$form, &$form_state) {
    $vocabularies = taxonomy_get_vocabularies();
    $options = array();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = $voc->name;
    }

    $form['validate_argument_vocabulary'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div id="edit-options-validate-argument-vocabulary-wrapper">',
      '#suffix' => '</div>',
      '#title' => t('Vocabularies'),
      '#options' => $options,
      '#default_value' => isset($this->argument->options['validate_argument_vocabulary']) ? $this->argument->options['validate_argument_vocabulary'] : array(),
      '#description' => t('If you wish to validate for specific vocabularies, check them; if none are checked, all nodes will pass.'),
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_type'] = array(
      '#type' => 'select',
      '#title' => t('Argument type'),
      '#options' => array(
        'tid' => t('Term ID'),
        'tids' => t('Term IDs separated by , or +'),
        'name' => t('Term name or synonym'),
        'convert' => t('Term name/synonym converted to Term ID'),
      ),
      '#default_value' => isset($this->argument->options['validate_argument_type']) ? $this->argument->options['validate_argument_type'] : 'tid',
      '#description' => t('Select the form of this argument; if using term name, it is generally more efficient to convert it to a term ID and use Taxonomy: Term ID rather than Taxonomy: Term Name" as an argument.'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );
  }

  function validate_argument($argument) {
    $vids = isset($this->argument->options['validate_argument_vocabulary']) ? array_filter($this->argument->options['validate_argument_vocabulary']) : array();

    $type = isset($this->argument->options['validate_argument_type']) ? $this->argument->options['validate_argument_type'] : 'tid';

    switch ($type) {
      case 'tid':
        if (!is_numeric($argument)) {
          return FALSE;
        }

        $result = db_fetch_object(db_query("SELECT * FROM {term_data} WHERE tid = %d", $argument));
        if (!$result) {
          return FALSE;
        }

        return empty($vids) || !empty($vids[$result->vid]);
      case 'tids':
        $tids = new stdClass();
        $tids->value = $argument;
        $tids = views_break_phrase($argument, $tids);
        if ($tids->value == -1) {
          return FALSE;
        }

        $placeholders = implode(', ', array_fill(0, sizeof($tids->value), '%d'));

        $test = drupal_map_assoc($tids->value);
        $titles = array();

        $result = db_query("SELECT * FROM {term_data} WHERE tid IN ($placeholders)", $tids->value);
        while ($term = db_fetch_object($result)) {
          if ($vids && empty($vids[$term->vid])) {
            return FALSE;
          }

          $titles[] = check_plain($term->name);
          unset($test[$term->tid]);
        }

        $this->argument->validated_title = implode($tids->operator == 'or' ? ' + ' : ', ', $titles);
        // If this is not empty, we did not find a tid.
        return empty($test);
      case 'name':
      case 'convert':
        $and = '';
        if (!empty($vids)) {
          $and = " AND td.vid IN(" . implode(', ', $vids) . ')';
        }

        $result = db_fetch_object(db_query("SELECT td.* FROM {term_data} td LEFT JOIN {term_synonym} ts ON ts.tid = td.tid WHERE (td.name = '%s' OR ts.name = '%s')$and", $argument, $argument));
        if (!$result) {
          return FALSE;
        }

        if ($type == 'convert') {
          $this->argument->argument = $result->tid;
        }

        return TRUE;
    }
  }
}

/**
 * @}
 */
