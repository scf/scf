<?php
// $Id: rdf.admin.inc,v 1.37 2009/03/23 12:32:08 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// RDF settings form

function rdf_admin_settings() {
  $form = array();

  // Formats
  $form['formats'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Formats'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
    '#description' => rdf_help('admin/settings/rdf#formats'),
  );

  $formats = array();
  foreach (rdf_get_formats('names') as $key => $value) {
    $formats[$key] = '';
  }
  $form['formats']['rdf_format'] = array('#type' => 'radios', '#options' => $formats, '#default_value' => RDF_FORMAT);

  // RDFa
  $form['rdfa'] = array('#type' => 'fieldset', '#title' => t('RDFa'), '#collapsible' => TRUE, '#collapsed' => TRUE);
  $form['rdfa']['rdf_rdfa_doctype'] = array(
    '#type'          => 'radios',
    '#title'         => t('Output an XHTML+RDFa DOCTYPE'),
    '#default_value' => (int)variable_get('rdf_rdfa_doctype', FALSE),
    '#options'       => array(FALSE => t('Disabled'), TRUE => t('Enabled')),
    '#description'   => t('Whether to override the current theme\'s output and replace any existing <a href="http://en.wikipedia.org/wiki/Document_Type_Declaration">Document Type Declaration</a> with an <tt>XHTML+RDFa 1.0</tt> DOCTYPE that enables the use of <a href="http://en.wikipedia.org/wiki/RDFa">RDFa</a> metadata in Drupal content. Please note that this feature may be incompatible with <a href="@admin-performance">aggressive caching</a>. <em>This is an experimental feature, please use caution and <a href="http://drupal.org/node/add/project-issue/rdf">report any problems</a></em>.</a>.', array('@admin-performance' => url('admin/settings/performance'))),
  );
  $form['rdfa']['rdf_rdfa_prefixes'] = array(
    '#type'          => 'checkboxes',
    '#title'         => t('Enabled RDFa namespace prefixes'),
    '#default_value' => array_keys(rdf_get_namespaces('rdfa')),
    '#options'       => array_diff_key(array_combine($ns = array_keys(rdf_get_namespaces()), $ns), array('_' => '')),
    '#description'   => t('Select which <a href="@admin-rdf-namespaces">namespace prefixes</a> will be available on RDFa-enabled pages.', array('@admin-rdf-namespaces' => url('admin/settings/rdf/namespaces'))),
  );
  if (!variable_get('rdf_rdfa_doctype', FALSE)) {
    $form['rdfa']['rdf_rdfa_prefixes']['#attributes']['disabled'] = 'disabled';
    $form['rdfa']['rdf_rdfa_disabled'] = array('#type' => 'hidden', '#value' => '1');
  }

  // RDF Schema
  $form['rdfs'] = array('#type' => 'fieldset', '#title' => t('RDF Schema'), '#collapsible' => TRUE, '#collapsed' => TRUE);
  $form['rdfs']['rdf_schema_uri'] = array(
    '#type'          => 'textfield', 
    '#title'         => t('RDFS base URI'),
    '#default_value' => RDF_SCHEMA_URI,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#description'   => t(''), // TODO
  );

  // Maintenance
  $form['maintenance'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Maintenance'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
    '#description' => rdf_help('admin/settings/rdf#maintenance'),
    '#weight'      => 70,
  );
  $form['maintenance']['rdf_db_prevent_duplicates'] = array(
    '#type'          => 'radios',
    '#title'         => t('Prevent duplicate statement insertion'),
    '#default_value' => (int)variable_get('rdf_db_prevent_duplicates', FALSE),
    '#options'       => array(FALSE => t('Disabled'), TRUE => t('Enabled')),
    '#description'   => t('Whether to always perform a check to see if an RDF statement that is about to be inserted into a local RDF repository already exists in said repository, in which case the insertion request can be silently ignored. Note that this involves a slight performance hit, and it may be a good idea to disable this option e.g. when importing large RDF data sets.'),
  );
  $form['maintenance']['rdf_db_merge_duplicates'] = array(
    '#type'          => 'radios',
    '#title'         => t('Merge duplicate statements on cron runs'),
    '#default_value' => (int)variable_get('rdf_db_merge_duplicates', FALSE),
    '#options'       => array(FALSE => t('Disabled'), TRUE => t('Enabled')),
    '#description'   => t('Whether to check for and remove superfluous identical RDF statements in each local RDF repository during cron runs, ensuring each repository contains only unique statements. It may be useful to enable this after importing large RDF data sets if the real-time duplicate prevention option was disabled during the import.'),
  );
  $form['maintenance']['rdf_db_purge_resources'] = array(
    '#type'          => 'radios',
    '#title'         => t('Purge unused resource URIs on cron runs'),
    '#default_value' => (int)variable_get('rdf_db_purge_resources', FALSE),
    '#options'       => array(FALSE => t('Disabled'), TRUE => t('Enabled')),
    '#description'   => t('Whether to perform periodic purges of unused resource URIs on cron runs, decreasing the size of the <code>rdf_resources</code> table. It may be useful to enable this when dealing with transient RDF data sets that contain large numbers of URIs that are not likely to be seen again once the data sets expire or are removed.'),
  );

  return array_merge_recursive(system_settings_form($form), array('#theme' => 'rdf_admin_settings', 'buttons' => array('#weight' => 99)));
}

function rdf_admin_settings_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!empty($rdf_rdfa_disabled)) {
    // Ensure we don't muck up the default value when disabled:
    unset($form_state['values']['rdf_rdfa_prefixes']);
    unset($form_state['values']['rdf_rdfa_disabled']);
  }
  else {
    $rdf_rdfa_prefixes = array_filter($rdf_rdfa_prefixes, 'is_string');
  }
}

