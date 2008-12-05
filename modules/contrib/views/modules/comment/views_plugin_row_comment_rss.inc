<?php
// $Id: views_plugin_row_comment_rss.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * @file
 * Contains the comment RSS row style plugin.
 */

/**
 * Plugin which formats the comments as RSS items.
 */
class views_plugin_row_comment_rss extends views_plugin_row {

  function render($row) {
    global $base_url;

    if (empty($this->view->style_plugin->namespaces)) {
      $this->view->style_plugin->namespaces['xmlns:dc'] = 'http://purl.org/dc/elements/1.1/';
    }

    // Load the specified comment:
    $item = _comment_load($row->cid);
    $item->link = url('node/' . $item->nid, array('absolute' => TRUE, 'fragment' => 'comment-' . $item->cid));

    $properties = array(
      array('key' => 'pubDate', 'value' => gmdate('r', $item->timestamp)),
      array('key' => 'dc:creator', 'value' => $item->name),
      array(
        'key' => 'guid',
        'value' => 'comment ' .  $item->cid . ' at ' . $base_url,
        'attributes' => array('isPermaLink' => 'false')
      ),
    );
    $item_text = check_markup($item->comment, $item->format, FALSE);
    return format_rss_item($item->subject, $item->link, $item_text, $properties);
  }
}
