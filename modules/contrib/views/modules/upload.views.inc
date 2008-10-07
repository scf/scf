<?php
// $Id: upload.views.inc,v 1.7 2008/05/27 23:01:57 merlinofchaos Exp $
/**
 * @file
 *
 * Provide views data and handlers for upload tables that are not represented by
 * their own module.
 */

/**
 * @defgroup views_upload_module upload.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function upload_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // upload table

  $data['upload']['table']['group']  = t('Upload');

  $data['upload']['table']['join'] = array(
    'node' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'users' => array(
      'left_table' => 'node',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'files' => array(
      'left_field' => 'fid',
      'field' => 'fid',
    ),
  );

  $data['upload']['description'] = array(
    'title' => t('Description'),
    'help' => t('The description of the uploaded file.'),
    'field' => array(
      'handler' => 'views_handler_field_upload_description',
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

  $data['upload']['list'] = array(
    'title' => t('Listed'),
    'help' => t('Whether or not the file is marked to be listed.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['upload']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The weight, used for sorting.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  return $data;
}

function upload_views_data_alter(&$data) {
  $data['node']['upload_fid'] = array(
    'title' => t('Attached files'),
    'help' => t('All files attached to a node with upload.module.'),
    'real field' => 'vid',
    'field' => array(
      'handler' => 'views_handler_field_upload_fid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_upload_fid',
      'title' => t('Has attached files'),
      'help' => t('Only display items with attached files. This can cause duplicates if there are multiple attached files.'),
    ),
    'relationship' => array(
      'title' => t('Attached files'),
      'help' => t('Add a relationship to gain access to more file data for files uploaded by upload.module. Note that this relationship will cause duplicate nodes if there are multiple files attached to the node.'),
      'relationship table' => 'upload',
      'relationship field' => 'fid',
      'base' => 'files',
      'field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('Files'),
    ),
  );
}

/**
 * Field handler to provide a list of roles.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_upload_fid extends views_handler_field_prerender_list {
  function construct() {
    parent::construct();
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );

    $form['only_listed'] = array(
      '#title' => t('Only show "listed" file attachments'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['only_listed']),
    );
  }

  function pre_render($values) {
    $vids = array();
    $this->items = array();

    $data = array();
    foreach ($values as $result) {
      $vids[] = $result->{$this->field_alias};
    }

    if ($vids) {
      // Support "only listed files" option.
      $where = '';
      if (!empty($this->options['only_listed'])) {
        $where = " AND u.list <> 0";
      }
      $result = db_query("SELECT u.vid, u.fid, f.filepath, u.description FROM {upload} u LEFT JOIN {files} f ON f.fid = u.fid WHERE u.vid IN (" . implode(', ', $vids) . ")$where ORDER BY u.weight, u.description");
      while ($file = db_fetch_object($result)) {
        $this->items[$file->vid][$file->fid] = $this->render_link(check_plain($file->description), $file);
      }
    }
  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_file'])) {
      return l($data, file_create_url($values->filepath), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }
}

/**
 * Field handler to provide a list of roles.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_upload_description extends views_handler_field_prerender_list {
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($options['link_to_file'])) {
      $this->additional_fields['fid'] = 'fid';
    }
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );
  }

  function pre_render($values) {
    if (empty($this->options['link_to_file'])) {
      return;
    }

    $fids = array();
    $this->items = array();

    $data = array();
    foreach ($values as $result) {
      if ($result->{$this->aliases['fid']}) {
        $fids[] = $result->{$this->aliases['fid']};
      }
    }

    if ($fids) {
      // Support "only listed files" option.
      $result = db_query("SELECT f.fid, f.filepath FROM {files} f WHERE f.fid IN (" . implode(', ', $fids) . ")");
      while ($file = db_fetch_object($result)) {
        $this->items[$file->fid] = $file;
      }
    }
  }

  function render($values) {
    return $this->render_link($values->{$this->field_alias}, $values);
  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $value) {
    if (!empty($this->options['link_to_file']) && $value->{$this->aliases['fid']}) {
      $values = $this->items[$value->{$this->aliases['fid']}];
      return l($data, file_create_url($values->filepath), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }
}

/**
 * Filter by whether or not a node has attached files from the upload module
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_upload_fid extends views_handler_filter_boolean_operator {
  function construct() {
    parent::construct();
    $this->value_value = t('Has attached files');
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "(SELECT COUNT(*) FROM {upload} u WHERE u.vid = $this->table_alias.$this->real_field) " . (empty($this->value) ? '=' : '<>') . " 0");
  }
}

/**
 * @}
 */
