<?php
// $Id: views_plugin_row_node_rdf.inc,v 1.12 2009/03/02 03:08:17 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Views API plugins

/**
 * Views row plugin that invokes node_view() on each node and prepares its
 * body for inclusion as an RSS 1.0 item description in the feed.
 *
 * @see http://web.resource.org/rss/1.0/spec
 * @ingroup views_row_plugins
 */
class views_plugin_row_node_rdf extends views_plugin_row {
  function option_definition() {
    $options = parent::option_definition();
    $options['item_length'] = array('default' => 'default');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    module_load_include('inc', 'rdf', 'rdf.admin'); // rdf.admin.inc
    rdf_admin_feed_item_form($form, $form_state, $this->options['item_length'], 'views');
  }

  function render($row) {
    // Build the RSS 1.0-compatible RDF description of this node:
    module_load_include('inc', 'rdf', 'rdf.feed'); // rdf.feed.inc
    return rdf_build_rss_feed_node($row->nid, $this->options);
  }
}
