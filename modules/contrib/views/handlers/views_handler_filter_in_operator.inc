<?php
// $Id: views_handler_filter_in_operator.inc,v 1.4 2008/09/24 22:28:06 merlinofchaos Exp $
/**
 * Simple filter to handle matching of multiple options selectable via checkboxes
 *
 * Definition items:
 * - numeric: If set to true, this item will use numeric operators instead of string.
 *
 */
class views_handler_filter_in_operator extends views_handler_filter {
  var $value_form_type = 'checkboxes';

  function construct() {
    parent::construct();
    $this->value_title = t('Options');
    $this->value_options = NULL;
  }

  /**
   * Child classes should override this function to set the 'value options'.
   * This can use a guard to be used to reduce database hits as much as
   * possible.
   */
  function get_value_options() {
    $this->value_options = array(t('Yes'), t('No'));
  }

  function expose_options() {
    parent::expose_options();
    $this->options['expose']['reduce'] = FALSE;
  }

  function expose_form_right(&$form, &$form_state) {
    parent::expose_form_right($form, $form_state);
    $form['expose']['reduce'] = array(
      '#type' => 'checkbox',
      '#title' => t('Limit list to selected items'),
      '#description' => t('If checked, the selected items presented to the user will be the only ones selected here.'),
      '#default_value' => !empty($this->options['expose']['reduce']), // safety
    );
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'in';
    $options['value']['default'] = array();

    return $options;
  }

  /**
   * Provide inclusive/exclusive matching
   */
  function operator_options() {
    return array(
      'in' => t('Is one of'),
      'not in' => t('Is not one of'),
    );
  }

  function value_form(&$form, &$form_state) {
    $this->get_value_options();
    $options = $this->value_options;
    $default_value = (array) $this->value;

    if (!empty($form_state['exposed'])) {
      $identifier = $this->options['expose']['identifier'];

      if (!empty($this->options['expose']['reduce'])) {
        $options = $this->reduce_value_options();

        if (empty($this->options['expose']['single']) && !empty($this->options['expose']['optional'])) {
          $default_value = array();
        }
      }

      if (!empty($this->options['expose']['single'])) {
        if (!empty($this->options['expose']['optional']) && (empty($default_value) || !empty($this->options['expose']['reduce']))) {
          $default_value = 'All';
        }
        else if (empty($default_value)) {
          $keys = array_keys($options);
          $default_value = array_shift($keys);
        }
        else {
          $copy = $default_value;
          $default_value = array_shift($copy);
        }
      }
    }

    $form['value'] = array(
      '#type' => $this->value_form_type,
      '#title' => $this->value_title,
      '#options' => $options,
      '#default_value' => $default_value,
      // These are only valid for 'select' type, but do no harm to checkboxes.
      '#multiple' => TRUE,
      '#size' => count($options) > 8 ? 8 : count($options),
    );

    if (!empty($form_state['exposed']) && !isset($form_state['input'][$identifier])) {
      $form_state['input'][$identifier] = $default_value;
    }

  }

  /**
   * When using exposed filters, we may be required to reduce the set.
   */
  function reduce_value_options($input = NULL) {
    if (!isset($input)) {
      $input = $this->value_options;
    }

    // Because options may be an array of strings, or an array of mixed arrays
    // and strings (optgroups) or an array of objects, we have to
    // step through and handle each one individually.
    $options = array();
    foreach ($input as $id => $option) {
      if (is_array($option)) {
        $options[$id] = $this->reduce_value_options($option);
        continue;
      }
      else if (is_object($option)) {
        $keys = array_keys($option->option);
        $key = array_shift($keys);
        if (isset($this->value[$key])) {
          $options[$id] = $option;
        }
      }
      else if (isset($this->value[$id])) {
        $options[$id] = $option;
      }
    }
    return $options;
  }

  function accept_exposed_input($input) {
    // A very special override because the All state for this type of
    // filter could have a default:
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    // If this is single and optional, this says that yes this filter will
    // participate, but using the default settings.
    if (!empty($this->options['expose']['single']) && !empty($this->options['expose']['optional'])) {
      $identifier = $this->options['expose']['identifier'];
      if ($input[$identifier] == 'All') {
        return TRUE;
      }
    }

    return parent::accept_exposed_input($input);
  }

  function value_submit($form, &$form_state) {
    // Drupal's FAPI system automatically puts '0' in for any checkbox that
    // was not set, and the key to the checkbox if it is set.
    // Unfortunately, this means that if the key to that checkbox is 0,
    // we are unable to tell if that checkbox was set or not.

    // Luckily, the '#value' on the checkboxes form actually contains
    // *only* a list of checkboxes that were set, and we can use that
    // instead.

    $form_state['values']['options']['value'] = $form['value']['#value'];
//    $form_state['values']['options']['value'] = array_filter($form_state['values']['options']['value']);
  }

  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    $this->get_value_options();

    if (!is_array($this->value)) {
      return;
    }

    if (count($this->value) == 1) {
      // If there is only one, show it as an =.
      $keys = $this->value;
      $key = array_shift($keys);

      if (isset($this->value_options[$key])) {
        $value = check_plain($this->value_options[$key]);
      }
      else {
        $value = t('Unknown');
      }

      return (substr($this->operator, 0, 3) != 'not' ? '=' : '<>') . ' ' . $value;
    }
    $output = '';
    foreach ($this->value as $value) {
      if ($output !== '') {
        $output .= ', ';
      }
      if (strlen($output) > 8) {
        $output .= '...';
        break;
      }
      $output .= check_plain($this->value_options[$value]);
    }

    return check_plain($this->operator) . ' ' . $output;
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->ensure_my_table();
    $placeholder = !empty($this->definition['numeric']) ? '%d' : "'%s'";

    $replace = array_fill(0, sizeof($this->value), $placeholder);
    $in = ' (' . implode(", ", $replace) . ')';

    // We use array_values() because the checkboxes keep keys and that can cause
    // array addition problems.
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . $this->operator . $in, array_values($this->value));
  }
}
