<?php
// $Id: field.handlers.inc,v 1.10 2008/05/08 21:20:31 merlinofchaos Exp $
/**
 * @file
 * Views' field handlers.
 */
/**
 * @defgroup views_field_handlers Views' field handlers
 * @{
 * Handlers to tell Views how to build and display fields.
 *
 */

/**
 * Base field handler that has no options and renders an unformatted field.
 */
class views_handler_field extends views_handler {
  var $field_alias = 'unknown';

  /**
   * Construct a new field handler.
   */
  function construct() {
    $this->additional_fields = array();
    if (!empty($this->definition['additional fields'])) {
      $this->additional_fields = $this->definition['additional fields'];
    }
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
   * Provide a default label
   */
  function options(&$options) {
    parent::options($options);
    $options['label'] = $this->definition['title'];
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
    $themes[] = $hook;

    return $themes;
  }
}

/**
 * A handler to provide proper displays for dates.
 */
class views_handler_field_date extends views_handler_field {
  /**
   * Fill in default options.
   */
  function options(&$options) {
    parent::options($options);
    $options['date_format'] = 'small';
    $options['custom_date_format'] = '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $time = time();

    $form['date_format'] = array(
      '#type' => 'select',
      '#title' => t('Date format'),
      '#options' => array(
        'small' => format_date($time, 'small'),
        'medium' => format_date($time, 'medium'),
        'large' => format_date($time, 'large'),
        'custom' => t('Custom'),
        'time ago' => t('Time ago'),
      ),
      '#default_value' => isset($this->options['date_format']) ? $this->options['date_format'] : 'small',
    );
    $form['custom_date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom date format'),
      '#description' => t('If "Custom", see <a href="http://us.php.net/manual/en/function.date.php">the PHP docs</a> for date formats. If "Time ago" this is the the number of different units to display, which defaults to two.'),
      '#default_value' => isset($this->options['custom_date_format']) ? $this->options['custom_date_format'] : '',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-date-format' => array('custom', 'time ago')),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = $this->options['date_format'];
    if ($format == 'custom' || $format == 'time ago') {
      $custom_format = $this->options['custom_date_format'];
    }

    switch ($format) {
      case 'time ago':
        return $value ? t('%time ago', array('%time' => format_interval(time() - $value, is_numeric($custom_format) ? $custom_format : 2))) : theme('views_nodate');
      case 'custom':
        return $value ? format_date($value, $format, $custom_format) : theme('views_nodate');
      default:
        return $value ? format_date($value, $format) : theme('views_nodate');
    }
  }
}

/**
 * A handler to provide proper displays for dates.
 *
 * Allows for display of true/false, yes/no, on/off.
 */
class views_handler_field_boolean extends views_handler_field {
  function options(&$options) {
    parent::options($options);
    $options['type'] = 'yes-no';
    $options['not'] = !empty($this->definition['reverse']);
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Output format'),
      '#options' => array(
        'yes-no' => t('Yes/No'),
        'true-false' => t('True/False'),
        'on-off' => t('On/Off'),
      ),
      '#default_value' => $this->options['type'],
    );
    $form['not'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reverse'),
      '#description' => t('If checked, true will be displayed as false.'),
      '#default_value' => $this->options['not'],
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['not'])) {
      $value = !$value;
    }

    switch ($this->options['type']) {
      case 'yes-no':
      default:
        return $value ? t('Yes') : t('No');
      case 'true-false':
        return $value ? t('True') : t('False');
      case 'on-off':
        return $value ? t('On') : t('Off');
    }
  }
}

/**
 * A handler to run a field through check_markup, using a companion
 * format field.
 */
