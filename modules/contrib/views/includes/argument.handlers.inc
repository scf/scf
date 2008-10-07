<?php
// $Id: argument.handlers.inc,v 1.23 2008/05/29 00:16:48 merlinofchaos Exp $
/**
 * @file
 * Views' argument handlers.
 */

/**
 * @defgroup views_argument_handlers Handlers for arguments
 * @{
 */
/**
 * Base class for arguments.
 *
 * The basic argument works for very simple arguments such as nid and uid
 */
class views_handler_argument extends views_handler {
  var $name_field = NULL;
  /**
   * Constructor
   */
  function construct() {
    if (!empty($this->definition['name field'])) {
      $this->name_field = $this->definition['name field'];
    }
    if (!empty($this->definition['name table'])) {
      $this->name_table = $this->definition['name table'];
    }
  }

  function init(&$view, &$options) {
    parent::init($view, $options);
    // Ensure this is set; early arguments did not have validators.
    if (empty($this->options['validate_type'])) {
      $this->options['validate_type'] = 'none';
    }
    if (empty($this->options['validate_fail'])) {
      $this->options['validate_fail'] = 'not found';
    }
    if (empty($this->options['default_argument_type'])) {
      $this->options['default_argument_type'] = 'fixed';
    }
  }

  /**
   * Give an argument the opportunity to modify the breadcrumb, if it wants.
   * This only gets called on displays where a breadcrumb is actually used.
   *
   * The breadcrumb will be in the form of an array, with the keys being
   * the path and the value being the already sanitized title of the path.
   */
  function set_breadcrumb(&$breadcrumb) { }

  /**
   * Determine if the argument can generate a breadcrumb
   *
   * @return TRUE/FALSE
   */
  function uses_breadcrumb() {
    $info = $this->default_actions($this->options['default_action']);
    return !empty($info['breadcrumb']);
  }

  function is_wildcard() {
    return !empty($this->options['wildcard']) && $this->options['wildcard'] == $this->argument;
  }

  function wildcard_title() {
    return $this->options['wildcard_substitution'];
  }

  /**
   * Determine if the argument needs a style plugin.
   *
   * @return TRUE/FALSE
   */
  function needs_style_plugin() {
    $info = $this->default_actions($this->options['default_action']);
    return !empty($info['style plugin']);
  }

  /**
   * Provide defaults for the argument when a new one is created.
   */
  function options(&$options) {
    parent::options($options);
    $options['default_action'] = 'ignore';
    $options['style_plugin'] = 'default_summary';
    // provide default options for the summary style.
    $options['style_options'] = array();
    views_plugin_style_summary::options($options['style_options']);
    $options['wildcard'] = 'all';
    $options['wildcard_substitution'] = t('All');
    $options['title'] = '';
    $options['default_argument_type'] = 'fixed';
    $options['default_argument'] = '';
    $options['validate_type'] = 'none';
    $options['validate_fail'] = 'not found';
  }

