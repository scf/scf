<?php
// $Id: rdf.pages.inc,v 1.11 2009/03/25 22:55:20 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Menu callbacks for RDF export

function rdf_export_site() {
  $data = rdf_query(url(NULL, array('absolute' => TRUE)));
  rdf_export($data, 'site');
}

function rdf_export_node($node) {
  return rdf_export_entity('node', $node->nid);
}

function rdf_export_user($account) {
  return rdf_export_entity('user', $account->uid);
}

function rdf_export_entity($type, $id) {
  $data = rdf_query(url($type . '/' . $id, array('absolute' => TRUE)));
  rdf_export($data, implode('-', array($type, $id)));
}

/**
 * @deprecated Replace with rdf_output().
 */
function rdf_export($data, $filename = 'export', $format = RDF_FORMAT, $options = array()) {
  //module_load_include('inc', 'rdf_export', 'rdf_export');

  $formats = rdf_get_formats();
  $format = isset($formats[@$_GET['format']]) ? $_GET['format'] : $format;
  $format = $formats[$format];
  $filename = implode('.', array($filename, $format->file_ext));

  if (!empty($options['log'])) {
    watchdog('rdf', 'Exported RDF data file: %filename.', array('%filename' => $filename));
  }

  $output = rdf_serialize(is_string($data) ? $data() : $data, array('format' => $format->name));

  header('Content-Disposition: inline; filename=' . $filename);
  header('Content-Type: ' . $format->mime_type . '; charset=' . $format->encoding);
  header('Content-Length: ' . strlen($output));
  die($output);
}

function rdf_output_rss($filename, $format, $data, array $options = array()) {
  if ($format == 'rdf+xml' && !isset($options['content_type'])) {
    $options['content_type'] = 'application/rss+xml';
  }
  $options['base'] = isset($options['base']) ? $options['base'] : RDF_RSS_URI;
  return rdf_output($filename, $format, $data, $options);
}

function rdf_output($filename, $format, $data, array $options = array()) {
  global $base_url, $language;

  // Figure out the serialization format and compose the filename
  $formats  = rdf_get_formats();
  $format   = !empty($format) ? $format : RDF_FORMAT;
  $format   = isset($formats[$format]) ? $formats[$format] : reset($formats); // TODO: auto-negotiation
  $filename = !empty($filename) ? $filename : 'drupal';
  $filename = implode('.', array($filename, $format->file_ext));

  // Merge in the default options
  $defaults = array(
    'content_disposition'  => 'inline',
    'content_type'         => $format->mime_type,
    'content_type_charset' => $format->encoding,
  );
  $options = array_merge($defaults, $options);

  // Developers: define this constant in your settings.php to make life easier:
  if (defined('TRACE_TEXT_OUTPUT')) {
    $options['content_type'] = 'text/plain';
  }

  // Serialize the RDF data
  $content = rdf_serialize($data, array(
    'format'   => $format->name,
    'language' => isset($options['language']) ? $options['language'] : $language->language,
    'base'     => isset($options['base'])     ? $options['base']     : NULL,
  ));

  // Compose HTTP response headers
  $options['http_headers'] = !empty($options['http_headers']) ? $options['http_headers'] : array();
  $options['http_headers'] = array_merge($options['http_headers'], array(
    'Content-Disposition' => $options['content_disposition'] . '; filename=' . $filename,
    'Content-Type'        => $options['content_type'] . '; charset=' . $options['content_type_charset'],
    'Content-Length'      => strlen($content),
  ));

  // Output HTTP response headers and content
  if (!headers_sent()) {
    if (isset($options['http_status'])) {
      drupal_set_header('HTTP/1.1 ' . $options['http_status']);
    }
    foreach ($options['http_headers'] as $k => $vs) {
      foreach (is_array($vs) ? $vs : array($vs) as $v) {
        drupal_set_header($k . ': ' . $v);
      }
    }
  }
  print $content;
}

//////////////////////////////////////////////////////////////////////////////
// Menu callbacks for RDF feeds