class views_handler_field_markup extends views_handler_field {
  /**
   * Constructor; calls to base object constructor.
   */
  function construct() {
    $this->format = $this->definition['format'];

    $this->additional_fields = array();
    if (!is_numeric($this->format)) {
      $this->additional_fields['format'] = $this->format;
    }
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = is_numeric($this->format) ? $this->format : $values->{$this->aliases['format']};
    return check_markup($value, $format, FALSE);
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
 * Field handler to provide simple renderer that turns a URL into a clickable link.
 */
class views_handler_field_url extends views_handler_field {
  /**
   * Provide link to the page being visited.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['display_as_link'] = array(
      '#title' => t('Display as link'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['display_as_link']),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['display_as_link'])) {
      return l(check_plain($value), $value, array('html' => TRUE));
    }
    else {
      return $value;
    }
  }
}

/**
 * Field handler to provide a list of items.
 *
 * The items are expected to be loaded by a child object during pre_render,
 * and 'my field' is expected to be the pointer to the items in the list.
 *
 * Items to render should be in a list in $this->items
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_prerender_list extends views_handler_field {
  function options(&$options) {
    parent::options($options);
    $options['type'] = 'separator';
    $options['separator'] = ', ';
    $options['empty'] = '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Display type'),
      '#options' => array(
        'ul' => t('Unordered list'),
        'ol' => t('Ordered list'),
        'separator' => t('Simple separator'),
      ),
      '#default_value' => $this->options['type'],
    );

    $form['separator'] = array(
      '#type' => 'textfield',
      '#title' => t('Separator'),
      '#default_value' => $this->options['separator'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('radio:options[type]' => array('separator')),
    );

    $form['empty'] = array(
      '#type' => 'textfield',
      '#title' => t('Empty list text'),
      '#default_value' => $this->options['empty'],
      '#description' => t('If the list is empty, you may enter text here that will be displayed.'),
    );
  }

  function render($values) {
    $field = $values->{$this->field_alias};
    if (!empty($this->items[$field])) {
      if ($this->options['type'] == 'separator') {
        return implode(check_plain($this->options['separator']), $this->items[$field]);
      }
      else {
        return theme('item_list', $this->items[$field], NULL, $this->options['type']);
      }
    }
    else if (!empty($this->options['empty'])) {
      return $this->options['empty'];
    }
  }
}

/**
 * Render a numeric value as a size.
 */
class views_handler_field_file_size extends views_handler_field {
  function render($values) {
    return format_size($values->{$this->field_alias});
  }
}

class views_handler_field_numeric extends views_handler_field {
  function options(&$options) {
    parent::options($options);
    $options['set_precision'] = FALSE;
    $options['precision'] = 0;
    $options['decimal'] = '.';
    $options['separator'] = ',';
    $options['prefix'] = '';
    $options['suffix'] = '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    if (!empty($this->definition['float'])) {
      $form['set_precision'] = array(
        '#type' => 'checkbox',
        '#title' => t('Round'),
        '#description' => t('If checked, the number will be rounded.'),
        '#default_value' => $this->options['set_precision'],
      );
      $form['precision'] = array(
        '#type' => 'textfield',
        '#title' => t('Precision'),
        '#default_value' => $this->options['precision'],
        '#description' => t('Specify how many digits to print after the decimal point.'),
        '#process' => array('views_process_dependency'),
        '#dependency' => array('edit-options-set-precision' => array(TRUE)),
        '#size' => 2,
      );
      $form['decimal'] = array(
        '#type' => 'textfield',
        '#title' => t('Decimal point'),
        '#default_value' => $this->options['decimal'],
        '#description' => t('What single character to use as a decimal point.'),
        '#size' => 2,
      );
    }
    $form['separator'] = array(
      '#type' => 'textfield',
      '#title' => t('Thousands separator'),
      '#default_value' => $this->options['separator'],
      '#description' => t('What single character to use as the thousands separator.'),
      '#size' => 2,
    );
    $form['prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Prefix'),
      '#default_value' => $this->options['prefix'],
      '#description' => t('Text to put before the number, such as currency symbol.'),
    );
    $form['suffix'] = array(
      '#type' => 'textfield',
      '#title' => t('Suffix'),
      '#default_value' => $this->options['suffix'],
      '#description' => t('Text to put after the number, such as currency symbol.'),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['set_precision'])) {
      $value = number_format($value, $this->options['precision'], $this->options['decimal'], $this->options['separator']);
    }
    else {
      $remainder = abs($value) - intval(abs($value));
      $value = number_format($value, 0, '', $this->options['separator']);
      if ($remainder) {
        $value .= $this->options['decimal'] . $remainder;
      }
    }
    return check_plain($this->options['prefix'] . $value . $this->options['suffix']);
  }
}

/**
 * @}
 */

