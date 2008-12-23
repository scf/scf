<?php
// $Id: views.views.inc,v 1.5 2008/09/03 19:21:29 merlinofchaos Exp $
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
 * @}
 */
