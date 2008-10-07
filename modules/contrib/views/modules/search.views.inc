<?php
// $Id: search.views.inc,v 1.5 2008/05/07 21:15:23 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers for search.module
 */

/**
 * @defgroup views_search_module search.module handlers
 *
 * Includes the tables 'search_index'
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function search_views_data() {
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['search_index']['table']['group']  = t('Search');

  // For other base tables, explain how we join
  $data['search_index']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'sid',
    ),
    'users' => array(
      'left_field' => 'uid',
      'field' => 'sid',
    ),
  );

  $data['search_total']['table']['join'] = array(
    'node' => array(
      'left_table' => 'search_index',
      'left_field' => 'word',
      'field' => 'word',
    ),
    'users' => array(
      'left_table' => 'search_index',
      'left_field' => 'word',
      'field' => 'word',
    )
  );

  $data['search_dataset']['table']['join'] = array(
    'node' => array(
      'left_table' => 'search_index',
      'left_field' => 'sid',
      'field' => 'sid',
    ),
    'users' => array(
      'left_table' => 'search_index',
      'left_field' => 'sid',
      'field' => 'sid',
    ),
  );

  // ----------------------------------------------------------------
  // Fields

  // score
  $data['search_index']['score'] = array(
    'title' => t('Score'), // The item it appears as on the UI,
    'help' => t('The score of the search item.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'field' => 'score', // the real field
      'group' => t('Search'), // The group it appears in on the UI,
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    // Information for sorting on a search score.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Search node links: forward links.
  $data['search_node_links_from']['table']['group'] = t('Search');
  $data['search_node_links_from']['table']['join'] = array(
    'node' => array(
      'arguments' => array('search_node_links', 'node', 'nid', 'nid', NULL, 'INNER'),
    ),
  );
  $data['search_node_links_from']['sid'] = array(
    'title' => t('Links from'),
    'help' => t('Nodes that link from the node.'),
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_equality',
    ),
  );

  // Search node links: backlinks.
  $data['search_node_links_to']['table']['group'] = t('Search');
  $data['search_node_links_to']['table']['join'] = array(
    'node' => array(
      'arguments' => array('search_node_links', 'node', 'nid', 'sid', NULL, 'INNER'),
    ),
  );
  $data['search_node_links_to']['nid'] = array(
    'title' => t('Links to'),
    'help' => t('Nodes that link to the node.'),
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_equality',
    ),
  );

  // search filter
  $data['search_index']['keys'] = array(
    'title' => t('Search Terms'), // The item it appears as on the UI,
    'help' => t('The terms to search for.'), // The help that appears on the UI,
    // Information for searching terms using the full search syntax
    'filter' => array(
      'handler' => 'views_handler_filter_search',
    ),
  );

  return $data;
}

/**
 * Field handler to provide simple renderer that allows linking to a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_filter_search extends views_handler_filter {
  /**
   * Provide basic defaults for the equality operator
   */
  function options(&$options) {
    parent::options($options);
    $options['operator'] = 'optional';
    $options['value'] = '';
  }

  /**
   * Provide simple equality operator
   */
  function operator_form(&$form, &$form_state) {
    $form['operator'] = array(
      '#type' => 'radios',
      '#title' => t('On empty input'),
      '#default_value' => $this->operator,
      '#options' => array(
        'optional' => t('Show All'),
        'required' => t('Show None'),
      ),
    );
  }

  /**
   * Provide a simple textfield for equality
   */
  function exposed_form(&$form, &$form_state) {
    $key = $this->options['expose']['identifier'];
    $form[$key] = array(
      '#type' => 'textfield',
      '#title' => t('Search'),
      '#size' => 15,
      '#default_value' => $this->value,
      '#attributes' => array('title' => t('Enter the terms you wish to search for.')),
    );
  }

  /**
   * Validate the options form.
   */
  function exposed_validate($form, &$form_state) {
    $key = $this->options['expose']['identifier'];
    if (!empty($form_state['values'][$key])) {
      $this->search_query = search_parse_query($form_state['values'][$key]);
      $this->search_query[0] = str_replace('d.', 'search_data.', $this->search_query[0]);
      $this->search_query[2] = str_replace('i.', 'search_index.', $this->search_query[2]);

      if ($this->search_query[2] == '') {
        form_set_error($key, t('You must include at least one positive keyword with @count characters or more.', array('@count' => variable_get('minimum_word_size', 3))));
      }
      if ($this->search_query[6]) {
        if ($this->search_query[6] == 'or') {
          drupal_set_message(t('Search for either of the two terms with uppercase <strong>OR</strong>. For example, <strong>cats OR dogs</strong>.'));
        }
      }
    }
  }

  /**
   * Add this filter to the query.
   *
   * Due to the nature of fapi, the value and the operator have an unintended
   * level of indirection. You will find them in $this->operator
   * and $this->value respectively.
   */
  function query() {
    $this->ensure_my_table();
    if (!isset($this->search_query)) {
      if ($this->operator == 'required') {
        $this->query->add_where($this->options['group'], '0');
      }
    }
    else {
      $join = new views_join;
      $join->construct('search_total', 'search_index', 'word', 'word');
      $this->query->add_relationship(NULL, $join, 'search_index');
#      $this->query->add_table('search_total', 'search_index', $join);
      $this->query->add_field('', "SUM(search_index.score * search_total.count)", 'score');
      $this->query->add_where($this->options['group'], $this->search_query[2], $this->search_query[3]);
      $this->query->add_where($this->options['group'], "search_index.type = '%s'", $this->view->base_table);
      if (!$this->search_query[5]) {
        $join = new views_join_search_dataset;
        $join->construct();
        $this->query->add_relationship(NULL, $join, 'search_index');
        $this->query->add_table('search_dataset', 'search_index', $join);
        $this->query->add_where($this->options['group'], $this->search_query[0], $this->search_query[1]);
      }
      // @note: i don't think that this needed since we restrict based on type
//    $this->query->add_groupby('node.type'); // isn't this the same as search_index.type?
      $this->query->add_groupby("search_index.sid");
      $this->query->add_having($this->options['group'], 'COUNT(*) >= %d', $this->search_query[4]);
      // @todo: normalize the score
    }
  }
}

