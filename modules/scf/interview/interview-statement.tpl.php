<?php
// $Id: $

/**
 * @file interview-statement.tpl.php
 *
 * Theme implementation to display a member in an embedded context (e.g. in an interview).
 * Has all node variables (see node.tpl.php) available, and also the following:
 *
 * - $content: 
 * - $statement_num: 
 * - $image: 
 * - $is_interviewer: 
 * - $participant_key:
 * - $participant_name:
 * - $is_first: 
 * - $is_last: 
 */


$divclass = ($is_interviewer ? 'interviewer' : 'interviewee');
$pkey = empty($participant_key) ? '??' : $participant_key;

?>
<div id="interview-statement-<?php print $statement_num; ?>" class="clear-block <?php print $divclass; ?>">

  <div class="participant-key" title="<?php print $participant_name ?>"><?php print $pkey; ?>:</div>
  <div class="interview-statement">
<?php if ($image): ?>
    <div class="interview-image"><?php print $image; ?></div>
<?php endif;?>
    <div class="content"><?php print $content; ?></div>
  </div>

</div>