  /**
   * Provide a default options form for the argument.
   */
  function options_form(&$form, &$form_state) {
    $defaults = $this->default_actions();

    $form['title'] = array(
      '#prefix' => '<div class="clear-block">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#default_value' => $this->options['title'],
      '#description' => t('The title to use when this argument is present; it will override the title of the view and titles from previous arguments. You can use percent substitution here to replace with argument titles. Use "%1" for the first argument, "%2" for the second, etc.'),
    );

    $form['clear_start'] = array(
      '#value' => '<div class="clear-block">',
    );

    $form['defaults_start'] = array(
      '#value' => '<div class="views-left-50">',
    );

    $form['default_action'] = array(
      '#type' => 'radios',
      '#title' => t('Action to take if argument is not present'),
      '#default_value' => $this->options['default_action'],
    );

    $form['defaults_stop'] = array(
      '#value' => '</div>',
    );

    $form['wildcard'] = array(
      '#prefix' => '<div class="views-right-50">',
      // prefix and no suffix means these two items will be grouped together.
      '#type' => 'textfield',
      '#title' => t('Wildcard'),
      '#size' => 20,
      '#default_value' => $this->options['wildcard'],
      '#description' => t('If this value is received as an argument, the argument will be ignored; i.e, "all values"'),
    );

    $form['wildcard_substitution'] = array(
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Wildcard title'),
      '#size' => 20,
      '#default_value' => $this->options['wildcard_substitution'],
      '#description' => t('The title to use for the wildcard in substitutions elsewhere.'),
    );

    $form['clear_stop'] = array(
      '#value' => '</div>',
    );

    $options = array();
    $validate_options = array();
    foreach ($defaults as $id => $info) {
      $options[$id] = $info['title'];
      if (empty($info['default only'])) {
        $validate_options[$id] = $info['title'];
      }
      if (!empty($info['form method'])) {
        $this->{$info['form method']}($form, $form_state);
      }
    }

    $form['default_action']['#options'] = $options;

    $form['validate_type'] = array(
      '#type' => 'select',
      '#title' => t('Validator'),
      '#default_value' => $this->options['validate_type'],
    );

    $validate_types = array('none' => t('<Basic validation>'));
    $plugins = views_fetch_plugin_data('argument validator');
    foreach ($plugins as $id => $info) {
      $valid = TRUE;
      if (!empty($info['type'])) {
        $valid = FALSE;
        if (empty($this->definition['validate type'])) {
          continue;
        }
        foreach ((array) $info['type'] as $type) {
          if ($type == $this->definition['validate type']) {
            $valid = TRUE;
            break;
          }
        }
      }

      // If we decide this validator is ok, add it to the list.
      if ($valid) {
        $plugin = views_get_plugin('argument validator', $id);
        if ($plugin) {
          $plugin->init($this->view, $this, $id);
          if ($plugin->access()) {
            $plugin->validate_form($form, $form_state, $id);
            $validate_types[$id] = $info['title'];
          }
        }
      }
    }

    asort($validate_types);
    $form['validate_type']['#options'] = $validate_types;
    // Show this gadget if *anything* but 'none' is selected

    $form['validate_fail'] = array(
      '#type' => 'select',
      '#title' => t('Action to take if argument does not validate'),
      '#default_value' => $this->options['validate_fail'],
      '#options' => $validate_options,
    );
  }

