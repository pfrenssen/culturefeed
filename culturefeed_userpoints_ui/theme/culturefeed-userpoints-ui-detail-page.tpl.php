<?php
/**
 * @file
 *   Template file for a detail of a CultureFeed_PointsPromotion
 *   
 * @var 
 *   - $id
 *   - $title
 *   - $real_points
 *   - $maxAvailableUnits
 *   - $unitsTaken
 *   - $currentUnitsTaken : Calculated from ( $unitsTaken + $unitsTaken van huidige sessie)
 *   - $unitsLeft : Calculated from ( $maxAvailableUnits - $currentUnitsTaken)
 *   - $cashingPeriodBegin
 *   - $cashingPeriodEnd
 *   - $description1
 *   - $description2
 *   - $picture_url
 *   - $exchange_link
 *   - $exchange_url
 */

$unitsLeft = $maxAvailableUnits - $currentUnitsTaken;
?>

<h2><?php print $title ?></h2>

<p>Geldig tot <?php print $cashingPeriodEnd ?> <?php print $unitsLeft ?> x op voorraad</p>

<?php if (!empty($picture_url)): ?>
<p>
  <img style="float: right;" width="100" src="<?php print $picture_url; ?>" />
  <span><?php print $real_points ?></span>
</p>
<?php endif; ?>

<p><?php print $id ?></p>
<p><?php print $cashingPeriodBegin ?></p>
<p><?php print $description1 ?></p>
<p><?php print $description2 ?></p>

<?php if ($can_exchange && !$active): ?>
  <p>
    <span><?php print $exchange_link ?></span>
    <span><?php print $real_points ?></span>
  </p>

<?php else: ?>

<?php endif; ?>