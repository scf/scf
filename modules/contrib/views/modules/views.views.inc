<?php
// $Id: views.views.inc,v 1.2 2008/05/07 21:54:10 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers that aren't tied to any other module.
 */

/**
 * @defgroup views_views_module miscellaneous handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function views_views_data() {
  $data['views']['table']['group'] = t('Global');
  $data['views']['table']['join'] = array(
    '#global' => array(),
  );
  $data['views']['random'] = array(
    'title' => t('Random'),
    'help' => t('Randomize the display order.'),
    'sort' => array(
      'handler' => 'views_handler_sort_random',
    ),
  );

  $data['views']['null'] = array(
    'title' => t('Null'),
    'help' => t('Allow an argument to be ignored. The query will not be altered by this argument.'),
    'argument' => array(
      'handler' => 'views_handler_argument_null',
    ),
  );
  return $data;
}

/**
 * Handle a random sort.
 *
 * @ingroup views_sort_handlers
 */
class views_handler_sort_random extends views_handler_sort {
  function query() {
    global $db_type;
    switch ($db_type) {
      case 'mysql':
      case 'mysqli':
        $formula = 'RAND()';
        break;
      case 'pgsql':
        $formula = 'RANDOM()';
        break;
    }
    if (!empty($formula)) {
      $this->query->add_orderby(NULL, $formula, $this->options['order'], '_' . $this->field);
    }
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['order']['#access'] = FALSE;
  }
}

/**
 * Argument handler that ignores the argument.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_null extends views_handler_argument {
  /**
   * Override options_form() so that only the relevant options
   * are displayed to the user.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['must_not_be'] = array(
      '#type' => 'checkbox',
      '#title' => t('Fail basic validation if any argument is given'),
      '#default_value' => !empty($this->options['must_not_be']),
      '#description' => t('By checking this field, you can use this to make sure views with more arguments than necessary fail validation.'),
    );

    unset($form['wildcard']);
    unset($form['wildcard_substitution']);
  }

  /**
   * Override default_actions() to remove actions that don't
   * make sense for a null argument.
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

  function validate_argument_basic($arg) {
    if (!empty($this->options['must_not_be'])) {
      return !isset($arg);
    }

    return parent::validate_argument_basic($arg);
  }

  /**
   * Override the behavior of query() to prevent the query
   * from being changed in any way.
   */
  function query() {}
}
/**
 * @}
 */
