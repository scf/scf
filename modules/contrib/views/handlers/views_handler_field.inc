<?php
// $Id: views_handler_field.inc,v 1.5 2008/10/16 19:38:52 merlinofchaos Exp $
/**
 * @defgroup views_field_handlers Views' field handlers
 * @{
 * Handlers to tell Views how to build and display fields.
 *
 */

/**
 * Base field handler that has no options and renders an unformatted field.
 *
 * Definition terms:
 * - additional fields: An array of fields that should be added to the query
 *                      for some purpose. The array is in the form of:
 *                      array('identifier' => array('table' => tablename,
 *                      'field' => fieldname); as many fields as are necessary
 *                      may be in this array.
 * - click sortable: If TRUE, this field may be click sorted.
 */
class views_handler_field extends views_handler {
  var $field_alias = 'unknown';
  var $aliases = array();

  /**
   * Construct a new field handler.
   */
  function construct() {
    parent::construct();

    $this->additional_fields = array();
    if (!empty($this->definition['additional fields'])) {
      $this->additional_fields = $this->definition['additional fields'];
    }

    if (!isset($this->options['exclude'])) {
      $this->options['exclude'] = '';
    }
  }

  function init(&$view, $options) {
    parent::init($view, $options);

    $this->options += array(
      'exclude' => FALSE,
    );
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Add the field.
    $this->field_alias = $this->query->add_field($this->table_alias, $this->real_field);

    $this->add_additional_fields();
  }

  /**
   * Add 'additional' fields to the query.
   *
   * @param $fields
   * An array of fields. The key is an identifier used to later find the
   * field alias used. The value is either a string in which case it's
   * assumed to be a field on this handler's table; or it's an array in the
   * form of
   * @code array('table' => $tablename, 'field' => $fieldname) @endcode
   */
  function add_additional_fields($fields = NULL) {
    if (!isset($fields)) {
      // notice check
      if (empty($this->additional_fields)) {
        return;
      }
      $fields = $this->additional_fields;
    }
    if (!empty($fields) && is_array($fields)) {
      foreach ($fields as $identifier => $info) {
        if (is_array($info)) {
          if (isset($info['table'])) {
            $table_alias = $this->query->ensure_table($info['table'], $this->relationship);
          }
          else {
            $table_alias = $this->table_alias;
          }
          $this->aliases[$identifier] = $this->query->add_field($table_alias, $info['field']);
        }
        else {
          $this->aliases[$info] = $this->query->add_field($this->table_alias, $info);
        }
      }
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $this->query->add_orderby($this->table, $this->field, $order, $this->field_alias);
  }

  /**
   * Determine if this field is click sortable.
   */
  function click_sortable() {
    return !empty($this->definition['click sortable']);
  }

  /**
   * Get this field's label.
   */
  function label() {
    if (!isset($this->options['label'])) {
      return '';
    }
    return $this->options['label'];
  }

  /**
   * Return DIV or SPAN based upon the field's element type.
   */
  function element_type() {
    if (isset($this->definition['element type'])) {
      return $this->definition['element type'];
    }

    return 'span';
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['label'] = array('default' => $this->definition['title'], 'translatable' => TRUE);

    return $options;
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
      '#description' => t('The label for this field that will be displayed to end users if the style requires it.'),
    );
    $form['exclude'] = array(
      '#type' => 'checkbox',
      '#title' => t('Exclude from display'),
      '#default_value' => $this->options['exclude'],
      '#description' => t('Check this box to not display this field, but still load it in the view.  Use this option to not show a grouping field in each record, or when doing advanced theming.'),
    );
  }

  /**
   * Provide extra data to the administration form
   */
  function admin_summary() {
    return $this->label();
  }

  /**
   * Run before any fields are rendered.
   *
   * This gives the handlers some time to set up before any handler has
   * been rendered.
   *
   * @param $values
   *   An array of all objects returned from the query.
   */
  function pre_render($values) { }

  /**
   * Render the field.
   *
   * @param $values
   *   The values retrieved from the database.
   */
  function render($values) {
    $value = $values->{$this->field_alias};
    return check_plain($value);
  }

  /**
   * Call out to the theme() function, which probably just calls render() but
   * allows sites to override output fairly easily.
   */
  function theme($values) {
    return theme($this->theme_functions(), $this->view, $this, $values);
  }

  function theme_functions() {
    $themes = array();
    $hook = 'views_view_field';

    $display = $this->view->display[$this->view->current_display];

    if (!empty($display)) {
      $themes[] = $hook . '__' . $this->view->name  . '__' . $display->id . '__' . $this->options['id'];
      $themes[] = $hook . '__' . $this->view->name  . '__' . $display->id;
      $themes[] = $hook . '__' . $display->id . '__' . $this->options['id'];
      $themes[] = $hook . '__' . $display->id;
      if ($display->id != $display->display_plugin) {
        $themes[] = $hook . '__' . $this->view->name  . '__' . $display->display_plugin . '__' . $this->options['id'];
        $themes[] = $hook . '__' . $this->view->name  . '__' . $display->display_plugin;
        $themes[] = $hook . '__' . $display->display_plugin . '__' . $this->options['id'];
        $themes[] = $hook . '__' . $display->display_plugin;
      }
    }
    $themes[] = $hook . '__' . $this->view->name . '__' . $this->options['id'];
    $themes[] = $hook . '__' . $this->view->name;
    $themes[] = $hook . '__' . $this->options['id'];
    $themes[] = $hook;

    return $themes;
  }
}

/**
 * A special handler to take the place of missing or broken handlers.
 */
class views_handler_field_broken extends views_handler_field {
  function ui_name() {
    return t('Broken/missing handler');
  }

  function ensure_my_table() { /* No table to ensure! */ }
  function query() { /* No query to run */ }
  function options_form(&$form, &$form_state) {
    $form['markup'] = array(
      '#prefix' => '<div class="form-item description">',
      '#value' => t('The handler for this item is broken or missing and cannot be used. If a module provided the handler and was disabled, re-enabling the module may restore it. Otherwise, you should probably delete this item.'),
    );
  }

  /**
   * Determine if the handler is considered 'broken'
   */
  function broken() { return TRUE; }
}

/**
 * Render a numeric value as a size.
 */
class views_handler_field_file_size extends views_handler_field {
  function render($values) {
    return format_size($values->{$this->field_alias});
  }
}

/**
 * A handler to run a field through simple XSS filtering
 */
class views_handler_field_xss extends views_handler_field {
  function render($values) {
    $value = $values->{$this->field_alias};
    return filter_xss($value);
  }
}

/**
 * @}
 */

