<?php
// $Id: views.inc,v 1.5 2008/01/06 22:15:24 drewish Exp $

/**
 * Implementation of hook_views_tables()
 */
function image_views_tables() {
  $tables['image_node'] = array(
    'name' => 'node',
    'fields' => array(
      'nid' => array(
        'name' => t('Image: Display Image'),
        'handler' => array(
          'image_views_handler_image_img' => t('Image'),
          'image_views_handler_image_img_link' => t('Image with link'),
        ),
        'option' => array(
          '#type' => 'select',
          '#options' => 'image_views_handler_filter_image_size',
        ),
        'notafield' => true,
        'sortable' => false,
      ),
    ),
  );
  $tables['image_image'] = array(
    'name' => 'image',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid',
      ),
      'right' => array(
        'field' => 'nid',
      ),
    ),
    'fields' => array(
      'fid' => array(
        'name' => t('Image: File Id'),
        'sortable' => true,
        'help' => t('File Id which represents the file.'),
      ),
    ),
  );
  $tables['image_files'] = array(
    'name' => 'files',
    'join' => array(
      'left' => array(
        'table' => 'image_image',
        'field' => 'fid',
      ),
      'right' => array(
        'field' => 'fid'
      ),
    ),
    'fields' => array(
      'filename' => array(
        'name' => t('Image: File name'),
        'handler' => array(
          'views_handler_file_filename' => t('Plain'),
          'views_handler_file_filename_download' => t('With download link'),
        ),
        'sortable' => true,
        'addlfields' => array('filepath'),
        'option' => 'string',
        'help' => t('Display file name'),
      ),
      'filepath' => array(
        'name' => t('Image: File path'),
        'sortable' => false,
        'help' => t('Display Path to File.'),
      ),
      'filesize' => array(
        'name' => t('Image: File size'),
        'handler' => 'views_handler_file_size',
        'sortable' => true,
        'help' => t('Display the file size of the associated file.'),
      ),
      'filemime' => array(
        'name' => t('Image: Mime type'),
        'sortable' => true,
        'help' => t('This filter allows nodes to be filtered by mime type.'),
      ),
    ),
  );
  return $tables;
}

/**
 * Views handler for displaying the image.
 */
function image_views_handler_image_img($fieldinfo, $fielddata, $value, $data) {
  $node = node_load($data->nid);
  return image_display($node, $fielddata['options']);
}

/**
 * Views handler for displaying the image in a link to the the image node
 */
function image_views_handler_image_img_link($fieldinfo, $fielddata, $value, $data) {
  $node = node_load($data->nid);
  return l(image_display($node, $fielddata['options']), "node/{$node->nid}", array('html' => TRUE));
}

/**
 * Views - Generate a list of all the valid sizes that are available
 */
function image_views_handler_filter_image_size($op) {
  foreach (_image_get_sizes() as $key => $size) {
    $a[$key] = $size['label'];
  }
  return $a;
}

