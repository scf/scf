<?php
// $Id: rdf.feed.inc,v 1.4 2009/03/25 20:29:27 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// RDF feeds

function rdf_build_rss_feed($feed, $url, $items = array(), array $options = array()) {
  rdf_register_namespace('rss', RDF_RSS_URI);
  rdf_register_namespace('sy',  RDF_RSS_URI . 'modules/syndication/');

  // Compose the feed title:
  $title = variable_get('site_name', 'Drupal') . (($slogan = variable_get('site_slogan', '')) ? ' - ' . $slogan : '');
  $title = !empty($options['title']) ? $options['title'] : $title; // FIXME

  // Compose the feed description:
  $description = !empty($options['description_from_mission']) ? variable_get('site_mission', '') : $options['description'];

  // Construct the channel's metadata:
  $index   = rdf_seq();
  $channel = rdf_denormalize(array(
    $url => array(
      'rdf:type'        => rdf_qname_to_uriref('rss:channel'),
      'rss:title'       => rdf_literal($title, TRUE),
      'rss:link'        => !empty($options['link']) ? $options['link'] : $url,
      'rss:description' => rdf_literal($description, TRUE),
      'rss:items'       => $index->uriref(),
    ),
  ));

  // Provide support for RSS 1.0 Syndication hints:
  // @see http://web.resource.org/rss/1.0/modules/syndication/
  if (!empty($options['update_period'])) {
    $channel[] = array($url, 'sy:updatePeriod', check_plain($options['update_period']));
  }
  if (!empty($options['update_frequency'])) {
    $channel[] = array($url, 'sy:updateFrequency', check_plain($options['update_frequency']));
  }
  if (!empty($options['update_base'])) {
    $channel[] = array($url, 'sy:updateBase', check_plain($options['update_base']));
  }

  // Invoke hook_rdf_feed_channel() to allow third-party modules to
  // manipulate the RSS channel metadata:
  foreach (module_implements('rdf_feed_channel') as $module) {
    $function = $module . '_rdf_feed_channel';
    $function($feed, $url, $channel);
  }

  // Compile the channel index and render the feed items into triples:
  $data  = array();
  foreach ($items as $url => $item) {
    // Invoke hook_rdf_feed_item() to allow third-party modules to
    // manipulate the RSS item metadata:
    $item = rdf_denormalize(array($url => $item));
    foreach (module_implements('rdf_feed_item') as $module) {
      $function = $module . '_rdf_feed_item';
      $function($feed, $url, $item);
    }

    $index[] = $url;
    $data    = array_merge($data, $item);
  }

  // Invoke hook_rdf_feed() to allow third-party modules to manipulate the
  // full contents of the RSS feed, including the channel metadata and all
  // constituent feed items. Note that unlike the other two hooks,
  // hook_rdf_feed_channel() and hook_rdf_feed_item(), the $data argument to
  // this hook is given in normalized form. This makes manipulation easier
  // and may be a good reason to implement this hook instead of the other
  // two which, getting passed the triples in denormalized form, are more
  // suited to easily adding more triples to the channel and the feed items
  // than for manipulating existing data.
  // Implementers of this hook have final say in the feed output.
  $data = rdf_normalize(array_merge($channel, $index->to_triples(), $data));
  foreach (module_implements('rdf_views_feed') as $module) {
    $function = $module . '_rdf_views_feed';
    $function($feed, $url, $data);
  }

  return $data;
}

//////////////////////////////////////////////////////////////////////////////

function rdf_build_rss_feed_item($path, array $options = array()) {
  return array(); // TODO
}

function rdf_build_rss_feed_aggregator_item($item, $category = NULL) {
  switch ($feed_length = variable_get('feed_item_length', 'teaser')) {
    case 'teaser':
      if ($item->description != ($item->description = node_teaser($item->description))) {
        $item->description .= '<p><a href="' . check_url($item->link) . '">' . t('read more') . "</a></p>\n";
      }
      break;
    case 'title':
      $item->description = '';
      break;
  }

  $data = array(
    $item->link => array(
      'rss:title'       => t('@feed_title: @item_title', array('@feed_title' => $item->ftitle, '@item_title' => $item->title)),
      'rss:link'        => $item->link,
      'rss:description' => $item->description,
      'dc:date'         => rdf_datetime($item->timestamp),
      'dc:creator'      => !empty($item->author) ? $item->author : $item->ftitle,
      'dc:publisher'    => rdf_uri($item->flink),
    ),
  );

  if (!empty($category->title)) {
    $data[$item->link]['dc:subject'] = $category->title;
  }

  return $data;
}

//////////////////////////////////////////////////////////////////////////////