class views_join_search_dataset extends views_join {
  function construct() {
    parent::construct('search_dataset', 'sid', array(), 'LEFT');
  }

  function join($table, &$query) {
    $output = parent::join($table, $query);
    $output .= " AND table[alias].type = search_dataset.type";
    return $output;
  }
}

/**
 * @}
 */

/**
 * Implementation of hook_views_plugins
 */
function search_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'row' => array(
      'search' => array(
        'title' => t('Search'),
        'help' => t('Display the results with standard search view.'),
        'handler' => 'views_plugin_row_search_view',
        'theme' => 'views_view_row_search',
        'base' => array('node'), // only works with 'node' as base.
      ),
    ),
  );
}

/**
 * Plugin which performs a node_view on the resulting object.
 *
 * @ingroup views_row_plugins
 */
class views_plugin_row_search_view extends views_plugin_row {
  function options(&$options) {
    $options['score'] = TRUE;
  }

  function options_form(&$form, &$form_state) {
    $form['score'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display score'),
      '#default_value' => $this->options['score'],
    );
  }

  /**
   * Override the behavior of the render() function.
   */
  function render($row) {
    return theme($this->theme_functions(), $this->view, $this->options, $row);
  }
}

/**
 * Template helper for theme_views_view_row_search
 */
function template_preprocess_views_view_row_search(&$vars) {
  $vars['node'] = ''; // make sure var is defined.
  $nid = $vars['row']->nid;
  if (!is_numeric($nid)) {
    return;
  }

  $node = node_load($nid);

  if (empty($node)) {
    return;
  }

  // Build the node body.
  $node = node_build_content($node, FALSE, FALSE);
  $node->body = drupal_render($node->content);

  // Fetch comments for snippet
  $node->body .= module_invoke('comment', 'nodeapi', $node, 'update index');

  // Fetch terms for snippet
  $node->body .= module_invoke('taxonomy', 'nodeapi', $node, 'update index');

  $vars['url'] = url('node/'. $nid);
  $vars['title'] = check_plain($node->title);

  $info = array();
  $info['type'] = node_get_types('name', $node);
  $info['user'] = theme('username', $node);
  $info['date'] = format_date($node->changed, 'small');
  $extra = node_invoke_nodeapi($node, 'search result');
  if (isset($extra) && is_array($extra)) {
    $info = array_merge($info, $extra);
  }
  $vars['info_split'] = $info;
  $vars['info'] = implode(' - ', $info);

  $vars['node'] = $node;
  // @todo: get score from ???
//$vars['score'] = $item->score;
  $vars['snippet'] = search_excerpt($vars['view']->value, $node->body);
}

/**
 * @}
 */

