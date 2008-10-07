<?php
// $Id: system.views.inc,v 1.4 2008/05/02 19:47:57 merlinofchaos Exp $
/**
 * @file
 *
 * Provide views data and handlers for system tables that are not represented by
 * their own module.
 */

/**
 * @defgroup views_system_module system.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function system_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // files table

  $data['files']['table']['group']  = t('File');

  // Advertise this table as a possible base table
  $data['files']['table']['base'] = array(
    'field' => 'fid',
    'title' => t('File'),
    'help' => t("Files maintained by Drupal and various modules."),
  );

  // The files table does not inherently join to the node table,
  // but may things (such as upload.module) can add relationships
  // that allow file fields to be used.

  // For other base tables, explain how we join
  $data['files']['table']['join'] = array(
    'users' => array(
      // direct join to the users table via 'uid' field.
      'left_field' => 'uid',
      'field' => 'uid',
     ),
  );

  // fid
  $data['files']['fid'] = array(
    'title' => t('File ID'),
    'help' => t('The ID of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_file_fid',
      'name field' => 'filename', // the field to display in the summary.
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // filename
  $data['files']['filename'] = array(
    'title' => t('Name'),
    'help' => t('The name of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // filepath
  $data['files']['filepath'] = array(
    'title' => t('Path'),
    'help' => t('The path of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // filemime
  $data['files']['filemime'] = array(
    'title' => t('Mime type'),
    'help' => t('The mime type of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // filesize
  $data['files']['filesize'] = array(
    'title' => t('Size'),
    'help' => t('The size of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file_size',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // status
  $data['files']['status'] = array(
    'title' => t('Status'),
    'help' => t('The status of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file_status',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_file_status',
    ),
  );

  // timestamp field
  $data['files']['timestamp'] = array(
    'title' => t('Upload date'),
    'help' => t('The date the file was uploaded.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );


  return $data;
}

/**
 * Field handler to provide simple renderer that allows linking to a file.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_file extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($options['link_to_file'])) {
      $this->additional_fields['filepath'] = 'filepath';
    }
  }

  /**
   * Provide link to file option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );
  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_file'])) {
      return l($data, file_create_url($values->{$this->aliases['filepath']}), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

function _views_file_status($choice = NULL) {
  $status = array(
    FILE_STATUS_TEMPORARY => t('Temporary'),
    FILE_STATUS_PERMANENT => t('Permanent'),
  );

  if (isset($choice)) {
    return isset($status[$choice]) ? $status[$choice] : t('Unknown');
  }

  return $status;
}

/**
 * Field handler to translate a node type into its readable form.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_file_status extends views_handler_field {
  function render($values) {
    return _views_file_status($values->{$this->field_alias});
  }
}

/**
 * Filter by file status
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_file_status extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_options = _views_file_status();
    }
  }
}

/**
 * Argument handler to accept a file id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_file_fid extends views_handler_argument {
  /**
   * Override the behavior of title(). Get the title of the file.
   */
  function title() {
    $title = db_result(db_query(db_rewrite_sql("SELECT f.filename FROM {files} f WHERE f.fid = %d", $this->argument)));
    if (empty($title)) {
      return t('No title');
    }

    return check_plain($title);
  }
}

/**
 * @}
 */
