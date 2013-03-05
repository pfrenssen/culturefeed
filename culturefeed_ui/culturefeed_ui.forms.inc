<?php

function culturefeed_ui_activities_filter_form($form, &$form_state, $type = NULL) {
  $form = array();

  $types = array(-1 => 'Alle acties') + culturefeed_ui_activity_types();

  $form['type'] = array(
    '#type' => 'select',
    '#title' => 'Toon enkel',
    '#default_value' => $type ? $type : -1,
    '#options' => $types,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Filter',
  );

  return $form;
}

function culturefeed_ui_activities_filter_form_submit($form, &$form_state) {
  $type = $form_state['values']['type'];

  if ($type > -1) {
    drupal_goto('culturefeed/activities/' . $type);
  }
  else {
    drupal_goto('culturefeed/activities');
  }
}

function culturefeed_ui_recommendations_filter_form($form, &$form_state, $zip = NULL) {
  $form = array();

  $geo_value = 'In de buurt van mijn <b>huidige locatie</b>';
  $geo_value .= ' (<span class="geo-location"></span>) <a href="#" class="geo-refresh">opnieuw zoeken</a>';
  $options = array(
    'geo' => $geo_value,
    'zip' => t('In de buurt van'),
  );
  $form['search'] = array(
    '#prefix' => '<p><b>Toon enkel aanbevelingen</b></p>',
    '#type' => 'radios',
    '#title' => '',
    '#default_value' => isset($_GET['type']) ? $_GET['type'] : 'zip',
    '#options' => $options,
  );

  $form['previous_search_type'] = array(
    '#type' => 'hidden',
    '#value' => isset($_GET['type']) ? $_GET['type'] : '',
  );

  $form['geo_zipcode'] = array(
    '#prefix' => '<div class="filter-fields">',
    '#type' => 'hidden',
    '#default_value' => $zip ? $zip : '',
  );

  $form['zip'] = array(
    '#type' => 'textfield',
    '#default_value' => $zip ? $zip : '',
    '#size' => 4,
  );

  if (drupal_valid_path('cnapi/autocomplete/location/cities')) {
    $form['zip']['#autocomplete_path'] = 'cnapi/autocomplete/location/cities';
  }

  $form['markup'] = array(
    '#markup' => ' + ',
  );

  $form['radius'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => isset($_GET['radius']) ? $_GET['radius'] : '',
    '#maxlength' => 3,
    '#suffix' => 'km in de omtrek',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Filter',
    '#suffix' => '</div>',
  );

  // load the extra javascript and css for the geolocation feature
  $form['#attached']['css'][] = drupal_get_path('module', 'culturefeed_ui') . '/css/geo.css';
  $form['#attached']['js'][] = 'http://maps.google.com/maps/api/js?sensor=false';
  $form['#attached']['js'][] = drupal_get_path('module', 'culturefeed_ui') . '/js/geo.js';

  return $form;
}

function culturefeed_ui_recommendations_filter_form_submit($form, &$form_state) {
  $zip = $form_state['values']['zip'];
  $zip_codes = explode(' ', $zip);
  $zip = reset($zip_codes);

  $options = array('query' => array());

  if (!empty($form_state['values']['radius']) && is_numeric($form_state['values']['radius'])) {
    $options['query']['radius'] = $form_state['values']['radius'];
  }

  $options['query']['type'] = $form_state['values']['search'];
  if ($form_state['values']['search'] == 'geo') {
    if (!empty($form_state['values']['geo_zipcode']))  {
      $zip = $form_state['values']['geo_zipcode'];
    }
  }

  if (!empty($zip)) {
    drupal_goto('culturefeed/recommendations/' . $zip, $options);
  }
  else {
    drupal_goto('culturefeed/recommendations');
  }
}

function culturefeed_ui_users_search_form() {
  $form = array();

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Naam',
    '#size' => 20,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Zoeken',
  );

  return $form;
}

function culturefeed_ui_users_search_form_submit($form, &$form_state) {
  $name = trim($form_state['values']['name']);

  if (!empty($name)) {
    $form_state['redirect'] = 'culturefeed/users/search/' . $name;
  }
  else {
    $form_state['redirect'] = 'culturefeed/users/search';
  }
}


/**
 * Implements hook_forms().
 *
 * Needed because we need multiple comment forms on the same page, but Drupal
 * currently can not handle submission of forms with the same form_id on the same page.
 *
 * @see http://drupal.org/node/766146
 * @see http://drupal.org/node/766146
 */
function culturefeed_ui_forms($form_id, $args) {
  $forms = array();

  if (preg_match('/^culturefeed_ui_subcomment_form_(.+)$/', $form_id, $matches)) {
    $forms[$form_id] = array(
      'callback' => 'culturefeed_ui_subcomment_form',
      'callback arguments' => array($matches[1]),
    );
  }

  return $forms;
}

function culturefeed_ui_get_subcomment_form($parent_activity, $context) {
  return drupal_get_form('culturefeed_ui_subcomment_form_' . $parent_activity, $context);
}

function culturefeed_ui_subcomment_form($form, &$form_state, $parent_activity, $context) {
  $form = culturefeed_ui_comment_form($form, $form_state, $context, $parent_activity);

  $form['#submit'] = array('culturefeed_ui_comment_form_submit');

  $form['comment']['#title'] = t('Reactie');
  $form['submit']['#value'] = t('Verzend reactie');

  return $form;
}

function culturefeed_ui_comment_form($form, &$form_state, $context, $parent_activity = NULL) {
  $form['context'] = array(
    '#type' => 'value',
    '#value' => $context,
  );

  $form['parent_activity'] = array(
    '#type' => 'value',
    '#value' => $parent_activity,
  );

  $form['comment'] = array(
    '#type' => 'textarea',
    '#title' => t('Schrijf zelf een beoordeling'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Verzend beoordeling'),
  );

  return $form;
}

function culturefeed_ui_comment_form_submit($form, &$form_state) {
  $comment = $form_state['values']['comment'];
  $context = $form_state['values']['context'];

  $cf = DrupalCultureFeed::getLoggedInUserInstance();

  $activity = new CultureFeed_Activity();
  $activity->type = CultureFeed_Activity::TYPE_COMMENT;
  $activity->value = $comment;
  $activity->contentType = $context['activity_content_type'];
  $activity->nodeId = $context['node_id'];
  $activity->nodeTitle = $context['node_title'];

  if ($form_state['values']['parent_activity']) {
    $activity->parentActivity = $form_state['values']['parent_activity'];
  }

  $activity->userId = DrupalCultureFeed::getLoggedInUserId();

  $cf->createActivity($activity);
}
