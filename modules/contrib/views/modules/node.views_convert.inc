<?php
//$Id: node.views_convert.inc,v 1.2 2008/03/20 15:40:18 merlinofchaos Exp $
/**
 * @file
 * Field conversion for fields handled by this module.
 */
/**
 * Implementation of hook_views_convert().
 *
 * Intervene to convert field values from the Views 1 format to the
 * Views 2 format. Intervene only if $view->add_item() won't produce
 * the right results, usually needed to set field options or values.
 */
function node_views_convert($display, $type, &$view, $field) {
  switch ($type) {
    case 'field':
      switch ($field['field']) {
        case 'title':
          return;
      }
      return;

    case 'filter':
      switch ($field['field']) {
        case 'type':
          $view->display['default']->display_options['filters'][$field['field']]['value'] = drupal_map_assoc($field['value']);
          return;

        case 'status':
        case 'promote':
        case 'sticky':
          $view->display['default']->display_options['filters'][$field['field']]['value'] = $field['value'];
          return;
      }
      return;

    case 'argument':
      switch ($field['type']) {
      }
      return;

    case 'sort':
      switch ($field['field']) {
      }
      break;

  }
  return;
}
