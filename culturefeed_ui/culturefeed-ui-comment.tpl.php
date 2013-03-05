<div class="comment">
  <?php

  /* @var \CultureFeed_Activity $activity */
  $activity = $comment->activity;
  $activity_date = date("d/m/Y \o\m H:i", check_plain($activity->creationDate));

  ?>
    <div class="comment-detail">
        <div class="comment-image"><img src="<?php print $activity->depiction . '?width=50&height=50&crop=auto'; ?>" /></div>
        <div class="large"><span>&quot;</span><?php print check_plain($activity->value); ?><span>&quot;</span></div>

        <div class="small quiet">Geplaatst door <?php print check_plain($activity->nick); ?> op <?php print $activity_date; ?></div>
    </div>

  <?php if (!$activity->parentActivity): ?>
    <div class="comment-form-title">Reageer op deze beoordeling</div>
    <?php print drupal_render(culturefeed_ui_get_subcomment_form($activity->id, $context)); ?>
  <?php endif; ?>

  <?php if (!empty($comment->children)): ?>
    <div class="comment-children" style="padding-left: 60px;">
        <div class="comment-children-title">
            Reacties op deze beoordeling
        </div>
      <?php foreach ($comment->children as $child_comment): ?>
      <?php print theme('culturefeed_ui_comment', array('comment' => $child_comment, 'context' => $context)); ?>
      <?php endforeach; ?>
    </div>
  <?php endif; ?>
</div>