function theme_rdf_admin_settings($form) {
  $head = array(t('Default'), t('Name'), t('MIME type'), t('Readable'), t('Writable'));
  $rows = array();
  foreach (rdf_get_formats() as $format) {
    $rows[] = array(
      drupal_render($form['formats']['rdf_format'][$format->name]),
      !empty($format->link) ? l($format->title, $format->link) : $format->title,
      $format->mime_type,
      !empty($format->unserialize) ? t('Yes') : t('No'),
      !empty($format->serialize) ? t('Yes') : t('No')
    );
  }

  $form['formats']['#value'] = theme('table', $head, $rows, array('class' => 'formats'));
  unset($form['formats']['rdf_format']);

  return drupal_render($form);
}

//////////////////////////////////////////////////////////////////////////////
// RDF data management

function rdf_admin_data() {
  $subject   = isset($_GET['s']) ? urldecode($_GET['s']) : url(NULL, array('absolute' => TRUE));
  $predicate = isset($_GET['p']) ? urldecode($_GET['p']) : '';
  $object    = isset($_GET['o']) ? urldecode($_GET['o']) : '';
  $context   = isset($_GET['g']) ? urldecode($_GET['g']) : '';

  $form = array();
  $form['query'] = array('#type' => 'fieldset', '#title' => t('Query'), '#prefix' => '<div class="container-inline">', '#suffix' => '</div>');
  $form['query']['subject'] = array(
    '#type'          => 'textfield',
    '#title'         => '',
    '#default_value' => $subject,
    '#maxlength'     => 255,
    '#size'          => 20,
    '#autocomplete_path' => 'rdf/autocomplete/resource',
  );
  $form['query']['predicate'] = array(
    '#type'          => 'textfield',
    '#title'         => '',
    '#default_value' => $predicate,
    '#maxlength'     => 255,
    '#size'          => 20,
    '#autocomplete_path' => 'rdf/autocomplete/property',
  );
  $form['query']['object'] = array(
    '#type'          => 'textfield',
    '#title'         => '',
    '#default_value' => $object,
    '#maxlength'     => 255,
    '#size'          => 20,
  );
  $form['query']['submit'] = array('#type' => 'submit', '#value' => t('Find'), '#submit' => array('rdf_admin_data_submit'));

  $form['advanced'] = array('#type' => 'fieldset', '#title' => t('Advanced options'), '#collapsible' => TRUE, '#collapsed' => empty($context));
  $form['advanced']['context'] = array(
    '#type'          => 'select',
    '#title'         => 'Context',
    '#default_value' => $context,
    '#options'       => array_merge(array('' => t('(all)')), array_combine($contexts = rdf_get_contexts(), $contexts)),
  );

  return $form;
}

function rdf_admin_data_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  $form_state['redirect'] = array('admin/content/rdf', drupal_query_string_encode(array('g' => $context, 's' => $subject, 'p' => $predicate, 'o' => $object)));
}

function theme_rdf_admin_data($form) {
  foreach (array('query' => array('subject', 'predicate', 'object'), 'advanced' => array('context')) as $fieldset => $fields) {
    foreach ($fields as $field) {
      $$field = !empty($form[$fieldset][$field]['#value']) ? trim($form[$fieldset][$field]['#value']) : NULL;
    }
  }
  $data = rdf_normalize(rdf_query($subject, $predicate, $object, $context ? array('context' => $context) : array()));

  $output  = drupal_render($form['query']);
  $output .= drupal_render($form['advanced']);
  $output .= theme('rdf_triple_table', $data, array('link' => 'theme_rdf_admin_link'));
  $output .= drupal_render($form);
  return $output;
}

function theme_rdf_admin_link($title, $uri, array $attributes = array()) {
  return l($title, 'admin/content/rdf', array('query' => drupal_query_string_encode(array('s' => $uri))));
}

function rdf_db_admin_data_form(&$form_state, $edit = array('subject' => '', 'predicate' => '', 'object' => '', 'repository' => 'local')) {
  $edit = (object)$edit;
  $form = array();

  $form['repository'] = array(
    '#type'          => 'select',
    '#title'         => t('Repository'),
    '#default_value' => $edit->repository,
    '#options'       => rdf_get_repositories('names', array('persistent' => TRUE, 'mutable' => TRUE)),
    '#description'   => t('Select which RDF repository to store new statement in.'),
  );
  $form['subject'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Subject'),
    '#default_value' => $edit->subject,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#description'   => t('The subject of the statement: the entity or resource that the statement is about, i.e. the who or what. This should be given as either a <a href="http://en.wikipedia.org/wiki/URI" title="Uniform Resource Identifier">URI</a> or a <a href="http://en.wikipedia.org/wiki/CURIE" title="Compact URI">CURIE</a>.'),
    '#autocomplete_path' => 'rdf/autocomplete/resource',
  );
  $form['predicate'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Predicate'),
    '#default_value' => $edit->predicate,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#description'   => t('The predicate of the statement: the relation between subject and object, i.e. the property name or verb. This should be given as either a <a href="http://en.wikipedia.org/wiki/URI" title="Uniform Resource Identifier">URI</a> or a <a href="http://en.wikipedia.org/wiki/CURIE" title="Compact URI">CURIE</a>.'),
    '#autocomplete_path' => 'rdf/autocomplete/property',
  );
  $form['object'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Object'),
    '#default_value' => $edit->object,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#description'   => t('The object of the statement: the entity or resource that relates to the subject as described by the predicate; i.e. the property value. This should be given as either a <a href="http://en.wikipedia.org/wiki/URI" title="Uniform Resource Identifier">URI</a>, a <a href="http://en.wikipedia.org/wiki/CURIE" title="Compact URI">CURIE</a>, or a literal.'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Add statement'));
  return $form;
}

function rdf_db_admin_data_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!rdf_is_valid_uri($subject) && !rdf_is_valid_curie($subject)) {
    form_set_error('subject', t('Subject value is not a valid URI or CURIE.'));
  }

  if (!rdf_is_valid_uri($predicate) && !rdf_is_valid_curie($predicate)) {
    form_set_error('predicate', t('Predicate value is not a valid URI or CURIE.'));
  }

  // TODO: relax this once we have a specific input box for literals.
  if (!rdf_is_valid_uri($object) && !rdf_is_valid_curie($object)) {
    //form_set_error('object', t('Object value is not a valid URI or CURIE.'));
  }
}

