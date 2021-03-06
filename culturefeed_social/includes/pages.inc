<?php
/**
 * @file
 * Page callbacks for culturefeed social.
 */

/**
 * Page callback to get the activities for current requested page.
 */
function culturefeed_social_page_ajax_get_activities() {

  $requested_page = isset($_GET['page']) ? $_GET['page'] : 1;

  $query = new CultureFeed_SearchActivitiesQuery();
  $query->max = CULTUREFEED_SOCIAL_BLOCK_USER_ACTIVITIES_COUNT;
  $query->start = $requested_page * CULTUREFEED_SOCIAL_BLOCK_USER_ACTIVITIES_COUNT;

  if (isset($_GET['culturefeed_uid'])) {
    $query->userId = $_GET['culturefeed_uid'];
  }

  // Filter on type.
  if (isset($_GET['type'])) {
    $query->type = array($_GET['type']);
  }
  else {
    $types = culturefeed_social_get_activity_filter_options(TRUE);
    $query->type = array_keys($types);
  }

  $query->private = FALSE;

  $result = new stdClass();
  $result->results = array();
  $result->new_pager_url = '';
  try {

    $activities = DrupalCultureFeed::searchActivities($query);
    $accounts = culturefeed_get_uids_for_activities($activities->objects);

    foreach ($activities->objects as $activity) {
      $result->results[] = theme('culturefeed_social_user_activity_summary', array('activity' => $activity, 'uid' => $accounts[$activity->userId]));
    }

    if (($query->max  + $query->start) < $activities->total) {
      $query_parameters = drupal_get_query_parameters($_GET, array('q'));
      $query_parameters['page'] = $requested_page + 1;
      $result->new_pager_url = url($_GET['q'], array('query' => $query_parameters));
    }

  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
  }

  // If we are applying, and we have no results. Make sure we show a no results message.
  if (isset($_GET['new_filter']) && empty($result->results)) {
    $result->results[] = t('No activities found');
  }

  drupal_json_output($result);

}

/**
 * Page callback for the management of the activities of current user.
 */
