<?php

class CultureFeed_Uitpas_Passholder_PointsPromotion extends CultureFeed_Uitpas_ValueObject {

  /**
   * ID of the advantage object.
   *
   * @var int
   */
  public $id;

  /**
   * Title of the advantage object.
   *
   * @var string
   */
  public $title;

  /**
   * The number of points of the advantage object
   *
   * @var int
   */
  public $points;

  /**
   * True is the advantage object has been cashed in.
   *
   * @var boolean
   */
  public $cashedIn;

  /**
   * The counters of the promotion item
   *
   * @var array
   */
  public $counters;

  /**
   * The creation date of the promotion
   *
   * @var integer
   */
  public $creationDate;

  /**
   * The date when the cashing for the promotion will start
   *
   * @var integer
   */
  public $cashingPeriodBegin;

  /**
   * The date when the cashing for the promotion will end
   *
   * @var integer
   */
  public $cashingPeriodEnd;

  /**
   * The cities for which the promotion applies
   *
   * @var array
   */
  public $validForCities = array();

  /**
   * The amount of units available for this promotion
   *
   * @var integer
   */
  public $maxAvailableUnits;

  /**
   * The constraint on the cash-in
   *
   * @var CultureFeed_Uitpas_Passholder_PeriodConstraint
   */
  public $periodConstraint;

  /**
   * The amount of units taken for this promotion
   *
   * @var integer
   */
  public $unitsTaken;

  /*
   * The cash-in state of the welcome advantage
   *
   * @var boolean
   */
  public $cashInState;

  public static function createFromXML(CultureFeed_SimpleXMLElement $object) {
    $promotion = new CultureFeed_Uitpas_Passholder_PointsPromotion();
    $promotion->id = $object->xpath_int('id');
    $promotion->title = $object->xpath_str('title');
    $promotion->points = $object->xpath_int('points');
    $promotion->cashedIn = $object->xpath_bool('cashedIn');
    $promotion->counters = array();
    foreach ($object->xpath('balies/balie') as $counter) {
      $promotion->counters[] = CultureFeed_Uitpas_Passholder_Counter::createFromXML($counter);
    }
    $promotion->creationDate = $object->xpath_time('creationDate');
    $promotion->cashingPeriodBegin = $object->xpath_time('cashingPeriodBegin');
    $promotion->cashingPeriodEnd = $object->xpath_time('cashingPeriodEnd');
    $promotion->cashingPeriodBegin = $object->xpath_time('cashingPeriodBegin');
    $promotion->validForCities = $object->xpath_str('validForCities/city', TRUE);
    $promotion->maxAvailableUnits = $object->xpath_int('maxAvailableUnits');
    $promotion->unitsTaken = $object->xpath_int('unitsTaken');
    $promotion->cashInState = $object->xpath_str('cashInState');
    $periodConstraint = $object->xpath('periodConstraint', FALSE);
    if (!empty($periodConstraint)) {
      $promotion->periodConstraint = CultureFeed_Uitpas_Passholder_PeriodConstraint::createFromXml($periodConstraint);
    }

    return $promotion;
  }

}