function rdf_db_admin_data_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  // TODO: properly sanitize/expand/convert subject, predicate and object.
  $subject   = rdf_is_valid_curie($subject)   ? rdf_qname_to_uriref($subject)   : rdf_uri($subject);
  $predicate = rdf_is_valid_curie($predicate) ? rdf_qname_to_uriref($predicate) : rdf_uri($predicate);
  $object    = rdf_is_valid_uri($object)      ? rdf_uri($object) : $object;

  if (rdf_insert($subject, $predicate, $object, array('repository' => $repository))) {
    drupal_set_message(t('The statement has been added.'));
  }
  else {
    drupal_set_message(t('The statement was not added due to an error reported by the repository.'), 'error');
  }

  $form_state['redirect'] = array('admin/content/rdf', drupal_query_string_encode(array('s' => (string)$subject)));
}

//////////////////////////////////////////////////////////////////////////////
// RDF mappings management

function theme_rdf_mapping($mapping) {
  return empty($mapping) ? '-' : l(rdf_uri_to_qname($mapping, FALSE, $mapping), $mapping, array('attributes' => array('title' => $mapping)));
}

function rdf_admin_mappings() {
  // TODO: this should get refactored for extensibility, at some point.

  $header = array(t('Title'), t('Class/property'), t('RDF mapping'), array('data' => t('Operations'), 'colspan' => '2'));
  $rows = array();

  // Content types
  if (module_exists('node')) {
    $rows[] = array(array('data' => t('Content types'), 'colspan' => 5, 'class' => 'category'));
    $mapping = NULL;
    foreach (node_get_types('names') as $node_type => $node_type_title) {
      $mapping = theme_rdf_mapping(variable_get('rdf_schema_class_' . $node_type, ''));
      $link    = l(t('edit'), 'admin/content/node-type/' . $node_type, array('query' => 'destination=' . drupal_urlencode($_GET['q'])));
      $rows[]  = array($node_type_title, $node_type, $mapping, $link, '');
    }
    if (is_null($mapping)) {
      $rows[] = array(array('data' => t('No content types defined.'), 'colspan' => 5));
    }
  }

  // Content fields
  $rows[] = array(array('data' => t('Content fields'), 'colspan' => 5, 'class' => 'category'));
  if (module_exists('node')) {
    // Show the core field mappings that are currently hardcoded in the RSS output:
    $rows[] = array(t('Title'),       'title',    theme_rdf_mapping('dc:title'), '-', '');
    $rows[] = array(t('Body'),        'body',     theme_rdf_mapping('dc:description'), '-', '');
    $rows[] = array(t('Language'),    'language', theme_rdf_mapping('dc:language'), '-', '');
    $rows[] = array(t('Authored by'), 'name',     theme_rdf_mapping('dc:creator'), '-', '');
    $rows[] = array(t('Authored on'), 'created',  theme_rdf_mapping('dc:date'), '-', '');
  }
  if (module_exists('content')) {
    $mapping = NULL;
    foreach (content_fields() as $field_name => $field_info) {
      $link = l(t('edit'), 'admin/content/types/fields', array('fragment' => $field_name)); // FIXME: link
      if (($mapping = variable_get('rdf_schema_property_content_' . $field_name, ''))) {
        $mapping = theme_rdf_mapping($mapping);
        $rows[] = array($field_info['widget']['label'], $field_name, $mapping, $link, ''); 
      }
      // HACK: support CCK Date fields that have both a start and an end date:
      else if (variable_get('rdf_schema_property_content_' . $field_name . '[from]', '')) {
        $mappings = array();
        if (($mapping = variable_get('rdf_schema_property_content_' . $field_name . '[from]', ''))) {
          $mappings[] = theme_rdf_mapping($mapping);
        }
        if (($mapping = variable_get('rdf_schema_property_content_' . $field_name . '[to]', ''))) {
          $mappings[] = theme_rdf_mapping($mapping);
        }
        $rows[] = array($field_info['widget']['label'], $field_name, implode(', ', $mappings), $link, ''); 
      }
      else {
        $rows[] = array($field_info['widget']['label'], $field_name, '-', $link, ''); 
      }
    }
    if (is_null($mapping)) {
      $rows[] = array(array('data' => t('No content fields defined.'), 'colspan' => 5));
    }
  }

  // Taxonomy vocabularies
  if (module_exists('taxonomy')) {
    $rows[] = array(array('data' => t('Taxonomy vocabularies'), 'colspan' => 5, 'class' => 'category'));
    $mapping = NULL;
    foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
      $mapping = theme_rdf_mapping(variable_get('rdf_schema_property_vocabulary_' . $vid, ''));
      $link    = l(t('edit'), 'admin/content/taxonomy/edit/vocabulary/' . $vid, array('query' => 'destination=' . drupal_urlencode($_GET['q'])));
      $rows[]  = array($vocabulary->name, '-', $mapping, $link, '');
    }
    if (is_null($mapping)) {
      $rows[] = array(array('data' => t('No vocabularies defined.'), 'colspan' => 5));
    }
  }

  // Profile fields
  if (module_exists('profile')) {
    $rows[] = array(array('data' => t('Profile fields'), 'colspan' => 5, 'class' => 'category'));
    $result = db_query('SELECT fid, title, name FROM {profile_fields} ORDER BY category, weight');
    $mapping = NULL;
    while ($field = db_fetch_object($result)) {
      $mapping = theme_rdf_mapping(variable_get('rdf_schema_property_profile_' . $field->fid, ''));
      $link    = l(t('edit'), 'admin/user/profile/edit/' . $field->fid, array('query' => 'destination=' . drupal_urlencode($_GET['q'])));
      $rows[]  = array($field->title, $field->name, $mapping, $link, '');
    }
    if (is_null($mapping)) {
      $rows[] = array(array('data' => t('No profile fields defined.'), 'colspan' => 5));
    }
  }

  return theme('table', $header, $rows);
}