  /**
   * Provide a list of default behaviors for this argument if the argument
   * is not present.
   *
   * Override this method to provide additional (or fewer) default behaviors.
   */
  function default_actions($which = NULL) {
    $defaults = array(
      'ignore' => array(
        'title' => t('Display all values'),
        'method' => 'default_ignore',
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'not found' => array(
        'title' => t('Hide view / Page not found (404)'),
        'method' => 'default_not_found',
      ),
      'empty' => array(
        'title' => t('Display empty text'),
        'method' => 'default_empty',
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'summary asc' => array(
        'title' => t('Summary, sorted ascending'),
        'method' => 'default_summary',
        'method args' => array('asc'),
        'style plugin' => TRUE,
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'summary desc' => array(
        'title' => t('Summary, sorted descending'),
        'method' => 'default_summary',
        'method args' => array('desc'),
        'style plugin' => TRUE,
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'default' => array(
        'title' => t('Provide default argument'),
        'method' => 'default_default',
        'form method' => 'default_argument_form',
        'has default argument' => TRUE,
        'default only' => TRUE, // this can only be used for missing argument, not validation failure
      ),
    );

    if ($which) {
      if (!empty($defaults[$which])) {
        return $defaults[$which];
      }
    }
    else {
      return $defaults;
    }
  }

  /**
   * Provide a form for selecting the default argument when the
   * default action is set to provide default argument.
   */
  function default_argument_form(&$form, &$form_state) {
    $plugins = views_fetch_plugin_data('argument default');
    $options = array();

    $form['default_argument_type'] = array(
      '#prefix' => '<div id="edit-options-default-argument-type-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'radios',
      '#id' => 'edit-options-default-argument-type',
      '#title' => t('Default argument type'),
      '#default_value' => $this->options['default_argument_type'],
      '#process' => array('expand_radios', 'views_process_dependency'),
      '#dependency' => array('radio:options[default_action]' => array('default')),
    );

    foreach ($plugins as $id => $info) {
      $plugin = views_get_plugin('argument default', $id);
      if ($plugin) {
        $plugin->init($this->view, $this, $id);

        if ($plugin->access() || $this->options['default_argument_type'] == $id) {
          $options[$id] = $info['title'];
          $plugin->argument_form($form, $form_state);
        }
      }
    }

    asort($options);
    $form['default_argument_type']['#options'] = $options;
  }

  /**
   * Handle the default action, which means our argument wasn't present.
   *
   * Override this method only with extreme care.
   *
   * @return
   *   A boolean value; if TRUE, continue building this view. If FALSE,
   *   building the view will be aborted here.
   */
  function default_action($info = NULL) {
    if (!isset($info)) {
      $info = $this->default_actions($this->options['default_action']);
    }

    if (!$info) {
      return FALSE;
    }

    if (!empty($info['method args'])) {
      return call_user_func_array(array(&$this, $info['method']), $info['method args']);
    }
    else {
      return $this->{$info['method']}();
    }
  }

  /**
   * How to act if validation failes
   */
  function validate_fail() {
    $info = $this->default_actions($this->options['validate_fail']);
    return $this->default_action($info);
  }
  /**
   * Default action: ignore.
   *
   * If an argument was expected and was not given, in this case, simply
   * ignore the argument entirely.
   */
  function default_ignore() {
    return TRUE;
  }

  /**
   * Default action: not found.
   *
   * If an argument was expected and was not given, in this case, report
   * the view as 'not found' or hide it.
   */
  function default_not_found() {
    // Set a failure condition and let the display manager handle it.
    $this->view->build_info['fail'] = TRUE;
    return FALSE;
  }

  /**
   * Default action: empty
   *
   * If an argument was expected and was not given, in this case, display
   * the view's empty text
   */
  function default_empty() {
    // We return with no query; this will force the empty text.
    $this->view->built = TRUE;
    $this->view->executed = TRUE;
    $this->view->result = array();
    return FALSE;
  }

  /**
   * This just returns true. The view argument builder will know where
   * to find the argument from.
   */
  function default_default() {
    return TRUE;
  }

  /**
   * Determine if the argument is set to provide a default argument.
   */
  function has_default_argument() {
    $info = $this->default_actions($this->options['default_action']);
    return !empty($info['has default argument']);
  }

  /**
   * Get a default argument, if available.
   */
  function get_default_argument() {
    $plugin = views_get_plugin('argument default', $this->options['default_argument_type']);
    if ($plugin) {
      $plugin->init($this->view, $this);
      return $plugin->get_argument();
    }
  }

  /**
   * Default action: summary.
   *
   * If an argument was expected and was not given, in this case, display
   * a summary query.
   */
  function default_summary($order) {
    $this->view->build_info['summary'] = TRUE;
    $this->view->build_info['summary_level'] = $this->options['id'];

    // Change the display style to the summary style for this
    // argument.
    $this->view->plugin_name = $this->options['style_plugin'];
    $this->view->style_options = $this->options['style_options'];

    // Clear out the normal primary field and whatever else may have
    // been added and let the summary do the work.
    $this->query->clear_fields();
    $this->summary_query();

    $this->summary_sort($order);

    // Summaries have their own sorting and fields, so tell the View not
    // to build these.
    $this->view->build_sort = $this->view->build_fields = FALSE;
    return TRUE;
  }

  /**
   * Build the info for the summary query.
   *
   * This must:
   * - add_groupby: group on this field in order to create summaries.
   * - add_field: add a 'num_nodes' field for the count. Usually it will
   *   be a count on $view->base_field
   * - set_count_field: Reset the count field so we get the right paging.
   *
   * @return
   *   The alias used to get the number of records (count) for this entry.
   */
  function summary_query() {
    $this->ensure_my_table();
    // Add the field.
    $this->base_alias = $this->query->add_field($this->table_alias, $this->real_field);

    $this->summary_name_field();
    return $this->summary_basics();
  }

  /**
   * Add the name field, which is the field displayed in summary queries.
   * This is often used when the argument is numeric.
   */
  function summary_name_field() {
    // Add the 'name' field. For example, if this is a uid argument, the
    // name field would be 'name' (i.e, the username).

    if (isset($this->name_table)) {
      // if the alias is different then we're probably added, not ensured,
      // so look up the join and add it instead.
      if ($this->table_alias != $this->table) {
        $j = views_get_table_join($this->name_table, $this->table);
        if ($j) {
          $join = drupal_clone($j);
          $join->left_table = $this->table_alias;
          $this->name_table_alias = $this->query->add_table($this->name_table, $this->relationship, $join);
        }
      }
      else {
        $this->name_table_alias = $this->query->ensure_table($this->name_table, $this->relationship);
      }
    }
    else {
      $this->name_table_alias = $this->table_alias;
    }

    if (isset($this->name_field)) {
      $this->name_alias = $this->query->add_field($this->name_table_alias, $this->name_field);
    }
    else {
      $this->name_alias = $this->base_alias;
    }
  }

  /**
   * Some basic summary behavior that doesn't need to be repeated as much as
   * code that goes into summary_query()
   */
  function summary_basics($count_field = TRUE) {
    // Add the number of nodes counter
    $count_alias = $this->query->add_field(NULL, 'COUNT('. $this->query->base_table .'.'. $this->query->base_field . ')', 'num_records');
    $this->query->add_groupby($this->name_alias);

    if ($count_field) {
      $this->query->set_count_field($this->table_alias, $this->real_field);
    }

    $this->count_alias = $count_alias;
  }

  /**
   * Sorts the summary based upon the user's selection. The base variant of
   * this is usually adequte.
   *
   * @param $order
   *   The order selected in the UI.
   */
  function summary_sort($order) {
    $this->query->add_orderby(NULL, NULL, $order, $this->name_alias);
  }

  /**
   * Provide the argument to use to link from the summary to the next level;
   * this will be called once per row of a summary, and used as part of
   * $view->get_url().
   *
   * @param $data
   *   The query results for the row.
   */
  function summary_argument($data) {
    return $data->{$this->base_alias};
  }

  /**
   * Provides the name to use for the summary. By default this is just
   * the name field.
   *
   * @param $data
   *   The query results for the row.
   */
  function summary_name($data) {
    $value = $data->{$this->name_alias};
    if (empty($value) && !empty($this->definition['empty field name'])) {
      $value = $this->definition['empty field name'];
    }
    return check_plain($value);
  }

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  function query() {
    $this->ensure_my_table();
    $placeholder = empty($this->definition['numeric']) ? "'%s'" : '%d';
    $this->query->add_where(0, "$this->table_alias.$this->real_field = $placeholder", $this->argument);
  }

  /**
   * Get the title this argument will assign the view, given the argument.
   *
   * This usually needs to be overridden to provide a proper title.
   */
  function title() {
    return check_plain($this->argument);
  }

  /**
   * Called by the view object to get the title. This may be set by a
   * validator so we don't necessarily call through to title().
   */
  function get_title() {
    if (isset($this->validated_title)) {
      return $this->validated_title;
    }
    else {
      return $this->title();
    }
  }

  /**
   * Validate that this argument works. By default, all arguments are valid.
   */
  function validate_argument($arg) {
    // By using % in URLs, arguments could be validated twice; this eases
    // that pain.
    if (isset($this->argument_validated)) {
      return $this->argument_validated;
    }

    if ($this->options['validate_type'] == 'none') {
      return $this->argument_validated = $this->validate_argument_basic($arg);
    }

    $plugin = views_get_plugin('argument validator', $this->options['validate_type']);
    if ($plugin) {
      $plugin->init($this->view, $this, $this->options['validate_type']);
      return $this->argument_validated = $plugin->validate_argument($arg);
    }

    // If the plugin isn't found, fall back to the basic validation path:
    return $this->argument_validated = $this->validate_argument_basic($arg);
  }

  /**
   * Provide a basic argument validation.
   *
   * This can be overridden for more complex types; the basic
   * validator only checks to see if the argument is not NULL
   * or is numeric if the definition says it's numeric.
   */
  function validate_argument_basic($arg) {
    if (!isset($arg) || $arg === '') {
      return FALSE;
    }

    if (isset($this->definition['numeric']) && !isset($this->options['break_phrase']) && !is_numeric($arg)) {
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Set the input for this argument
   *
   * @return TRUE if it successfully validates; FALSE if it does not.
   */
  function set_argument($arg) {
    $this->argument = $arg;
    if ($this->is_wildcard()) {
      return TRUE;
    }

    return $this->validate_argument($arg);
  }
}

/**
 * Abstract argument handler for simple formulae.
 *
 * Child classes of this object should implement summary_argument, at least.
 */
class views_handler_argument_formula extends views_handler_argument {
  var $formula = NULL;
  /**
   * Constructor
   */
  function construct() {
    if (!empty($this->definition['formula'])) {
      $this->formula = $this->definition['formula'];
    }
  }

  function get_formula() {
    return str_replace('***table***', $this->table_alias, $this->formula);
  }

  /**
   * Build the summary query based on a formula
   */
  function summary_query() {
    $this->ensure_my_table();
    // Now that our table is secure, get our formula.
    $formula = $this->get_formula();

    // Add the field.
    $this->base_alias = $this->name_alias = $this->query->add_field(NULL, $formula, $this->field);
    $this->query->set_count_field(NULL, $formula, $this->field);

    return $this->summary_basics(FALSE);
  }

  /**
   * Build the query based upon the formula
   */
  function query() {
    $this->ensure_my_table();
    // Now that our table is secure, get our formula.
    $formula = $this->get_formula();

    $this->query->add_where(0, "$formula = '%s'", $this->argument);
  }
}

/**
 * Basic argument handler to implement string arguments that may have length
 * limits.
 */
class views_handler_argument_string extends views_handler_argument {
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($this->definition['many to one'])) {
      $this->helper = new views_many_to_one_helper($this);

      // Ensure defaults for these, during summaries and stuff:
      $this->operator = 'or';
      $this->value = array();
    }
  }

  function options(&$options) {
    parent::options($options);
    $options['glossary'] = FALSE;
    $options['limit'] = 0;
    $options['case'] = 'none';
    $options['path_case'] = 'none';
    $options['transform_dash'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['glossary'] = array(
      '#type' => 'checkbox',
      '#title' => t('Glossary mode'),
      '#description' => t('Glossary mode applies a limit to the number of characters used in the argument, which allows the summary view to act as a glossary.'),
      '#default_value' => $this->options['glossary'],
    );

    $form['limit'] = array(
      '#type' => 'textfield',
      '#title' => t('Character limit'),
      '#description' => t('How many characters of the argument to filter against. If set to 1, all fields starting with the letter in the argument would be matched.'),
      '#default_value' => $this->options['limit'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-glossary' => array(TRUE)),
    );

    $form['case'] = array(
      '#type' => 'select',
      '#title' => t('Case'),
      '#description' => t('When printing the argument result, how to transform the case.'),
      '#options' => array(
        'none' => t('No transform'),
        'upper' => t('Upper case'),
        'lower' => t('Lower case'),
        'ucfirst' => t('Capitalize first letter'),
        'ucwords' => t('Capitalize each word'),
      ),
      '#default_value' => $this->options['case'],
    );

    $form['path_case'] = array(
      '#type' => 'select',
      '#title' => t('Case in path'),
      '#description' => t('When printing url paths, how to transform the of the argument. Do not use this unless with Postgres as it uses case sensitive comparisons.'),
      '#options' => array(
        'none' => t('No transform'),
        'upper' => t('Upper case'),
        'lower' => t('Lower case'),
        'ucfirst' => t('Capitalize first letter'),
        'ucwords' => t('Capitalize each word'),
      ),
      '#default_value' => $this->options['path_case'],
    );

    $form['transform_dash'] = array(
      '#type' => 'checkbox',
      '#title' => t('Transform spaces to dashes in URL'),
      '#default_value' => $this->options['transform_dash'],
    );

    if (!empty($this->definition['many to one'])) {
      $form['add_table'] = array(
        '#type' => 'checkbox',
        '#title' => t('Allow multiple arguments to work together.'),
        '#description' => t('If selected, multiple instances of this argument can work together, as though multiple terms were supplied to the same argument. This setting is not compatible with the "Reduce duplicates" setting.'),
        '#default_value' => !empty($this->options['add_table']),
      );

      $form['require_value'] = array(
        '#type' => 'checkbox',
        '#title' => t('Do not display items with no value in summary'),
        '#default_value' => !empty($this->options['require_value']),
      );
    }
  }

  /**
   * Build the summary query based on a formula
   */
  function summary_query() {
    if (empty($this->definition['many to one'])) {
      $this->ensure_my_table();
    }
    else {
      $this->table_alias = $this->helper->summary_join();
    }

    if (empty($this->options['glossary'])) {
      // Add the field.
      $this->base_alias = $this->name_alias = $this->query->add_field($this->table_alias, $this->real_field);
      $this->query->set_count_field($this->table_alias, $this->real_field);
    }
    else {
      // Add the field.
      $formula = $this->get_formula();
      $this->base_alias = $this->name_alias = $this->query->add_field(NULL, $formula, $this->field . '_truncated');
      $this->query->set_count_field(NULL, $formula, $this->field, $this->field . '_truncated');
    }

    return $this->summary_basics(FALSE);
  }

  /**
   * Get the formula for this argument.
   *
   * $this->ensure_my_table() MUST have been called prior to this.
   */
  function get_formula() {
    return "LEFT($this->table_alias.$this->real_field, " . intval($this->options['limit']) . ")";
  }

  /**
   * Build the query based upon the formula
   */
  function query() {
    $argument = $this->argument;
    if (!empty($this->options['transform_dash'])) {
      $argument = strtr($argument, '-', ' ');
    }

    if (!empty($this->definition['many to one'])) {
      if (!empty($this->options['glossary'])) {
        $this->helper->formula = TRUE;
      }
      $this->value = array($argument);
      $this->helper->ensure_my_table();
      $this->helper->add_filter();
      return;
    }

    $this->ensure_my_table();
    if (empty($this->options['glossary'])) {
      $field = "$this->table_alias.$this->real_field";
    }
    else {
      $field = $this->get_formula();
    }

    $this->query->add_where(0, "$field = '%s'", $argument);
  }

  function summary_argument($data) {
    $value = $this->case_transform($data->{$this->base_alias}, 'path_case');
    if (!empty($this->options['transform_dash'])) {
      $value = strtr($value, ' ', '-');
    }
    return $value;
  }

  function case_transform($string, $option) {
    switch ($this->options[$option]) {
      default:
        return $string;
      case 'upper':
        return strtoupper($string);
      case 'lower':
        return strtolower($string);
      case 'upper':
        return strtoupper($string);
      case 'ucfirst':
        return ucfirst($string);
      case 'ucwords':
        return ucwords($string);
    }
  }

  function title() {
    return check_plain($this->case_transform($this->argument, 'case'));
  }
}

/**
 * Basic argument handler for arguments that are numeric. Incorporates
 * break_phrase.
 */
class views_handler_argument_numeric extends views_handler_argument {
  function options(&$options) {
    parent::options($options);
    $options['break_phrase'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // allow + for or, , for and
    $form['break_phrase'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow multiple terms per argument.'),
      '#description' => t('If selected, users can enter multiple arguments in the form of 1+2+3 or 1,2,3.'),
      '#default_value' => !empty($this->options['break_phrase']),
    );

    $form['not'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude the argument'),
      '#description' => t('If selected, the numbers entered in the argument will be excluded rather than limiting the view.'),
      '#default_value' => !empty($this->options['not']),
    );
  }

  function title() {
    if (!$this->argument) {
      return !empty($this->definition['empty field name']) ? $this->definition['empty field name'] : t('Uncategorized');
    }

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
      $this->operator = 'or';
    }

    if (empty($this->value)) {
      return !empty($this->definition['empty field name']) ? $this->definition['empty field name'] : t('Uncategorized');
    }

    if ($this->value === array(-1)) {
      return !empty($this->definition['invalid input']) ? $this->definition['invalid input'] : t('Invalid input');
    }

    return implode($this->operator == 'or' ? ' + ' : ', ', $this->title_query());
  }

  /**
   * Override for specific title lookups.
   */
  function title_query() {
    return $this->value;
  }

  function query() {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
    }

    if (count($this->value) > 1) {
      $operator = empty($this->options['not']) ? 'IN' : 'NOT IN';
      $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));
      $this->query->add_where(0, "$this->table_alias.$this->real_field $operator ($placeholders)", $this->value);
    }
    else {
      $operator = empty($this->options['not']) ? '=' : '!=';
      $this->query->add_where(0, "$this->table_alias.$this->real_field $operator %d", $this->argument);
    }
  }
}

/**
 * An argument handler for use in fields that have a many to one relationship
 * with the table(s) to the left. This adds a bunch of options that are
 * reasonably common with this type of relationship.
 */
class views_handler_argument_many_to_one extends views_handler_argument {
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->helper = new views_many_to_one_helper($this);

    // Ensure defaults for these, during summaries and stuff:
    $this->operator = 'or';
    $this->value = array();
  }

  function options(&$options) {
    parent::options($options);
    $options['break_phrase'] = FALSE;
    $options['add_table'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // allow + for or, , for and
    if (!empty($this->definition['numeric'])) {
      $form['break_phrase'] = array(
        '#type' => 'checkbox',
        '#title' => t('Allow multiple terms per argument.'),
        '#description' => t('If selected, users can enter multiple arguments in the form of 1+2+3 (for OR) or 1,2,3 (for AND).'),
        '#default_value' => !empty($this->options['break_phrase']),
      );
    }

    $form['add_table'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow multiple arguments to work together.'),
      '#description' => t('If selected, multiple instances of this argument can work together, as though multiple terms were supplied to the same argument. This setting is not compatible with the "Reduce duplicates" setting.'),
      '#default_value' => !empty($this->options['add_table']),
    );

    $form['require_value'] = array(
      '#type' => 'checkbox',
      '#title' => t('Do not display items with no value in summary'),
      '#default_value' => !empty($this->options['require_value']),
    );

    $this->helper->options_form($form, $form_state);
  }

  /**
   * Override ensure_my_table so we can control how this joins in.
   * The operator actually has influence over joining.
   */
  function ensure_my_table() {
    $this->helper->ensure_my_table();
  }

  function query() {
    if (empty($this->argument)) {
      parent::ensure_my_table();
      $this->query->add_where(0, "$this->table_alias.$this->real_field IS NULL");
      return;
    }

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
      $this->operator = 'or';
    }

    $this->helper->add_filter();
  }

  function title() {
    if (!$this->argument) {
      return !empty($this->definition['empty field name']) ? $this->definition['empty field name'] : t('Uncategorized');
    }

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase($this->argument, $this);
    }
    else {
      $this->value = array($this->argument);
      $this->operator = 'or';
    }

    // @todo -- both of these should check definition for alternate keywords.

    if (empty($this->value)) {
      return !empty($this->definition['empty field name']) ? $this->definition['empty field name'] : t('Uncategorized');
    }

    if ($this->value === array(-1)) {
      return !empty($this->definition['invalid input']) ? $this->definition['invalid input'] : t('Invalid input');
    }

    return implode($this->operator == 'or' ? ' + ' : ', ', $this->title_query());
  }

  function get_join() {
    return drupal_clone(views_get_table_join($this->table, $this->query->base_table));
  }

  function summary_query() {
    $field = $this->table . '.' . $this->field;
    $join = $this->get_join();

    if (!empty($this->options['require_value'])) {
      $join->type = 'INNER';
    }

    if (empty($this->options['add_table']) || empty($this->view->many_to_one_tables[$field])) {
      $this->table_alias = $this->query->ensure_table($this->table, $this->relationship, $join);
    }
    else {
      $this->table_alias = $this->helper->summary_join();
    }

    // Add the field.
    $this->base_alias = $this->query->add_field($this->table_alias, $this->real_field);

    $this->summary_name_field();

    return $this->summary_basics();
  }

  function summary_argument($data) {
    $value = $data->{$this->base_alias};
    if (empty($value)) {
      $value = 0;
    }

    return $value;
  }

  /**
   * Override for specific title lookups.
   */
  function title_query() {
    return $this->value;
  }
}

/**
 * @}
 */
