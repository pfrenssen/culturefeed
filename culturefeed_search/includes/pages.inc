<?php
/**
 * @file
 * Defines page callback to render the search results.
 */

use \Guzzle\Http\Exception;

/**
 * Autcomplete callback for the suggestions on Cdb items.
 */
function culturefeed_search_suggestions_autocomplete_page($search_string, $type = NULL) {

  $matches = array();

  if ($search_string) {

    try {

      $types = isset($type) ? array($type) : array();

      // Get the list of suggestions from service.
      $result = culturefeed_get_search_service()->searchSuggestions($search_string, $types);
      if ($result->hasSuggestions()) {
        $suggestions = $result->getSuggestions();
        foreach ($suggestions as $suggestion) {
          $matches[$suggestion] = check_plain($suggestion);
        }
      }

    }
    catch (ClientErrorResponseException $e) {
      watchdog_exception('culturefeed_search', $e);
    }
    catch (Exception $e) {
      watchdog_exception('culturefeed_search', $e);
    }

  }

  drupal_json_output($matches);

  // Cache if possible per request.
  drupal_page_footer();
}

/**
 * Page callback for the region suggestions autocomplete.
 */
function culturefeed_search_region_suggestion_autocomplete_page($search_string) {

  $matches = array();

  if ($search_string) {

    try {

      // Get the list of suggestions from service.
      $suggestions = culturefeed_search_get_flandersregion_suggestions($search_string);
      foreach ($suggestions as $key => $suggestion) {
        $matches[$key] = check_plain($suggestion);
      }
    }
    catch (ClientErrorResponseException $e) {
      watchdog_exception('culturefeed_search', $e);
    }

  }

  drupal_json_output($matches);

  // Cache if possible per request.
  drupal_page_footer();
}

/**
 * Page callback for the city suggestions autocomplete.
 */
function culturefeed_search_city_suggestion_autocomplete_page($search_string, $include_regions = TRUE) {

  $matches = array();

  if ($search_string) {

    // Get the list of suggestions from service.
    $suggestions = culturefeed_search_get_city_suggestions($search_string, $include_regions);
    foreach ($suggestions as $key => $suggestion) {
      $matches[$key] = check_plain($suggestion);
    }

  }

  drupal_json_output($matches);

  // Cache if possible per request.
  drupal_page_footer();
}