function rdf_admin_autocomplete_resource($string = '', $limit = 15) {
  $matches = array();
  if (strlen($string = trim($string))) {
    $result = db_query("SELECT r.uri FROM {rdf_resources} r WHERE r.uri LIKE '%s%%' ORDER BY r.uri ASC LIMIT %d", $string, $limit);
    while ($resource = db_fetch_object($result)) {
      $matches[$resource->uri] = $resource->uri;
    }
  }
  drupal_json($matches);
}

function rdf_admin_autocomplete_class($string = '', $limit = 15) {
  $matches = array();
  if (!empty($string)) {
    // TODO
  }
  drupal_json($matches);
}

function rdf_admin_autocomplete_property($string = '', $limit = 15) {
  $matches = array();
  if (!empty($string)) {
    foreach (module_invoke_all('rdf_properties') as $namespace => $properties) {
      foreach ($properties as $property) {
        $match = implode(':', array($namespace, $property));
        if (strpos($match, $string) === 0) {
          $matches[$match] = $match;
        }
        if (count($matches) == $limit) {
          break 2;
        }
      }
    }
  }
  drupal_json($matches);
}

//////////////////////////////////////////////////////////////////////////////
// RDF feeds management

function rdf_admin_feeds() {
  $header = array('', t('Path'), t('Module'), t('Title'), t('Operations'));

  $rows = array();
  foreach (rdf_get_feed_info(NULL, TRUE) as $feed_id => $feed) {
    $module = unserialize(db_result(db_query("SELECT info FROM {system} WHERE type = 'module' AND name = '%s'", $feed->module)));

    $is_wildcard = (strpos($feed->path, '%') !== FALSE);
    $is_core     = in_array($feed->module, array('node', 'taxonomy', 'blog', 'aggregator'));
    $is_rdf      = !$is_core || array_search($feed->id, array_keys(variable_get('rdf_feed_override', array()))) !== FALSE;

    $path   = drupal_get_path_alias($feed->path);
    $rows[] = array(
      theme('feed_icon', !$is_wildcard ? url($feed->path) : '#', $path),
      !$is_wildcard ? l($path, $feed->path) : $feed->path,
      $module ? $module['name'] : $feed->module,
      $feed->title,
      l(t($is_rdf ? 'configure' : 'enable'), 'admin/settings/rdf/feeds/edit/' . $feed_id),
    );
  }

  return theme('table', $header, $rows);
}

function rdf_admin_feed(&$form_state, $feed_id) {
  $form = array();
  $feed = rdf_get_feed_info($feed_id);
  $edit = rdf_get_feed_settings($feed_id);

  $is_core = in_array($feed->module, array('node', 'taxonomy', 'blog', 'aggregator'));
  $is_rdf  = !$is_core || array_search($feed->id, array_keys(variable_get('rdf_feed_override', array()))) !== FALSE;

  // Feed compatibility
  $form['compatibility'] = array(
    '#type'          => 'fieldset',
    '#title'         => t('Feed compatibility'),
    '#collapsible'   => FALSE,
    '#collapsed'     => FALSE,
    '#description'   => rdf_help('admin/settings/rdf/feeds/edit#compatibility', $is_core),
  );
  if ($is_core) { // this is a feed from a core module, and needs an RDF override
    $form['compatibility']['override'] = array(
      '#type'          => 'radios',
      '#title'         => '',
      '#default_value' => $is_rdf ? 1 : 0,
      '#options'       => array(0 => t('RSS 2.0 (default, but <strong>not</strong> compatible with RDF)'), 1 => t('RSS 1.0 (<strong>recommended</strong>, compatible with RDF)')),
      '#description'   => t('The feeds provided by Drupal\'s core modules are <strong>not</strong> RDF-compatible by default. Use this setting to let the RDF module upgrade those feeds to reusable, extensible and future-proof RSS 1.0-compatible RDF output.'),
    );
  }
  else {
    $form['compatibility']['text'] = array('#value' => t('<a href="http://web.resource.org/rss/1.0/">RSS 1.0</a> (compatible with RDF)'));
  }

  // Channel settings
  $form['channel'] = array(
    '#type'          => 'fieldset',
    '#title'         => t('Channel settings'),
    '#collapsible'   => TRUE,
    '#collapsed'     => FALSE,
    '#description'   => rdf_help('admin/settings/rdf/feeds/edit#channel'),
  );
  rdf_admin_feed_channel_form($form['channel'], $form_state, $edit);

  // Item settings
  $form['item'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Item settings'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
    '#description' => rdf_help('admin/settings/rdf/feeds/edit#item'),
  );
  rdf_admin_feed_item_form($form['item'], $form_state, $edit);

  // TODO: namespace selection

  if ($is_core) { // this is a feed from a core module, and hence lacks some options
    $form['channel']['description_from_mission'] = array('#type' => 'hidden', '#value' => '');
    $form['channel']['description'] = array('#type' => 'hidden', '#value' => '');
  }

  if (!$is_rdf) { // disable extended configuration if the feed isn't RSS 1.0
    foreach (array('channel', 'item') as $fieldset) {
      $form[$fieldset]['#collapsed'] = TRUE;
      $form[$fieldset]['#attributes']['disabled'] = 'disabled';
      foreach ($form[$fieldset] as $field => &$field_info) {
        if ($field[0] != '#') {
          $field_info['#attributes']['disabled'] = 'disabled';
        }
      }
    }
  }

  $form['feed_id'] = array('#type' => 'hidden', '#value' => $feed_id);
  $form['submit']  = array('#type' => 'submit', '#value' => t('Save configuration'));

  return $form;
}

