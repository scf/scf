<?php
/**
 * UNUSED: this makes a CCK version of antibody, but it doesn't fully
 * work...
 *
 * NOTE: this requires the CCK content_copy module
 *
 */
function antibody_define_node_type () {
  $content['type']  = array(
    'name' => 'Antibody',
    'type' => 'antibody',
    'description' => 'Antibody description',
    'has_title' => TRUE,
    'has_body' => TRUE,
    'title_label' => 'Title',
    'body_label' => 'Body',
    'min_word_count' => '0',
    'help' => '',
    'node_options' => 
    array(
      'status' => TRUE,
      'promote' => FALSE,
      'sticky' => FALSE,
      'revision' => FALSE,
      ),
    'upload' => TRUE,
    'print_display' => '1',
    'orig_type' => '',
    'module' => 'node',
    'custom' => FALSE,
    'modified' => FALSE,
    'locked' => FALSE,
    'content_profile' => FALSE,
    'comment' => '2',
    'comment_default_mode' => '4',
    'comment_default_order' => '1',
    'comment_default_per_page' => '50',
    'comment_controls' => '3',
    'comment_anonymous' => 0,
    'comment_subject_field' => '1',
    'comment_preview' => '1',
    'comment_form_location' => '0',
    'print_display_comment' => '0',
    );
  $content['fields']  = array(
    array(
      'widget_type' => 'nodereference_autocomplete',
      'label' => 'Gene',
      'weight' => '',
      'description' => '',
      'default_value_widget' => 
      array(
        'field_gene' => 
        array(
          array(
            'nid' => NULL,
            ),
          ),
        ),
      'default_value_php' => '',
      'group' => FALSE,
      'required' => '0',
      'multiple' => '1',
      'previous_field' => '',
      'referenceable_types' => array ('gene' => 'gene'),
      'field_name' => 'field_gene',
      'type' => 'nodereference',
      'module' => 'nodereference',
      'widget_module' => 'nodereference',
      'columns' => 
      array(
        'nid' => 
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => FALSE,
          ),
        ),
      'default_value' => 
      array(
        array(
          'nid' => NULL,
          ),
        ),
      ),
    array(
      'widget_type' => 'text_textfield',
      'label' => 'Source',
      'weight' => '',
      'rows' => 1,
      'description' => 'From what lab can this Antibody be obtained?',
      'default_value_widget' => 
      array(
        'field_source' => 
        array(
          array(
            array(
              'value' => '',
              ),
            'value' => '',
            ),
          ),
        ),
      'default_value_php' => '',
      'group' => FALSE,
      'required' => '0',
      'multiple' => '1',
      'previous_field' => '',
      'text_processing' => '0',
      'max_length' => '',
      'allowed_values' => '',
      'allowed_values_php' => '',
      'field_name' => 'field_source',
      'type' => 'text',
      'module' => 'text',
      'widget_module' => 'text',
      'columns' => 
      array(
        'value' => 
        array(
          'type' => 'text',
          'size' => 'big',
          'not null' => FALSE,
          'sortable' => TRUE
        )
      )
    )
  );
  $macro = "\$content = " . var_export($content, TRUE) . ";\n";
  $form_state['values'] = array(
    'type_name' => '<create>',
    'macro' => $macro,
    'op' => t('Submit')
  );
  drupal_load('module', 'content_copy');
  drupal_execute('content_copy_import_form', $form_state);
}
