<?php
// $Id: views_plugin_style_rdf.inc,v 1.10 2009/03/02 03:08:17 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Views API plugins

/**
 * Views style plugin that outputs an RSS 1.0-compatible RDF feed.
 *
 * @see http://web.resource.org/rss/1.0/spec
 * @ingroup views_style_plugins
 */
class views_plugin_style_rdf extends views_plugin_style {
  function attach_to($display_id, $path, $title) {
    $display = $this->view->display[$display_id]->handler;
    $url_options = array();
    if (($input = $this->view->get_exposed_input())) {
      $url_options['query'] = $input;
    }

    $title = !empty($title) ? $title : t('RSS 1.0');
    $url = url($this->view->get_url(NULL, $path), $url_options);
    if ($display->has_path()) {
      if (empty($this->preview)) {
        rdf_add_autodiscovery_link($title, $url, $this->options['format']);
        rdf_add_autodiscovery_link($title, $url, $this->options['format'], array('rel' => 'alternate'));
      }
    }
    else {
      if (empty($this->view->feed_icon)) {
        $this->view->feed_icon = '';
      }
      $this->view->feed_icon .= theme('feed_icon', $url, $title);
      rdf_add_autodiscovery_link($title, $url, $this->options['format']);
      rdf_add_autodiscovery_link($title, $url, $this->options['format'], array('rel' => 'alternate'));
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['description'] = array('default' => '', 'translatable' => TRUE);
    $options['description_from_mission'] = array('default' => '', 'translatable' => TRUE);
    $options['update_period'] = array('default' => '', 'translatable' => TRUE);
    $options['update_frequency'] = array('default' => '', 'translatable' => TRUE);
    $options['update_base'] = array('default' => '', 'translatable' => TRUE);
    $options['format'] = array('default' => RDF_FORMAT, 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    module_load_include('inc', 'rdf', 'rdf.admin'); // rdf.admin.inc
    rdf_admin_feed_channel_form($form, $form_state, $this->options, 'views');
  }

  function render() {
    global $base_url;

    if (empty($this->row_plugin)) {
      vpr('views_plugin_style_rdf: Missing row plugin');
      return;
    }

    // Figure out which display has a path we're using for this feed.
    // If there isn't one, use the global $base_url.
    $link_display_id = $this->view->display_handler->get_link_display(); // FIXME: can return incorrect feed...
    if (!$link_display_id || empty($this->view->display[$link_display_id])) {
      vpr('views_plugin_style_rdf: Missing display ID');
      return;
    }

    // Compose the feed title:
    $title = $this->view->display_handler->get_option('sitename_title') ?
      (variable_get('site_name', 'Drupal') . ($slogan = variable_get('site_slogan', '')) ? ' - ' . $slogan : '') :
      $this->view->get_title();

    // Construct the feed URL:
    $path = $this->view->display[$link_display_id]->handler->get_path();
    $path = $this->view->get_url(NULL, $path);

    // Add any view arguments passed in the URL query string:
    $url_options = array('absolute' => TRUE);
    if (!empty($this->view->exposed_raw_input)) {
      $url_options['query'] = $this->view->exposed_raw_input;
    }

    // Compare the link to the site's default home page; if it is, in fact,
    // the default home page, just use $base_url directly:
    $path = ($path == variable_get('site_frontpage', 'node')) ? '' : $path;
    $link = check_url(url($path, $url_options)); // FIXME: RSS link should go to view URL, not feed URL

    // Build the data graph for the RSS feed, in memory:
    module_load_include('inc', 'rdf', 'rdf.feed'); // rdf.feed.inc
    $data = rdf_build_rss_feed('views_' . $this->view->name, $link,
      new RDF_CallbackIterator(array($this->row_plugin, 'render'), array(), $this->view->result),
      array_merge(compact('title', 'link'), $this->options));

    // Output the RSS feed using the configured RDF serialization:
    module_load_include('inc', 'rdf', 'rdf.pages'); // rdf.pages.inc
    rdf_output_rss($this->view->name, $this->options['format'], $data);
    exit;
  }
}