function rdf_admin_feed_channel_form(&$form, &$form_state, $defaults = array()) {
  $form['description_from_mission'] = array(
    '#type'          => 'checkbox',
    '#default_value' => !empty($defaults['description_from_mission']),
    '#title'         => t('Use the site mission as the channel description'),
  );
  $form['description'] = array(
    '#type'          => 'textfield',
    '#title'         => t('RSS 1.0 channel description'),
    '#default_value' => @$defaults['description'],
    '#description'   => t('This will appear as the channel description in the RDF feed.'),
    '#process'       => array('views_process_dependency'),
    '#dependency'    => array('edit-style-options-override' => array(FALSE)),
  );
  $form['update_period'] = array(
    '#type'          => 'select',
    '#title'         => t('Update period'),
    '#default_value' => @$defaults['update_period'],
    '#options'       => array('' => t('<not specified>'), 'hourly' => t('hourly'), 'daily' => t('daily'), 'weekly' => t('weekly'), 'monthly' => t('monthly'), 'yearly' => t('yearly')),
    '#description'   => t('Provides <a href="http://web.resource.org/rss/1.0/modules/syndication/">RSS 1.0 syndication hints</a> to aggregators and others picking up this RSS feed regarding how often it is updated.'),
  );
  $form['update_frequency'] = array(
    '#type'          => 'select',
    '#title'         => t('Update frequency'),
    '#default_value' => @$defaults['update_frequency'],
    '#options'       => array_merge(array('' => t('<not specified>')), drupal_map_assoc(range(1, 24))),
    '#description'   => t('Used to describe the frequency of updates in relation to the update period. This is a positive integer that indicates how many times in that period the channel is updated. For example, an update period of <em>daily</em> with an update frequency of <em>2</em> indicates the channel is updated twice daily. If omitted, a value of <em>1</em> is assumed.'),
  );
  $form['update_base'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Update base date'),
    '#default_value' => @$defaults['update_base'],
    '#description'   => t('Defines a base date to be used in concert with the update period and update frequency to calculate the publishing schedule. The date format takes the form: <code>yyyy-mm-ddThh:mm</code>.'),
    '#size'          => 16,
    '#maxlength'     => 16,
  );
  $form['format'] = array(
    '#type'          => 'select',
    '#title'         => t('Default format'),
    '#default_value' => !empty($defaults['format']) ? $defaults['format'] : RDF_FORMAT_RSS,
    '#options'       => rdf_get_formats('names'),
    '#description'   => t('This determines the default RDF serialization format to output. Browsers and RSS feed consumers can override this through <a href="http://en.wikipedia.org/wiki/Content_negotiation">HTTP content negotiation</a>, that is, by passing an <code>Accept</code> header specifying another supported format.'),
  );
}

function rdf_admin_feed_item_form(&$form, &$form_state, $defaults = array(), $module = NULL) {
  $form['item_length'] = array(
    '#type'          => 'select',
    '#title'         => t('Content display type'),
    '#default_value' => !empty($defaults['item_length']) ? $defaults['item_length'] : '',
    '#options'       => array(
      ''         => t('Use default RSS settings'),
      'fulltext' => t('Full text'),
      'teaser'   => t('Title plus teaser'),
      'title'    => t('Title only'),
    ),
    '#description'   => t('How to display content items in the feed.'),
  );
  $form['date_timezone'] = array(
    '#type'          => 'select',
    '#title'         => t('Time zone output'),
    '#default_value' => !empty($defaults['date_timezone']) ? $defaults['date_timezone'] : '',
    '#options'       => array('' => t('UTC'), 'local' => t('Local')),
    '#description'   => t('It is <strong>strongly</strong> recommended you output dates in UTC to prevent a multitude of problems due to buggy client software.'),
  );
}

function rdf_admin_feed_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  // TODO
}