function rdf_build_rss_feed_node($nid, array $options = array()) {
  // Load the specified node:
  $item = node_load(is_object($nid) ? $nid->nid : $nid);
  $item->build_mode = NODE_BUILD_RSS;
  $item->url  = url('node/' . $item->nid, array('absolute' => TRUE, 'alias' => TRUE));
  $item->link = url('node/' . $item->nid, array('absolute' => TRUE));

  // Apply input filters and prepare the node teaser, also allowing other
  // $modules to change node->teaser before viewing:
  $item_length = variable_get('feed_item_length', 'teaser');
  $item_length = (!empty($options['item_length']) && $options['item_length'] != 'default') ? $options['item_length'] : $item_length;
  $teaser = ($item_length == 'teaser');
  $item = node_hook($item, 'view') ? node_invoke($item, 'view', $teaser, FALSE) : node_prepare($item, $teaser);
  node_invoke_nodeapi($item, 'view', $teaser, FALSE);

  // Make sure the item has all the basic mandatory RSS 1.0 properties:
  // @see http://web.resource.org/rss/1.0/spec#s5.5
  $item->rdf = array(
    'rdf:type'  => rdf_qname_to_uriref('rss:item'),
    'rss:title' => rdf_literal($item->title, $item),
    'rss:link'  => $item->link, // NOTE: it actually *is* defined as an xsd:string property, not a URI.
  );

  // Include the node body using a rss:description property unless "Title
  // only" was selected in the feed configuration. This property is optional
  // in the spec so leaving it out is fine.
  if ($item_length != 'title') {
    // Prepare the item description:
    switch ($item_length) {
      case 'fulltext':
        $item_text = trim($item->body);
        break;
      case 'teaser':
        $item_text = trim($item->teaser);
        if (!empty($item->readmore)) {
          $item_text .= '<p>' . l(t('read more'), 'node/' . $item->nid, array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))) . '</p>';
        }
        break;
    }
    if ($item_text != '') {
      $item->rdf['rss:description'][] = rdf_literal($item_text, $item);
    }
  }

  // Add dc:date for the created date and dc:creator for the author:
  // @see http://web.resource.org/rss/1.0/modules/dc/
  $item->rdf['dc:date'][]    = rdf_datetime($item->created, empty($options['date_timezone']) ? 0 : variable_get('date_default_timezone', 0));
  $item->rdf['dc:creator'][] = $item->name;

  // Add dc:subject properties for the node's taxonomy terms.
  // @see http://web.resource.org/rss/1.0/modules/dc/
  if (!empty($item->taxonomy)) {
    foreach ($item->taxonomy as $tid => $term) {
      // TODO: should these be output as literals or taxonomy URIs?
      $item->rdf['dc:subject'][] = $term->name;
    }
  }

  // Invoke hook_nodeapi('rss item') to allow modules to add additional item
  // fields and/or modify $item; this is the same hook as used by
  // node_feed() in node.module.
  foreach (node_invoke_nodeapi($item, 'rss item') as $element) {
    // To prevent general pandemonium due to the modules responding to this
    // hook having been designed around RSS 2.0, we only support
    // explicitly-namespaced properties (such as those output by the
    // Location module).
    if (!empty($element['namespace'])) {
      foreach ($element['namespace'] as $ns => $ns_uri) {
        $ns_prefix = preg_replace('/^xmlns:(.*)$/', '\1', $ns);
        rdf_register_namespace($ns_prefix, $ns_uri);
      }

      if (!is_array($element['value'])) {
        $item->rdf[$element['key']][] = $element['value'];
      }
      else {
        // TODO: need to construct an equivalent BNode structure
        //$item->rdf[$element['key']][] = 
      }
    }
  }

  // HACK: this snippet provides direct support for RDF-mapped CCK fields.
  // This should eventually be properly handled by the RDF Schema API.
  if (isset($item->content) && is_array($item->content)) {
    foreach ($item->content as $field_name => $field_info) {
      rdf_build_rss_feed_node_field($item, $field_name, $field_info, $options);
    }
    if (module_exists('fieldgroup')) {
      foreach (fieldgroup_groups($item->type) as $group_name => $group) {
        foreach ($group['fields'] as $field_name => $field_info) {
          rdf_build_rss_feed_node_field($item, $field_name, $field_info, $options);
        }
      }
    }
  }

  // Query the RDF API for all available information on this node. The RDF
  // Schema API takes care of mapping core fields and CCK fields to RDF
  // properties for us to use. Note that we remove any returned rdf:type
  // properties as we want these nodes to be explicitly typed to rss:item.
  $skip_predicates = array_map('rdf_qname_to_uri', array('rdf:type'));
  foreach (rdf_query($item->url) as $statement) {
    list($s, $p, $o) = $statement;
    if (!in_array((string)$p, $skip_predicates)) { // skip rdf:type
      $item->rdf[$p][] = $o;
    }
  }

  return array($item->url => $item->rdf);
}

function rdf_build_rss_feed_node_field(&$item, $field_name, $field_info, array $options) {
  foreach (array('' => 'value', '[from]' => 'value', '[to]' => 'value2') as $field_suffix => $field_value_index) {
    if (($field_uri = variable_get('rdf_schema_property_content_' . $field_name . $field_suffix, '')) != '' && rdf_is_valid_uri($field_uri)) {
      if (isset($item->$field_name)) {
        $field_values = is_array($item->$field_name) ? $item->$field_name : array($item->$field_name);
        foreach ($field_values as $field_value) {
          if (($value = $field_value[$field_value_index]) != '') { // skip empty values

            // Cast CCK Date field values into the correct RDF datatype,
            // with optional timezone information:
            if (isset($field_value['date_type'])) {
              $value = rdf_get_rss_feed_date_field($field_value, $field_value_index, $options);
            }
            $item->rdf[$field_uri][] = $value;
          }
        }
      }
    }
  }
}

function rdf_get_rss_feed_date_field($field_value, $field_value_index, array $options) {
  $timezone_db  = timezone_open($field_value['timezone_db']);
  $timezone_cck = timezone_open($field_value['timezone']);
  $timezone     = timezone_offset_get($timezone_cck, date_create('now', $timezone_cck));

  switch ($field_value['date_type']) {
    case 'datestamp': // value, value2, timezone, offset, offset2, timezone_db
      $offset    = $field_value[str_replace('value', 'offset', $field_value_index)];
      $timestamp = (int)$field_value[$field_value_index];
      $timestamp = $timestamp + (int)$offset - $timezone;
      break;
    case 'date':
    case 'datetime':  // value, value2, timezone, timezone_db
      $datetime  = date_create($field_value[$field_value_index], $timezone_db);
      $timestamp = (int)date_format($datetime, 'U');
      break;
  }

  return rdf_datetime($timestamp, empty($options['date_timezone']) ? 0 : $timezone);
}