function rdf_feed_callback($feed_id) {
  $feed = rdf_get_feed_info($feed_id);
  $feed->arguments = array_slice(func_get_args(), 1);
  $feed->settings  = rdf_get_feed_settings($feed_id);

  $path = ($feed->path == variable_get('site_frontpage', 'node')) ? '' : $feed->path;
  $path = $_GET['q']; // FIXME
  $link = url($path, array('absolute' => TRUE));

  module_load_include('inc', 'rdf', 'rdf.feed'); // rdf.feed.inc
  $items = call_user_func_array($feed->callback, array_merge(array(&$feed), $feed->arguments));

  if (is_null($items) || is_string($items) || is_int($items)) {
    // Return to the Drupal menu handler
    return $items;
  }
  else {
    // Output RSS feed
    $data = rdf_build_rss_feed($feed, $link, $items, $feed->settings);
    rdf_output_rss('feed', $feed->settings['format'], $data);
    exit;
  }
}

/**
 * Overrides node.module's RSS output.
 *
 * @see node_feed()
 */
function rdf_feed_node_frontpage(&$feed) {
  $feed->settings['link'] = url('', array('absolute' => TRUE));

  $result = db_query_range(db_rewrite_sql('SELECT n.nid FROM {node} n WHERE n.promote = 1 AND n.status = 1 ORDER BY n.created DESC'), 0, variable_get('feed_default_items', 10));
  $items  = array();
  while ($item = db_fetch_object($result)) {
    $items[] = $item->nid;
  }
  return new RDF_CallbackIterator('rdf_build_rss_feed_node', array(), $items, array($feed->settings));
}

/**
 * Overrides taxonomy.module's RSS output.
 *
 * @see taxonomy_term_page()
 */
function rdf_feed_taxonomy_term(&$feed, $str_tids = '', $depth = 0, $op = 'page') {
  // For anything but the RSS feed, we must delegate back to taxonomy.module:
  if ($op != 'feed') {
    module_load_include('inc', 'taxonomy', 'taxonomy.pages'); // taxonomy.pages.inc
    return taxonomy_term_page($str_tids, $depth, $op);
  }

  $terms = taxonomy_terms_parse_string($str_tids);
  if (($terms['operator'] == 'and' || $terms['operator'] == 'or') && !empty($terms['tids'])) {
    // Rebuild the terms array to make sure it only contains terms the user has access to
    $result = db_query(db_rewrite_sql('SELECT t.tid, t.name FROM {term_data} t WHERE t.tid IN (' . db_placeholders($terms['tids']) . ')', 't', 'tid'), $terms['tids']);
    $terms['tids'] = $terms['names'] = array();
    while ($term = db_fetch_object($result)) {
      $terms['tids'][]  = $term->tid;
      $terms['names'][] = $term->name;
    }

    if (!empty($terms['tids'])) {
      $path = 'taxonomy/term/' . $str_tids . (empty($depth) ? '' : '/' . $depth);
      $feed->settings['link']  = url($path, array('absolute' => TRUE));
      $feed->settings['title'] = variable_get('site_name', 'Drupal') . ' - ' . check_plain(implode(', ', $terms['names']));
      // Only display the description if we have a single term:
      $feed->settings['description'] = (count($tids) == 1 && ($term = taxonomy_get_term($tids[0]))) ? $term->description : NULL;

      $result = taxonomy_select_nodes($terms['tids'], $terms['operator'], $depth, FALSE);
      $items  = array();
      while ($item = db_fetch_object($result)) {
        $items[] = $item->nid;
      }
      return new RDF_CallbackIterator('rdf_build_rss_feed_node', array(), $items, array($feed->settings));
    }
  }

  return drupal_not_found();
}

/**
 * Overrides blog.module's RSS output. Displays an RSS feed containing
 * recent blog entries of a given user.
 *
 * @see blog_feed_user()
 */
