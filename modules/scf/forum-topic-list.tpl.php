<?php
// $Id: forum-topic-list.tpl.php,v 1.4 2007/08/30 18:58:12 goba Exp $

/**
 * @file forum-topic-list.tpl.php
 * Theme implementation to display a list of forum topics.
 *
 * Available variables:
 * - $header: The table header. This is pre-generated with click-sorting
 *   information. If you need to change this, @see template_preprocess_forum_topic_list().
 * - $pager: The pager to display beneath the table.
 * - $topics: An array of topics to be displayed.
 * - $topic_id: Numeric id for the current forum topic.
 *
 * Each $topic in $topics contains:
 * - $topic->icon: The icon to display.
 * - $topic->moved: A flag to indicate whether the topic has been moved to
 *   another forum.
 * - $topic->title: The title of the topic. Safe to output.
 * - $topic->message: If the topic has been moved, this contains an
 *   explanation and a link.
 * - $topic->zebra: 'even' or 'odd' string used for row class.
 * - $topic->num_comments: The number of replies on this topic.
 * - $topic->new_replies: A flag to indicate whether there are unread comments.
 * - $topic->new_url: If there are unread replies, this is a link to them.
 * - $topic->new_text: Text containing the translated, properly pluralized count.
 * - $topic->created: An outputtable string represented when the topic was posted.
 * - $topic->last_reply: An outputtable string representing when the topic was
 *   last replied to.
 * - $topic->timestamp: The raw timestamp this topic was posted.
 * - $topic->admin_links: Themed links like "Edit", "Add", etc. output from
 *   theme_links().
 *
 * @see template_preprocess_forum_topic_list()
 * @see theme_forum_topic_list()
 */
?>
<table id="forum-topic-<?php print $topic_id; ?>" class="forum-topic-list">
  <thead>
    <tr><?php print $header; ?></tr>
  </thead>
  <tbody>
<?php foreach ($topics as $topic): ?>
    <tr class="<?php print $topic->zebra;?>">
      <td class="icon"><?php print $topic->icon; ?></td>
      <td class="title">
        <div class="forum-inner">
          <?php print $topic->title; ?>
          <div class="clear-block"><?php print $topic->admin_links; ?></div>
        </div>
      </td>
  <?php if ($topic->moved): ?>
      <td colspan="3"><?php print $topic->message; ?></td>
  <?php else: ?>
      <td class="replies">
        <?php print $topic->num_comments; ?>
    <?php if ($topic->new_replies): ?>
          , <span class="new"><a href="<?php print $topic->new_url; ?>"><?php print $topic->new_text; ?></a></span>
    <?php endif; ?>
      </td>
      <td class="created"><?php print $topic->created; ?>
      <td class="last-reply"><?php print $topic->last_reply; ?>
  <?php endif; ?>
    </tr>
<?php endforeach; ?>
  </tbody>
</table>
<?php print $pager; ?>