function culturefeed_social_page_activities($type = NULL) {

  $types = culturefeed_social_get_activity_filter_options(TRUE);

  // If a type was passed, check if it's valid.
  if (!empty($type) && !in_array($type, array_keys($types))) {
    drupal_not_found();
  }

  // Set page and pagelength.
  $pagelength = CULTUREFEED_SOCIAL_PAGE_USER_ACTIVITIES_COUNT;
  $page = isset($_GET['page']) ? $_GET['page'] : 0;

  // Build the query.
  $query = new CultureFeed_SearchActivitiesQuery();

  $query->userId      = DrupalCultureFeed::getLoggedInUserId();
  $query->private     = TRUE;
  $query->max         = $pagelength;
  $query->start       = $page * $pagelength;

  if (!empty($type)) {
    $query->type = array($type);
  }
  else {
    $query->type = array_keys($types);
  }

  // Do the query.
  try {

    $cf_account = DrupalCultureFeed::getLoggedInUser();
    $build['view-profile'] = array(
      '#prefix' => '<div id="view-profile">',
      '#markup' => l(t('View profile'), 'user/' . culturefeed_get_uid_for_cf_uid($cf_account->id, $cf_account->nick)),
      '#suffix' => '</div>',
    );

    $activities = DrupalCultureFeed::searchActivities($query);

  }
  catch (Exception $e) {
    return $build;
  }

  // Initialize the pager.
  pager_default_initialize($activities->total, $pagelength);

  $rows = array();

  foreach ($activities->objects as $activity) {

    $remove_path = 'culturefeed/activity/delete/' . $activity->id;
    $attributes = array(
      'class' => array('remove-link'),
      'role' => 'button',
      'data-toggle' => 'modal',
      'data-target' => '#delete-wrapper-' . $activity->id,
      'data-remote' => url($remove_path . "/ajax", array('query' => drupal_get_destination())),
    );

    $delete = array(
      '#type' => 'link',
      '#title' => t('Remove'),
      '#href' => $remove_path . '/nojs',
      '#attributes' => $attributes,
      '#options' => array('query' => drupal_get_destination()),
    );

    $privacy = array(
      '#type' => 'link',
      '#title' => $activity->private ? t('Private') : t('Public'),
      '#href' => 'culturefeed/activity/privacy/' . $activity->id . '/' . ($activity->private ? '0' : '1') . '/nojs',
      '#attributes' => array('id' => 'activity-privacy-' . $activity->id, 'class' => 'privacy-link ' . ($activity->private ? 'status-prive' : 'status-publiek'), 'title' => ($activity->private ? t('Make public') : t('Make private'))),
      '#options' => array('query' => drupal_get_destination()),
      '#ajax' => array(),
    );

    $type_text = isset($types[$activity->type]) ? $types[$activity->type]->label : '';

    $url = culturefeed_social_activity_object_path($activity->contentType, $activity->nodeId, $activity->nodeTitle);
    $data = l($activity->nodeTitle, $url);

    $rows[] = array(
      'id' => 'activity-' . $activity->id,
      'data' => array(
        $data,
        $type_text,
        format_interval(REQUEST_TIME - $activity->creationDate) . ' ' . t('ago <br />on') . ' ' . $activity->createdVia,
        drupal_render($privacy),
        drupal_render($delete) . '<div id="delete-wrapper-' . $activity->id . '"></div>',
      ),
    );
  }

  $activities = array(
    '#theme' => 'table',
    '#header' => array(t('Page'), t('Action'), t('Time and publication channel'), t('Public'), ''),
    '#rows' => $rows,
    '#empty' => t('You didn\'t perform any actions yet.'),
  );

  $facebook_privacy_toggle = NULL;
  $facebook = _culturefeed_ui_facebook_privacy_toggle();
  if ($facebook) {
    $item_list = array(
      '#theme' => 'item_list',
      '#items' => array(
        array(
          'data' => $facebook,
          'id' => 'onlineaccount-facebook',
        ),
      ),
    );
    $facebook_privacy_toggle = array(
      '#markup' => drupal_render($item_list),
    );
  }

  $build['filter'] = drupal_get_form('culturefeed_social_user_activity_filter_form', $type);
  $build['facebook_privacy_toggle'] = $facebook_privacy_toggle;
  $build['activities'] = $activities;
  $build['pager'] = array(
    '#theme' => 'pager',
  );

  return $build;

}

/**
 * Page callback to delete an activity.
 */
function culturefeed_social_page_activity_delete($id, $request_type = 'ajax') {

  $form = drupal_get_form('culturefeed_social_page_activity_delete_confirm_form', $id, $request_type);

  if ($request_type == 'ajax') {
    $output = drupal_render($form);
    print $output;
  }
  else {
    return $form;
  }

}

/**
 * Form callback to delete one activity.
 * @param array $form
 * @param array $form_state
 * @param Integer $id
 * @param String $request_type (nojs/ajax)
 */
function culturefeed_social_page_activity_delete_confirm_form($form, &$form_state, $id, $request_type) {

  try {
    $query = new CultureFeed_SearchActivitiesQuery();
    $query->activityId = $id;
    $data = DrupalCultureFeed::searchActivities($query);
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_social', $e);
  }

  $config = culturefeed_social_activity_get_config($data->objects[0]->type);

  if ($request_type == 'ajax') {
    $form['#prefix'] = '<div class="modal-header"><button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
<h6>' . t('Remove') . ' ' . $config->action . ' </h6></div>';
  }
  else {
    drupal_set_title(t('Remove') . ' ' . $config->action . '?');
  }

  $destination = drupal_get_destination();

  $form_state['id'] = $id;

  $form['body_wrapper'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="confirm-wrapper" class="modal-body">',
    '#suffix' => '</div>',
  );
  $form['body_wrapper']['page_remove_confirm'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Are you sure you want to delete your @action permanently', array('@action' => $config->action)) . '?</p>',
  );

  $form['footer_wrapper'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="modal-footer">',
    '#suffix' => '</div>',
  );

  $form['footer_wrapper']['remove'] = array(
    '#type' => 'submit',
    '#value' => t('Remove'),
    '#attributes' => array('class' => array('btn btn-danger')),
  );

  $form['footer_wrapper']['decline'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => url($destination['destination']),
    '#attributes' => array(
      'data-dismiss' => 'modal',
      'aria-hidden' => 'true',
      'class' => array('btn btn-link')
    ),
  );

  return $form;

}