function rdf_admin_feed_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $feed = rdf_get_feed_info($feed_id);

  $form_state['redirect'] = 'admin/settings/rdf/feeds';

  // For core module feeds
  if (isset($override)) {
    $overrides = variable_get('rdf_feed_override', array());
    $was_overridden = !empty($overrides[$feed->id]);

    if (!empty($override) && !$was_overridden) {
      $overrides[$feed->id] = TRUE;

      variable_set('rdf_feed_override', $overrides);
      drupal_set_message(t('The feed %path has been upgraded for RDF compatibility.', array('%path' => $feed->path)));
      menu_rebuild(); // we need to rebuild the menu cache for this to take effect

      $form_state['redirect'] = 'admin/settings/rdf/feeds/edit/' . $feed->id;
      return; // continue workflow on the same screen to allow for further feed configuration
    }
    else if (empty($override) && $was_overridden) {
      unset($overrides[$feed->id]);

      variable_set('rdf_feed_override', $overrides);
      drupal_set_message(t('The feed %path has been downgraded to the default format.', array('%path' => $feed->path)));
      menu_rebuild(); // we need to rebuild the menu cache for this to take effect
    }
    else if (empty($override)) {
      return; // don't save feed settings for non-RDF feeds
    }
  }

  // Save feed settings
  $function = ($feed->module == 'views') ? 'rdf_views_set_feed_settings' : 'rdf_set_feed_settings';
  $function($feed, compact('description_from_mission', 'description', 'update_period', 'update_frequency', 'update_base', 'format', 'item_length', 'date_timezone'));
  drupal_set_message(t('The feed configuration for %path has been updated.', array('%path' => $feed->path)));
}

//////////////////////////////////////////////////////////////////////////////
// RDF namespace management

function rdf_admin_namespaces() {
  $header = array(t('Prefix'), t('Base URI'), array('data' => t('Operations'), 'colspan' => '2'));

  // FIXME: need something more loosely-coupled and robust for this:
  $mutables = rdf_db_rdf_namespaces();

  $rows = array();
  foreach (rdf_get_namespaces() as $prefix => $uri) {
    $mutable = isset($mutables[$prefix]);
    $rows[] = array(
      $prefix, //l($prefix, 'admin/settings/rdf/predicates/' . $prefix),
      check_plain($uri),
      !$mutable ? '' : l(t('edit'), 'admin/settings/rdf/namespaces/edit/' . $prefix),
      !$mutable ? '' : l(t('delete'), 'admin/settings/rdf/namespaces/delete/' . $prefix),
    );
  }

  return theme('table', $header, $rows);
}

function rdf_db_admin_ns_edit($prefix = '') {
  if (empty($prefix)) {
    return drupal_get_form('rdf_db_admin_ns_form');
  }
  else {
    return drupal_get_form('rdf_db_admin_ns_form', rdf_db_load_namespace($prefix));
  }
}

function rdf_db_admin_ns_form(&$form_state, $edit = array('prefix' => '', 'uri' => '')) {
  $edit = (object)$edit;
  $form = array();

  $form['prefix'] = array('#type' => 'textfield', '#title' => t('Prefix'), '#default_value' => $edit->prefix, '#maxlength' => 64, '#required' => TRUE, '#description' => t('The short abbreviation to use in the place of the base URI. Keep it short, and use only lowercase, alphanumeric letters.'));
  $form['uri'] = array('#type' => 'textfield', '#title' => t('Base URI'), '#default_value' => $edit->uri, '#maxlength' => 255, '#required' => TRUE, '#description' => t('The absolute base URI of the RDF vocabulary. Make sure that the URI terminates with a hash or slash character.'));

  $form['key'] = array('#type' => 'hidden', '#value' => $edit->prefix);
  $form['submit'] = array('#type' => 'submit', '#value' => empty($edit->prefix) ? t('Create new namespace') : t('Update namespace'));

  return $form;
}

function rdf_db_admin_ns_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!preg_match('/^[a-z_]+[a-z\d\._]*$/', $prefix)) {
    form_set_error('prefix', t('Prefix %prefix contains illegal characters.', array('%prefix' => $prefix)));
  }

  if (!rdf_is_valid_uri($uri)) {
    form_set_error('uri', t('Base URI %uri contains illegal characters.', array('%uri' => $uri)));
  }

  if (!preg_match('@[#/:]$@', $uri)) {
    form_set_error('uri', t('Base URI %uri must terminate with "#", "/" or ":".', array('%uri' => $uri)));
  }
}

function rdf_db_admin_ns_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (empty($key)) {
    db_query("INSERT INTO {rdf_namespaces} (prefix, uri) VALUES ('%s', '%s')", $prefix, $uri);
    drupal_set_message(t('The namespace has been created.'));
  }
  else {
    db_query("UPDATE {rdf_namespaces} SET prefix = '%s', uri = '%s' WHERE prefix = '%s'", $prefix, $uri, $key);
    drupal_set_message(t('The namespace has been updated.'));
  }

  $form_state['redirect'] = 'admin/settings/rdf/namespaces';
}

