<?php

interface CultureFeedSearchPageInterface {
  public function setResultsPerPage($resultsPerPage);
  public function setFullPage($fullPage);
  public function setPagerType($pagerType);
  public function getDefaultSort();
  public function setDefaultSort($key);
  public function loadPage();
  public function getDrupalTitle();
  public function getActiveTrail();
  public function setDefaultTitle($title);
}