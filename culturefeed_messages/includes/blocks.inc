<?php
/**
 * @file
 * Block callbacks for culturefeed messages.
 */

/**
 * Show a users inbox inside a block.
 * @param mixed $page
 *   CultureFeed_Cdb_Item_Page if we want only messages to this page.
 */
function culturefeed_messages_block_inbox($page = NULL) {

  try {
    $recipientPage = empty($page) ? NULL : $page->getId();
    $messages = DrupalCultureFeed::getLoggedInUserInstance()->messages()->getMessages($recipientPage);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_messages', $e);
    return;
  }

  $block['subject'] = 'Inbox';
  $block['content'] = array(
    '#theme' => 'culturefeed_messages_message_list',
    '#messages' => $messages,
    '#add_new_allowed' => TRUE,
  );

  return $block;

}