function rdf_db_admin_ns_delete($form_state, $prefix) {
  if (user_access('administer RDF namespaces') && rdf_db_load_namespace($prefix)) {
    $form['prefix'] = array('#type' => 'value', '#value' => $prefix);
    $output = confirm_form($form,
      t('Are you sure you want to delete the RDF namespace %title?', array('%title' => $prefix)),
      isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/rdf/namespaces');
  }
  return $output;
}

function rdf_db_admin_ns_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    db_query("DELETE FROM {rdf_namespaces} WHERE prefix = '%s'", $form_state['values']['prefix']);
    drupal_set_message(t('The namespace has been deleted.'));

    $form_state['redirect'] = 'admin/settings/rdf/namespaces';
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF context management

function rdf_admin_contexts() {
  $header = array(t('URI'), t('Statements'));

  // FIXME: need something more loosely-coupled and robust for this:
  $mutables = rdf_db_rdf_contexts();

  $rows = array();
  foreach (rdf_get_contexts() as $uri) {
    $mutable = in_array($uri, $mutables);
    $rows[] = array(
      l($uri, 'admin/content/rdf', array('query' => array('g' => $uri, 's' => '', 'p' => '', 'o' => ''))),
      !$mutable ? t('n/a') : rdf_count(NULL, NULL, NULL, array('context' => $uri)),
    );
  }

  return theme('table', $header, $rows);
}

//////////////////////////////////////////////////////////////////////////////
// RDF repository management

function rdf_admin_repositories() {
  $header = array(t('Name'), t('Module'), t('Statements'), t('Mutable'), array('data' => t('Operations'), 'colspan' => '2'));

  $rows = array();
  foreach (rdf_get_repositories() as $name => $info) {
    $module  = unserialize(db_result(db_query("SELECT info FROM {system} WHERE type = 'module' AND name = '%s'", $info['module'])));

    $mutable = !empty($info['mutable']) && !in_array($name, array('system', 'local', 'default')); // FIXME
    $rows[] = array(
      !$mutable ? $info['title'] : l($info['title'], 'admin/settings/rdf/repositories/' . $info['module'] . '/edit/' . $name,
        array('attributes' => array('title' => @$info['dc:description']))),
      $module ? $module['name'] : $info['module'],
      $name == 'system' ? t('n/a') : (empty($info['statements']) ? '-' : number_format($info['statements'])),
      !empty($info['mutable']) ? t('Yes') : t('No'),
      !$mutable ? '' : l(t('edit'), 'admin/settings/rdf/repositories/' . $info['module'] . '/edit/' . $name),
      !$mutable ? '' : l(t('delete'), 'admin/settings/rdf/repositories/' . $info['module'] . '/delete/' . $name),
    );
  }

  return theme('table', $header, $rows);
}

function rdf_db_admin_db_edit($name = '') {
  if (empty($name)) {
    return drupal_get_form('rdf_db_admin_db_form');
  }
  else {
    return drupal_get_form('rdf_db_admin_db_form', rdf_db_load_repository($name));
  }
}

function rdf_db_admin_db_form(&$form_state, $edit = array('name' => '', 'title' => '', 'description' => '')) {
  $edit = (object)$edit;
  $form = array();

  $form['identity'] = array('#type' => 'fieldset', '#title' => t('Identification'));

  $form['identity']['title'] = array('#type' => 'textfield', '#title' => t('Name'), '#default_value' => $edit->title, '#maxlength' => 64, '#required' => TRUE, '#description' => t('The human-readable name of this repository. It is recommended that this name begins with a capital letter and consists only of letters, numbers, and spaces.'));
  $form['identity']['name'] = array('#type' => 'textfield', '#title' => t('ID'), '#default_value' => $edit->name, '#maxlength' => 32, '#required' => TRUE, '#description' => t('The machine-readable name of this repository. This text will be used for constructing the unique URI identifying this repository. This name may consist of only of lowercase letters, numbers, and underscores. Hyphens are not allowed. Underscores will be converted into hyphens when constructing the URI for the repository. This name must be unique to this repository.'));
  $form['identity']['description'] = array('#title' => t('Description'), '#type' => 'textarea', '#default_value' => $edit->description, '#rows' => 2, '#description' => t('A brief description of this repository.'));

  $form['key'] = array('#type' => 'hidden', '#value' => $edit->name);
  $form['submit'] = array('#type' => 'submit', '#value' => empty($edit->name) ? t('Create new repository') : t('Update repository'));

  return $form;
}

function rdf_db_admin_db_form_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!preg_match('/^[a-z]+[a-z\d_]*$/', $name)) {
    form_set_error('name', t('The machine-readable name can only consist of lowercase letters, underscores, and numbers.', array('%name' => $name)));
  }

  if (array_search($name, rdf_db_get_repository_tables()) !== FALSE) { // FIXME
    form_set_error('name', t('The machine-readable name %name is already used by another repository.', array('%name' => $name)));
  }
}

function rdf_db_admin_db_form_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (empty($key)) {
    rdf_db_create_repository($name, array('title' => $title, 'description' => $description));
    drupal_set_message(t('The repository has been created.'));
  }
  else {
    if ($key != $name) {
      rdf_db_rename_repository($key, $name);
    }
    rdf_db_update_repository($name, array('title' => $title, 'description' => $description));
    drupal_set_message(t('The repository has been updated.'));
  }

  $form_state['redirect'] = 'admin/settings/rdf/repositories';
}

function rdf_db_admin_db_delete($form_state, $name) {
  if (user_access('administer RDF repositories')) {
    $form['name'] = array('#type' => 'value', '#value' => $name);
    $output = confirm_form($form,
      t('Are you sure you want to delete the RDF repository %title?', array('%title' => $name)),
      isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/rdf/repositories',
      t('This action will destroy all data contained in the repository and cannot be undone.'));
  }
  return $output;
}

function rdf_db_admin_db_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    rdf_db_delete_repository($form_state['values']['name']);
    drupal_set_message(t('The repository has been deleted.'));

    $form_state['redirect'] = 'admin/settings/rdf/repositories';
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF data import