function rdf_feed_blog_user(&$feed, $account) {
  $feed->settings['title'] = t('!name\'s blog', array('!name' => $account->name));
  $feed->settings['link']  = url('blog/' . $account->uid, array('absolute' => TRUE));

  $result = db_query_range(db_rewrite_sql("SELECT n.nid, n.created FROM {node} n WHERE n.type = 'blog' AND n.uid = %d AND n.status = 1 ORDER BY n.created DESC"), $account->uid, 0, variable_get('feed_default_items', 10));
  $items  = array();
  while ($item = db_fetch_object($result)) {
    $items[] = $item->nid;
  }
  return new RDF_CallbackIterator('rdf_build_rss_feed_node', array(), $items, array($feed->settings));
}

/**
 * Overrides blog.module's RSS output. Displays an RSS feed containing
 * recent blog entries of all users.
 *
 * @see blog_feed_last()
 */
function rdf_feed_blog_last(&$feed) {
  $feed->settings['title'] = t('@site_name blogs', array('@site_name' => variable_get('site_name', 'Drupal')));
  $feed->settings['link']  = url('blog', array('absolute' => TRUE));

  $result = db_query_range(db_rewrite_sql("SELECT n.nid, n.created FROM {node} n WHERE n.type = 'blog' AND n.status = 1 ORDER BY n.created DESC"), 0, variable_get('feed_default_items', 10));
  $items  = array();
  while ($item = db_fetch_object($result)) {
    $items[] = $item->nid;
  }
  return new RDF_CallbackIterator('rdf_build_rss_feed_node', array(), $items, array($feed->settings));
}

/**
 * Overrides aggregator.module's RSS output. Generates an RSS 1.0 feed of
 * aggregator items or categories.
 *
 * @see aggregator_page_rss()
 */
function rdf_feed_aggregator_rss(&$feed, $cid = NULL) {
  if ($cid) {
    $category = db_fetch_object(db_query('SELECT cid, title FROM {aggregator_category} WHERE cid = %d', $cid));
    $result = db_query_range('SELECT i.*, f.title AS ftitle, f.link AS flink FROM {aggregator_category_item} c LEFT JOIN {aggregator_item} i ON c.iid = i.iid LEFT JOIN {aggregator_feed} f ON i.fid = f.fid WHERE cid = %d ORDER BY timestamp DESC, i.iid DESC', $category->cid, 0, variable_get('feed_default_items', 10));
  }
  else {
    $category = NULL;
    $result = db_query_range('SELECT i.*, f.title AS ftitle, f.link AS flink FROM {aggregator_item} i INNER JOIN {aggregator_feed} f ON i.fid = f.fid ORDER BY i.timestamp DESC, i.iid DESC', 0, variable_get('feed_default_items', 10));
  }

  $feed->settings['title'] = t('@site_name aggregator', array('@site_name' => ($site_name = variable_get('site_name', 'Drupal'))));
  $feed->settings['link']  = url(empty($category) ? 'aggregator' : 'aggregator/categories/' . $category->cid, array('absolute' => TRUE));
  $feed->settings['description'] = !empty($category) ?
    t('@site_name - aggregated feeds in category @title', array('@site_name' => $site_name, '@title' => $category->title)) :
    t('@site_name - aggregated feeds', array('@site_name' => $site_name));

  $items = array();
  while ($item = db_fetch_object($result)) {
    $items[] = $item;
  }
  return new RDF_CallbackIterator('rdf_build_rss_feed_aggregator_item', array(), $items, array($category));
}

//////////////////////////////////////////////////////////////////////////////
// Menu callbacks

function rdf_schema_export_settings($prefix = NULL) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    return rdf_schema_import_settings();
  }

  $data = array();
  $result = $prefix ?
    db_query('SELECT name, value FROM {variable} WHERE name LIKE \'%s\' ORDER BY name, value', str_replace('*', '%', $prefix) .'%') :
    db_query('SELECT name, value FROM {variable} ORDER BY name, value');
  while ($variable = db_fetch_object($result)) {
    $subject = url('rdf/variable/'. $variable->name, array('absolute' => TRUE));

    if (strlen($variable->value) > 3000) {
      // TODO: is this still needed?
      continue; // HACK: the ARC Turtle parser currently chokes on literals longer than ~3000 characters
    }

    $data[$subject] = array(
      rdf::type   => array(rdf_uri('http://drupal.org/rdf/variable')),
      rdfs::label => array($variable->name),
      rdf::value  => array(rdf_literal($variable->value, NULL, 'http://purl.org/php/serialized')), // FIXME
    );
  }
  rdf_schema_export($data, 'settings', 'ntriples');
}

