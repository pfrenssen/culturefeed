<?php
/**
 * @file
 * Drupal cache layer for the CultureFeed Search Service.
 */

use \CultuurNet\Search\Guzzle\Service;
use \CultuurNet\Auth\ConsumerCredentials;

/**
 * DrupalCultureFeedSearchService_Cache
 */
class DrupalCultureFeedSearchService_Cache extends DrupalCultureFeedSearchService {

  /**
   * @var Integer
   */
  protected $loggedInUserId;

  /**
   * @var \CultuurNet\Search\Guzzle\Service
   */
  protected $realSearchService;

  /**
   * Consumer.
   * @var \CultuurNet\Auth\ConsumerCredentials
   */
  protected $consumer;

  /**
   * Constructor.
   *
   * @param Service $realSearchService
   * @param ConsumerCredentials $consumerCredentials
   * @param Integer $loggedInUserId
   */
  public function __construct(Service $realSearchService, $consumerCredentials, $loggedInUserId) {
    $this->loggedInUserId = $loggedInUserId;
    $this->consumerCredentials = $consumerCredentials;
    $this->realSearchService = $realSearchService;
  }

  public function getRealSearchService() {
    return $this->realSearchService();
  }

  protected function getCachePrefix() {
    return 'culturefeed:results:';
  }

  protected function getCacheSuffix() {
    // Don't cache per user. Search is the same for every user. (for now)
    return '';
  }

  protected function getCacheCid($cid) {
    return self::getCachePrefix() . $cid . self::getCacheSuffix();
  }

  protected function cacheSet($cid, $data, $expires = CACHE_PERMANENT) {
    $cid = $this->getCacheCid($cid);
    cache_set($cid, $data, 'cache_culturefeed_search', $expires);
  }

  protected function cacheGet($cid) {
    $cid = $this->getCacheCid($cid);
    return cache_get($cid, 'cache_culturefeed_search');
  }

  protected function cacheClear($cid = NULL, $wildcard = FALSE) {
    cache_clear_all($cid, 'cache_culturefeed_search', $wildcard);
  }

  /**
   * @return ConsumerCredentials
   */
  protected function getConsumer() {
    $this->consumer;
  }

  /**
   * @see \CultuurNet\Search\Service::search().
   */
  public function search(Array $parameters = array()) {

    DrupalCultureFeedSearchService::addLanguageParameter($parameters);

    $cid = 'search:' . md5(serialize($parameters));
    if ($cache = $this->cacheGet($cid)) {
      $result = $cache->data;
      return $result;
    }

    $result = $this->realSearchService->search($parameters);

    // Translate categories.
    DrupalCultureFeedSearchService::translateCategories($result);

    $this->cacheSet($cid, $result, REQUEST_TIME + CULTUREFEED_SEARCH_CACHE_EXPIRES);

    return $result;

  }

  /**
   * @see \CultuurNet\Search\Service::search().
   */
  public function searchPages(Array $parameters = array()) {

    $cid = 'search/page:' . md5(serialize($parameters));
    if ($cache = $this->cacheGet($cid)) {
      $result = $cache->data;
      return $result;
    }

    $result = $this->realSearchService->searchPages($parameters);

    $this->cacheSet($cid, $result, REQUEST_TIME + CULTUREFEED_SEARCH_CACHE_EXPIRES);

    return $result;

  }

  /**
   * @see \CultuurNet\Search\Service::searchSuggestions().
   */
  public function searchSuggestions($search_string, $types = array()) {

    $cid = sprintf('suggestions:%s', md5($search_string . implode('|', $types)));
    if ($cache = $this->cacheGet($cid)) {
      return $cache->data;
    }

    $suggestions = $this->realSearchService->searchSuggestions($search_string, $types);
    $this->cacheSet($cid, $suggestions, REQUEST_TIME + CULTUREFEED_SEARCH_CACHE_EXPIRES);

    return $suggestions;

  }

}