/**
 * Submit handler to actually remove an activity.
 * @param array $form
 * @param array $form_state
 */
function culturefeed_social_page_activity_delete_confirm_form_submit($form, &$form_state) {

  $success = TRUE;
  $id = $form_state['id'];

  try {
    DrupalCultureFeed::deleteActivity($id);
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_social', $e);
  }
  drupal_goto();

}

/**
 * Render a form in json for ajax requests.
 */
function culturefeed_social_page_activity_report_abuse($activity_id, $request_type = 'ajax') {

  try {
    $query = new CultureFeed_SearchActivitiesQuery();
    $query->activityId = $activity_id;
    $query->private = FALSE;

    $activities = DrupalCultureFeed::searchActivities($query);
    $activity = NULL;
    $active_object = NULL;
    if ($activities->total > 0) {
      $activity = $activities->objects[0];
      $active_object = culturefeed_social_detail_object($activity);
      $build = array(drupal_get_form('culturefeed_social_report_abuse_form_' . $activity->id, $activity, $active_object));
    }

  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_social', $e);
    $build[] = array(t('An unexpected error has occurred.'));
  }

  if ($request_type == 'ajax') {
    $output = drupal_render($build);
    print $output;
  }
  else {
    return $build;
  }

}

/**
 * Render a comment form to comment on a given activity.
 * @param string $activity_id
 *   Activity id to comment on.
 * @param string $request_type
 *   Request type, ajax or not.
 */
function culturefeed_social_page_activity_comment($activity_id, $request_type = 'ajax') {

  try {
    $query = new CultureFeed_SearchActivitiesQuery();
    $query->activityId = $activity_id;
    $query->private = FALSE;

    $activities = DrupalCultureFeed::searchActivities($query);
    $activity = NULL;
    $active_object = NULL;
    if ($activities->total > 0) {
      $activity = $activities->objects[0];
      $active_object = culturefeed_social_detail_object($activity);
      $build = culturefeed_social_comment_actionbox($active_object, $activity);
    }
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_social', $e);
    $build[] = array(t('An unexpected error has occurred.'));
  }

  if ($request_type == 'ajax') {
    $output = drupal_render($build);
    print $output;
  } else {
    return $build;
  }
}

/**
 * Page callback to unfollow a node.
 */