function rdf_schema_export_modules($prefix = NULL) {
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    return rdf_schema_import_modules();
  }

  $data = array();
  $result = $prefix ?
    db_query('SELECT name, status FROM {system} WHERE type = \'module\' AND name LIKE \'%s\' ORDER BY name, status', str_replace('*', '%', $prefix) .'%') :
    db_query('SELECT name, status FROM {system} WHERE type = \'module\' ORDER BY name, status');
  while ($module = db_fetch_object($result)) {
    $subject = url('rdf/module/'. $module->name, array('absolute' => TRUE));
    $data[$subject] = array(
      rdf::type   => array(rdf_uri('http://drupal.org/rdf/module')),
      rdfs::label => array($module->name),
      'http://drupal.org/rdf/terms#status' => array(rdf_literal($module->status, NULL, 'xsd:boolean')),
    );
  }
  rdf_schema_export($data, 'modules', 'ntriples');
}

function rdf_schema_export($data, $filename = 'export', $format = RDF_FORMAT, $options = array()) {
  $formats = rdf_get_formats();
  $format = isset($formats[@$_GET['format']]) ? $_GET['format'] : $format;
  $format = $formats[$format];
  $filename = implode('.', array($filename, $format->file_ext));

  $output = rdf_serialize(is_string($data) ? $data() : $data, array('format' => $format->name));

  $format->mime_type = 'text/plain'; // FIXME
  header('Content-Disposition: inline; filename='. $filename);
  header('Content-Type: '. $format->mime_type .'; charset='. $format->encoding);
  header('Content-Length: '. strlen($output));
  die($output);
}

function rdf_schema_import_settings() {
  $input = file_get_contents('php://input');
  $input = rdf_normalize(rdf_unserialize($input, array('format' => 'ntriples')));

  $config = array();
  foreach ($input as $data) {
    if ((string)$data[rdf::type][0] == 'http://drupal.org/rdf/variable') {
      if (($name = $data[rdfs::label][0]) && isset($data[rdf::value])) {
        $value = unserialize($data[rdf::value][0]->value);
        $config[$name] = $value;
      }
    }
  }

  foreach ($config as $name => $value) {
    variable_set($name, $value);
  }

  printf("OK (imported %d variables)\n", count($config));
}

function rdf_schema_import_modules() {
  include_once './includes/install.inc';

  $input = file_get_contents('php://input');
  $input = rdf_normalize(rdf_unserialize($input, array('format' => 'ntriples')));

  $modules = array();
  foreach ($input as $data) {
    if ((string)$data[rdf::type][0] == 'http://drupal.org/rdf/module') {
      if (($name = $data[rdfs::label][0]) && isset($data['http://drupal.org/rdf/terms#status'])) {
        $enabled = $data['http://drupal.org/rdf/terms#status'][0];
        $enabled = !empty($enabled) && (bool)$enabled->value;
        if ($enabled && drupal_get_installed_schema_version($name) == SCHEMA_UNINSTALLED) {
          $modules['install'][] = $name;
        }
        else {
          $modules[$enabled ? 'enable' : 'disable'][] = $name;
        }
      }
    }
  }

  if (!empty($modules['enable'])) {
    module_enable($modules['enable']);
  }

  if (!empty($modules['disable'])) {
    module_disable($modules['disable']);
  }

  if (!empty($modules['install'])) {
    foreach ($modules['install'] as $key => $module) {
      if (!drupal_check_module($module)) {
        unset($modules['install'][$key]);
      }
    }
    drupal_install_modules($modules['install']);
  }

  drupal_clear_css_cache();
  drupal_clear_js_cache();

  // Notify locale module about module changes, so translations can be
  // imported. This might start a batch, and only return to the redirect
  // path after that.
  module_invoke('locale', 'system_update', $modules['install']);

  // Synchronize to catch any actions that were added or removed.
  actions_synchronize();

  printf("OK\n");
}
