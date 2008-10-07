<?php
// $Id: relationship.handlers.inc,v 1.4 2008/05/17 19:46:03 merlinofchaos Exp $
/**
 * @file
 * Views' relationship handlers.
 */

/**
 * @defgroup views_relationship_handlers Views' relationship handlers
 * @{
 * Handlers to tell Views how to create alternate relationships.
 */

/**
 * Simple relationship handler that allows a new version of the primary table
 * to be linked in.
 */
class views_handler_relationship extends views_handler {
  /**
   * Init handler to let relationships live on tables other than
   * the table they operate on.
   */
  function init(&$view, $options) {
    parent::init($view, $options);
    if (isset($this->definition['relationship table'])) {
      $this->table = $this->definition['relationship table'];
    }
    if (isset($this->definition['relationship field'])) {
      $this->field = $this->definition['relationship field'];
    }
  }

  /**
   * Get this field's label.
   */
  function label() {
    if (!isset($this->options['label'])) {
      return $this->ui_name();
    }
    return $this->options['label'];
  }

  /**
   * Provide a default label
   */
  function options(&$options) {
    parent::options($options);
    $options['label'] = !empty($this->definition['label']) ? $this->definition['label'] : $this->definition['field'];
    $options['required'] = FALSE;
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#default_value' => isset($this->options['label']) ? $this->options['label'] : '',
      '#description' => t('The label for this relationship that will be displayed only administratively.'),
    );

    $form['required'] = array(
      '#type' => 'checkbox',
      '#title' => t('Require this relationship'),
      '#description' => t('If required, items that do not contain this relationship will not appear.'),
      '#default_value' => !empty($this->options['required']),
    );
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    // Figure out what base table this relationship brings to the party.
    $table_data = views_fetch_data($this->definition['base']);
    $base_field = $table_data['table']['base']['field'];

    $def = $this->definition;
    $def['table'] = $this->definition['base'];
    $def['field'] = $base_field;
    $def['left_table'] = $this->table;
    $def['left_field'] = $this->field;
    if (!empty($this->options['required'])) {
      $def['type'] = 'INNER';
    }

    if (!empty($def['join_handler']) && class_exists($def['join_handler'])) {
      $join = new $def['join_handler'];
    }
    else {
      $join = new views_join();
    }

    $join->definition = $def;
    $join->construct();

    $this->ensure_my_table();

    // use a short alias for this:
    $alias = $def['table'] . '_' . $def['left_table'];

    $this->alias = $this->query->add_relationship($alias, $join, $this->definition['base'], $this->relationship);
  }
}

/**
 * @}
 */
