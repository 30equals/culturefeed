<?php

/**
 * Helper functions for calendar.
 */

/**
 * Get the users activities.
 */
function culturefeed_calendar_get_user_activities() {

  $uid = DrupalCultureFeed::getLoggedInUserId();

  if ($uid) {
    // Only search for activities with type_like or type_ik_ga.
    $activity_options = array(
      CultureFeed_Activity::TYPE_LIKE,
      CultureFeed_Activity::TYPE_IK_GA,
    );

    // Load the config for the activity options.
    $options = array();
    foreach ($activity_options as $activity_type) {
      $options[$activity_type] = CultureFeedActivityConfigBase::loadByType($activity_type);
    }

    /*$service = culturefeed_get_search_service();
    $parameters = array();
    $parameters[] = new CultuurNet\Search\Parameter\Query('*:*');
    $parameters[] = new CultuurNet\Search\Parameter\FilterQuery('type:event');
    $parameters[] = new CultuurNet\Search\Parameter\FilterQuery('like_users:' . $uid);
    $parameters[] = new CultuurNet\Search\Parameter\FilterQuery('attend_users:' . $uid);

    $result = $service->search($parameters);
    */

    $query = new CultureFeed_SearchActivitiesQuery();
    $query->type = $activity_options;
    $query->contentType = 'event';
    $query->userId = DrupalCultureFeed::getLoggedInUserId();

    try {
      $activities = DrupalCultureFeed::searchActivities($query);
    }
    catch (Exception $e) {
      watchdog_exception('culturefeed_calendar', $e);
      return;
    }

    if ($activities->total == 0) {
      return;
    }
  }
  else {
    $activities = new stdClass();
    $activities->objects = array();
    $activities->total = 0;

    if(isset($_COOKIE['calendar'])) {
      $activities->objects = unserialize($_COOKIE['calendar']);
      $activities->total = count($activities->objects);
    }
  }

  return $activities;
}

/**
 * Render the button to go to the calendar page or add item form.
 */
function culturefeed_calendar_render_button($item, $location) {

  $id = $item->getId();
  $ids_to_check = array();
  $activities = culturefeed_calendar_get_user_activities();
  foreach($activities->objects as $key => $activity) {
    $ids_to_check[] = $activity->nodeId;
  }

  if (in_array($id, $ids_to_check)) {
    $button['action'] = 'view';
  }
  else {
    $button['action'] = 'add';
  }

  $button['location'] = $location;
  $button['item_id'] = $id;

  return theme('culturefeed_calendar_button', array('button' => $button));
}

/**
 * Helper function to add an activity to the calendar.
 */
function culturefeed_calendar_add_activity($node_id, $node_title, $date) {

  $authenticated = DrupalCultureFeed::isCultureFeedUser();
  $activity = new CultureFeed_Activity();

  if (!empty($date)) {
    $activity->value = strtotime($date);
    $activity->type = CultureFeed_Activity::TYPE_IK_GA;
  }
  else {
    $activity->type = CultureFeed_Activity::TYPE_LIKE;
  }

  $activity->contentType = 'event';
  $activity->nodeId = $node_id;
  $activity->nodeTitle = $node_title;


  if ($authenticated) {
    $activity->userId = DrupalCultureFeed::getLoggedInUserId();

    DrupalCultureFeed::createActivity($activity);
  }
  else {
    $cookie = array();
    if (isset($_COOKIE ['calendar'] )) {
      $cookie['calendar'] = unserialize($_COOKIE ['calendar']);
    }

    $cookie['calendar'][] = $activity;
    $cookie['calendar'] = serialize($cookie['calendar']);
    user_cookie_save($cookie);
  }
  drupal_set_message(t('The event has been added to your OuTcalendar.'));
  drupal_goto('agenda/e/' . $node_title . '/' . $node_id);
}