function culturefeed_social_page_node_unfollow($content_type, $node_id, $request_type = 'ajax') {

  $success = TRUE;
  $activity_content_type = culturefeed_get_content_type($content_type);
  $node_id = urldecode($node_id);

  try {
    DrupalCultureFeed::unFollowNode($activity_content_type, $node_id, DrupalCultureFeed::getLoggedInUserId());
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_social', $e);
  }

  if ($request_type == 'ajax') {
    $commands = array();

    if ($success) {
      $commands[] = ajax_command_html('#node-status-wrapper', t('You will no longer receive updates of new comments'));
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    drupal_set_message(t('You are currently not receiving updates of new comments.'));
    drupal_goto();
  }

}

/**
 * Page callback to change the privacy settings of 1 activity.
 */
function culturefeed_social_page_activity_privacy($id, $privacy, $type = 'ajax') {

  $success = TRUE;

  try {
    DrupalCultureFeed::updateActivity($id, $privacy == 1);
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_ui', $e);
  }

  if ($type == 'ajax') {
    if ($success) {
      $commands = array();
      $privacy = array(
        '#type' => 'link',
        '#title' => $privacy == 1 ? t('Private') : t('Public'),
        '#href' => 'culturefeed/activity/privacy/' . $id . '/' . ($privacy == 1 ? '0' : '1') . '/nojs',
        '#attributes' => array('id' => 'activity-privacy-' . $id, 'class' => 'privacy-link ' . ($privacy == 1 ? 'status-prive' : 'status-publiek'), 'title' => ($privacy == 1 ? t('Make public') : t('Make private'))),
        '#options' => array('query' => drupal_get_destination()),
        '#ajax' => array(),
      );

      $commands[] = ajax_command_replace('#activity-privacy-' . $id, drupal_render($privacy));
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    drupal_goto();
  }

}

/**
 * Page callback to perform an activity (do action).
 */
function culturefeed_social_page_do($activity_type, $content_type, $node_id, $argument = '') {

  $add_count = isset($_GET['add_count']);
  $node_id = urldecode($node_id);
  $activity = _culturefeed_social_do($activity_type, $content_type, $node_id);
  $config = culturefeed_social_activity_get_config($activity_type);

  $identifier = drupal_html_id('activity-element');
  $identifier_wrapper = drupal_html_id('activity-wrapper');

  // No redirect if argument is advanced, noredirect.
  $redirect = empty($argument);

  if ($redirect) {
    if (empty($activity)) {
      drupal_set_message(t('An unexpected error has occurred.'), 'error');
    }
    $title = empty($activity->nodeTitle) ? '' : $activity->nodeTitle;
    culturefeed_social_goto($activity, $content_type, $node_id, $title);
  }
  else {

    $commands = array();

    if (!empty($activity)) {

      // If undo is not allowed, we should always show the do link.
      if ($config->undoAllowed) {
        $action_url = 'culturefeed/undo/' . $activity_type . '/' . $content_type . '/' . urlencode($node_id);
        $title = $config->subjectUndo;
      }
      else {
        $action_url = 'culturefeed/do/' . $activity_type . '/' . $content_type . '/' . urlencode($node_id);
        $title = $config->undoNotAllowedMessage;
      }

      if (!empty($argument)) {
        $action_url .= '/' . $argument;
      }

      $build = array();
      $build[$identifier] = array(
        '#id' => $identifier,
        '#type' => 'link',
        '#title' => $title,
        '#href' => $action_url,
        '#options' => array(
          'query' => array(
            'add_count' => $add_count,
          ),
        ),
        '#attributes' => array(
          'id' => $identifier,
          'class' => array($config->linkClassUndo),
          'title' => $config->subjectUndo
        ),
        '#ajax' => array(
          'wrapper' => $identifier_wrapper,
        ),
        '#prefix' => '<div id="' . $identifier_wrapper . '" class="do-link-wrapper">',
        '#suffix' => '</div>',
      );

      if (isset($_GET['title_undo'])) {
        $build[$identifier]['#options']['query']['undo_title'] = $_GET['title_undo'];
        $build[$identifier]['#title'] = $_GET['title_undo'];
      }

      if (isset($_GET['title'])) {
        $build[$identifier]['#options']['query']['title'] = $_GET['title'];
      }

      if ($argument == 'advanced') {
        drupal_alter('culturefeed_social_page_do', $activity, $config, $build);
      }

      // Add the updated total activities.
      if ($add_count) {
        $activity_count = culturefeed_social_get_activity_count($activity_type, $content_type, $node_id);
        if ($activity_count > 0) {
          $build[$identifier]['#title'] .= ' (' . $activity_count . ')';
        }
      }

      $commands[] = ajax_command_replace(NULL, render($build));

      // Give other modules the ability to add extra js commands.
      drupal_alter('culturefeed_social_page_do_ajax_commmands', $commands, $activity);
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));

  }

}

/**
 * Page callback to perform an activity (undo action).
 */
function culturefeed_social_page_undo($activity_type, $content_type, $node_id, $argument = '') {

  $node_id = urldecode($node_id);
  $add_count = isset($_GET['add_count']);
  $activity = _culturefeed_social_undo($activity_type, $content_type, $node_id);
  $config = culturefeed_social_activity_get_config($activity_type);

  $identifier = drupal_html_id('activity-element');
  $identifier_wrapper = drupal_html_id('activity-wrapper');

  // No redirect if argument is advanced, noredirect.
  $redirect = empty($argument);

  if ($redirect) {
    if (empty($activity)) {
      drupal_set_message(t('Action could not be undone.'), 'error');
    }
    culturefeed_social_goto($activity, $content_type, $node_id, $activity->nodeTitle);
  }
  else {

    $commands = array();

    if (!empty($activity)) {

      $action_url = 'culturefeed/do/' . $activity_type . '/' . $content_type . '/' . urlencode($node_id);
      if (!empty($argument)) {
        $action_url .= '/' . $argument;
      }

      $build = array(
      );
      $build[$identifier] = array(
        '#id' => $identifier,
        '#type' => 'link',
        '#title' => $config->titleDo,
        '#href' => $action_url,
        '#options' => array(
          'query' => array(
            'add_count' => $add_count,
          ),
        ),
        '#attributes' => array(
          'id' => $identifier,
          'class' => array($config->linkClassDo),
          'title' => $config->titleDo
        ),
        '#ajax' => array(
          'wrapper' => $identifier_wrapper,
        ),
        '#prefix' => '<div id="' . $identifier_wrapper . '" class="do-link-wrapper">',
        '#suffix' => '</div>',
      );

      if (isset($_GET['undo_title'])) {
        $build[$identifier]['#options']['query']['undo_title'] = $_GET['undo_title'];
      }

      if (isset($_GET['title'])) {
        $build[$identifier]['#options']['query']['title'] = $_GET['title'];
        $build[$identifier]['#title'] = $_GET['title'];
      }

      if ($argument == 'advanced') {
        drupal_alter('culturefeed_social_page_undo', $activity, $config, $build);
      }

      // Add the updated total activities.
      if ($add_count) {
        $activity_count = culturefeed_social_get_activity_count($activity_type, $content_type, $node_id);
        if ($activity_count > 0) {
          $build[$identifier]['#title'] .= ' (' . $activity_count . ')';
        }
        elseif (!isset($_GET['title'])) {
          $build[$identifier]['#title'] = $config->titleDoFirst;
        }
      }

      $commands[] =  ajax_command_replace(NULL, render($build));

      // Give other modules the ability to add extra js commands.
      drupal_alter('culturefeed_social_page_undo_ajax_commmands', $commands, $activity);
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));

  }

}

/**
 * Page callback to show the overview of notifications for current user or page.
 */
function culturefeed_social_page_notifications($page = NULL) {

  $build = array();

  try {

    // Currently get the messages till 1 year ago.
    $params = array(
      'max' => CULTUREFEED_SOCIAL_PAGE_NOTIFICATIONS_COUNT,
      'dateFrom' => date('Y-m-d', strtotime('-1 year')),
    );

    if ($page) {
      culturefeed_pages_set_page_breadcrumb($page);
      $notifications = DrupalCultureFeed::getLoggedInUserInstance()->pages()->getNotifications($page->getId(), $params);
    }
    else {
      $notifications = DrupalCultureFeed::getNotifications(DrupalCultureFeed::getLoggedInUserId(), $params);
    }

    return theme('culturefeed_social_user_notifications_page', array('notifications' => $notifications));

  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_social', $e);
  }

  return $build;

}