function rdf_import_admin_screen(&$form_state, $edit = array('repository' => 'local', 'url' => 'http://', 'format' => RDF_FORMAT)) {
  $edit = (object)$edit;
  $form = array();

  foreach (array('repository', 'url', 'format') as $key) {
    if (isset($_GET[$key])) {
      // This is idiotic, but due to drupal_urlencode() the URL has a
      // double-escaped slash character that we need to get rid of...
      // @see http://api.drupal.org/api/function/drupal_urlencode/6
      $edit->$key = ($key == 'url') ? str_replace('/%2F', '//', $_GET[$key]) : $_GET[$key];
    }
  }

  $form['import'] = array('#type' => 'fieldset', '#title' => t('Import RDF data from a URL'));
  $form['import']['repository'] = array(
    '#type'          => 'select',
    '#title'         => t('Repository'),
    '#default_value' => $edit->repository,
    '#options'       => rdf_get_repositories('names', array('persistent' => TRUE, 'mutable' => TRUE)),
    '#description'   => t('Select which RDF repository to store the imported data in.'),
  );
  $form['import']['url'] = array(
    '#type'          => 'textfield',
    '#title'         => 'URL',
    '#default_value' => $edit->url,
    '#maxlength'     => 255,
    '#required'      => TRUE,
    '#description'   => t('Enter a URL where to fetch the RDF data from.'),
  );
  $form['import']['format'] = array(
    '#type'          => 'select',
    '#title'         => t('Format'),
    '#default_value' => $edit->format,
    '#options'       => rdf_get_formats('names'), // FIXME,
    '#description'   => t('Specify the serialization format of the RDF data file.'),
  );

  $form['import']['submit'] = array('#type' => 'submit', '#value' => t('Import'));
  return $form;
}

function rdf_import_admin_screen_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);

  if (!rdf_is_valid_url($url)) {
    form_set_error('url', t('URL is not valid.'));
  }
}

function rdf_import_admin_screen_submit($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $formats = rdf_get_formats('names');

  $form_state['redirect'] = array('admin/content/rdf/import', drupal_query_string_encode(compact('repository', 'url', 'format')));

  // Fetch the serialized representation from the given URL:
  if (!($text = file_get_contents($url))) {
    drupal_set_message(t('Error while fetching RDF data from %url. Double-check that you did not misspell the URL.', array('%url' => $url)), 'error');
    return;
  }

  // Parse into triples using the specified RDF parser:
  if (($data = @rdf_unserialize($text, array('format' => $format, 'uri' => $url))) === FALSE) {
    drupal_set_message(t('Unable to parse @format data at %url. Double-check that you specified the correct format.', array('@format' => $formats[$format], '%url' => $url)), 'error');
    return;
  }

  if (empty($data)) {
    drupal_set_message(t('No @format data found at %url. Double-check that you specified the correct format.', array('@format' => $formats[$format], '%url' => $url)), 'error');
    return;
  }

  // Assert each triple in the specified repository, while also preserving
  // provenance. We keep track of any failures during assertions.
  $counter = 0;
  foreach ($data as $stmt) {
    if (call_user_func_array('rdf_insert', array_merge($stmt, array(array('graph' => $url, 'repository' => $repository))))) {
      $counter++;
    }
  }

  drupal_set_message(t('!count statements imported from %url.', array('!count' => $counter, '%url' => $url)));

  if (($errors = (count($data) - $counter))) {
    drupal_set_message(t('!count statements were not imported due to an error when asserting them.', array('!count' => $errors)), 'error');
  }
  else {
    $form_state['redirect'] = array('admin/content/rdf', drupal_query_string_encode(array('g' => $url, 's' => '', 'p' => '', 'o' => ''))); // success
  }
}

//////////////////////////////////////////////////////////////////////////////
// RDF data export

function rdf_export_admin_screen(&$form_state) {
  $form = array();
  $form['export'] = array('#type' => 'fieldset', '#title' => t('Export RDF data'));

  $repos = rdf_get_repositories('names');
  $form['export']['repos'] = array(
    '#type'          => 'checkboxes',
    '#title'         => t('Repositories'),
    '#default_value' => array_keys($repos),
    '#options'       => $repos,
    '#description'   => t('Select which repositories to query data from.'),
  );

  /*$namespaces = array_keys(rdf_get_namespaces('names'));
  $namespaces = array_combine($namespaces, $namespaces);
  unset($namespaces['_']); // always export bnodes
  $form['export']['vocabs'] = array('#type' => 'checkboxes', '#title' => t('Vocabularies'), '#default_value' => array_keys($namespaces), '#options' => $namespaces);*/

  $formats = rdf_get_formats('names', 'w');
  $form['export']['format'] = array(
    '#type'          => 'select',
    '#title'         => t('RDF format'),
    '#options'       => $formats,
    '#default_value' => RDF_FORMAT,
    '#description'   => t('Select the output format.'),
  );

  $form['export']['sort'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Sort by subject'),
    '#default_value' => '1',
  );

  $form['export']['submit'] = array('#type' => 'submit', '#value' => t('Export'));
  return $form;
}

function rdf_export_admin_screen_validate($form, &$form_state) {
  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $repos = array_keys(array_filter($repos, 'is_string'));
  //$vocabs = array_keys(array_filter($vocabs, 'is_string'));

  if (empty($repos)) {
    form_set_error('repos', t('No repositories selected.'));
  }

  /*if (empty($vocabs)) {
    form_set_error('vocabs', t('No vocabularies selected.'));
  }*/
}

function rdf_export_admin_screen_submit($form, &$form_state) {
  module_load_include('inc', 'rdf', 'rdf.pages');

  extract($form_state['values'], EXTR_SKIP | EXTR_REFS);
  $repos = array_values(array_filter($repos, 'is_string'));
  //$vocabs = array_values(array_filter($vocabs, 'is_string'));

  // TODO: filter by vocabulary selections
  $data = rdf_query(NULL, NULL, NULL, array('repository' => $repos));
  rdf_export($data, 'export', $format, array('log' => TRUE));
}
