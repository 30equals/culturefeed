<?php

/**
 * @file
 * Theming / preprocess functions for culturefeed_calendar.
 */


function culturefeed_calendar_preprocess_culturefeed_calendar_page(&$variables) {


  // Check if we need to show the calendar for the logged in user or for another user.
  if (isset($variables['user_id'])) {
    $my_calendar = FALSE;
    $cf_user = DrupalCultureFeed::getUser($variables['user_id']);
  }
  else {
    $my_calendar = TRUE;
    $cf_user = DrupalCultureFeed::getLoggedInUser();
  }

  // Check if another persons calendar is shared.
  if (!$my_calendar && empty($cf_user->calendarId)) {
    $variables['deny_access'] = TRUE;
  }
  else {
    $variables['deny_access'] = FALSE;
  }

  $authenticated = DrupalCultureFeed::isCultureFeedUser();
  $variables['sidebar'] = 'empty';

  // Top nav with month names.
  $months = _culturefeed_calendar_get_nav_months();
  $variables['nav_months'] = theme('culturefeed_calendar_nav_months', array('months' => $months));

  // Sort activities by type.
  $activities = $variables['activities'];
  $activities_by_type = array();
  if (!empty($activities->objects)) {
    foreach($activities->objects as $activity) {
      $activity_type = $activity->type;
      if (!culturefeed_calendar_get_selected_date($activity)) {
        $activity_type = CultureFeed_Activity::TYPE_LIKE;
      }
      $activities_by_type[$activity_type][] = $activity;
    }
  }

  $variables['not_yet_planned'] = '';
  $variables['planned'] = '';
  if (!empty($activities_by_type)) {
    // Print the 'LIKED' activities.
    if (isset($activities_by_type[CultureFeed_Activity::TYPE_LIKE]) && $my_calendar) {
      $variables['not_yet_planned'] = theme('culturefeed_calendar_activities_liked',
        array('activities' => $activities_by_type[CultureFeed_Activity::TYPE_LIKE]));
    }
    // Print the 'IK GA' activities.
    if (isset($activities_by_type[CultureFeed_Activity::TYPE_IK_GA])) {
      $variables['planned'] = theme('culturefeed_calendar_activities_going',
        array('activities' => $activities_by_type[CultureFeed_Activity::TYPE_IK_GA], 'month_names' => $months, 'my_calendar' => $my_calendar));
    }
    // Print the save cookie button.
    if (!$authenticated && !empty($activities->objects) && $my_calendar) {
      $save_cookie_button_form = drupal_get_form('culturefeed_calendar_save_cookie_form');
      $variables['save_cookie_button'] = drupal_render($save_cookie_button_form);
    }
  }
  if ($authenticated) {
    // Set username.
    $variables['user_name'] = $cf_user->givenName . ' ' . $cf_user->familyName;
    if ($my_calendar) {
      $variables['share_calendar_url'] = 'calendar/share';
      $variables['calendar_settings_url'] = 'calendar/settings';

      if (!empty($cf_user->calendarId)) {
        $variables['shared'] = TRUE;
      }
      else {
        $variables['shared'] = FALSE;
        $variables['data_content'] = theme('culturefeed_calendar_share_page');
      }
    }
  }

}

/**
 * Preprocess the variables for the calendar buttons.
 * @see culturefeed-calendar-button.tpl.php
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_button(&$variables) {

  drupal_add_js(array(
    'culturefeed' => array(
      'currentEventId' => $variables['button']['item_id'],
    ),
  ), 'setting');

  drupal_add_library('system', 'drupal.ajax');
  drupal_add_library('system', 'jquery.cookie');

  if (!culturefeed_is_culturefeed_user()) {
    drupal_add_css(drupal_get_path('module', 'culturefeed_calendar') . '/css/culturefeed_calendar_anonymous.css');
  }

  if ($variables['button']['action'] == 'view') {

    $variables['button']['class'] = 'view-calendar';
    $variables['button']['path'] = 'culturefeed/calendar';
    if ($variables['button']['location'] == 'content') {
      $variables['button']['description'] = t('This event is added to your calendar.');
      $variables['button']['text'] = t('View calendar');
    }
    elseif ($variables['button']['location'] == 'footer') {
      $variables['button']['text'] = t('to your calendar');
    }

  }
  elseif ($variables['button']['action'] == 'add') {

    $variables['button']['class'] = 'add-to-calendar';
    $variables['button']['path'] = 'culturefeed/calendar/add/' . $variables['button']['item_id'] . '/nojs';
    if ($variables['button']['location'] == 'content') {
      $variables['button']['text'] = t('Add to my calendar');
    }
    elseif ($variables['button']['location'] == 'footer') {
      $variables['button']['text'] = t('Add to calendar');
    }

  }

}

/**
 * Theme the total activities profile box item.
 */
function theme_culturefeed_calendar_total_activities_profile_box_item($variables) {
  return l(format_plural($variables['total'], '<small class="activity-count">@count</small> unsaved event', '<small class="activity-count">@count</small> unsaved events'), 'culturefeed/calendar', array('html' => TRUE));
}

/**
 * Preprocess activities 'going'.
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_activities_going(&$variables) {

  // Get activities and sort them by date to add them to the calendar in the right order.
  $activities = $variables['activities'];
  usort($activities, "culturefeed_calendar_sort_by_date");

  $months = $variables['month_names'];

  $variables['months'] = array_fill_keys(array_map(function($element){return $element['full_month'];}, $months), array());

  foreach ($activities as $activity) {
    if ($selected_date = culturefeed_calendar_get_selected_date($activity)) {
      $month = format_date($selected_date, 'custom', 'F');
      $variables['months'][$month][] = $activity;
    }
  }
}

/**
 * Preprocess activity mini summary.
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_activity_mini(&$variables) {

  $activity = $variables['activity'];
  $node_id = $activity->nodeId;

  $activity_id = $activity->id;
  $item = culturefeed_search_item_load($node_id, 'event');
  $variables['item'] = $item;

  module_load_include('inc', 'culturefeed_agenda', 'theme/theme');
  culturefeed_agenda_preprocess_culturefeed_event_summary($variables);

  // Plan link.
  $variables['edit_link']['url'] = url('culturefeed/calendar/edit/'. $activity_id . '/nojs');
  $variables['edit_link']['text'] = t('Inplannen');
  // Remove link.
  $variables['delete_link']['url'] = url('culturefeed/calendar/delete/'. $activity_id . '/nojs');
  $variables['delete_link']['text'] = t('Verwijderen');

}

/**
 * Preprocess activity summary.
 *
 * @param type $variables
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_activity_summary(&$variables) {

  $activity = $variables['activity'];
  $item = culturefeed_search_item_load($activity->nodeId, 'event');
  $variables['item'] = $item;

  module_load_include('inc', 'culturefeed_agenda', 'theme/theme');
  $function = 'culturefeed_agenda_preprocess_culturefeed_event_summary';
  if (function_exists($function)) {
    $function($variables);
  }

  // Activity details.
  if ($selected_date = culturefeed_calendar_get_selected_date($activity)) {
    $variables['date'] = format_date($selected_date, 'custom', 'l j F Y H:i');
  }

  $activity_id = $activity->id;

  // Edit link.
  $variables['edit_link']['url'] = url('culturefeed/calendar/edit/'. $activity_id . '/nojs');
  $variables['edit_link']['text'] = t('Verplaatsen');
  $variables['edit_link']['show'] = culturefeed_calendar_event_has_multiple_dates($variables['calendar']);
  // Remove link.
  $variables['delete_link']['url'] = url('culturefeed/calendar/delete/'. $activity_id . '/nojs');
  $variables['delete_link']['text'] = t('Verwijderen');

}

/**
 * Preprocess for add button tootlip.
 *
 * @param type $variables
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_button_hover(&$variables) {

  $variables['url'] = 'culturefeed/oauth/connect';
  $variables['options'] = array(
    'query' => array(
      'destination' => 'culturefeed/calendar/add-cookie-to-calendar',
    ),
  );
}

/**
 * Preprocess the share calendar page.
 *
 * @param type $variables
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_share_page(&$variables) {

  try {
    $cf_user = DrupalCultureFeed::getUser(DrupalCultureFeed::getLoggedInUserId());
  }
  catch (Exception $ex) {
    drupal_set_message(t('There was an error while searching the user'), 'error');
  }

  global $base_url;
  $variables['back_to_calendar_url'] =  $base_url . '/culturefeed/calendar';

  $variables['shared'] = $cf_user->calendarId;
  // Calendar is public.
  if ($cf_user->calendarId != '') {

    $variables['calendar_share_url'] = 'culturefeed/calendar/' . $cf_user->id;
    $variables['calendar_full_share_url'] = $base_url . '/culturefeed/calendar/' . $cf_user->id;

    $fb_app_id = variable_get('culturefeed_calendar_fbAppId', '');
    if (isset($fb_app_id) && !empty($fb_app_id)) {
      drupal_add_js(drupal_get_path('module', 'culturefeed_calendar') . '/js/facebook_share.js');
      drupal_add_js(array('culturefeed' => array('fbAppId' => $fb_app_id)), 'setting');
    }

    $variables['facebook']['url'] = 'https://www.facebook.com/sharer/sharer.php';
    $variables['facebook']['attr'] = array(
      'query' => array('u' => $base_url . '/' . $variables['calendar_share_url']),
      'attributes' => array('class' => 'facebook-share'),
      );

    $variables['googleplus']['url'] = 'https://plus.google.com/share';
    $variables['googleplus']['attr'] = array('query' => array('url' => $base_url . '/' .  $variables['calendar_share_url']));

    $variables['twitter']['url'] = 'https://twitter.com/intent/tweet';
    $variables['twitter']['attr'] = array('query' => array('text' => $base_url . '/' .  $variables['calendar_share_url']));

    $variables['mail']['enabled'] = module_exists('forward');
    $variables['mail']['url'] = $base_url . '/forward';
    $variables['mail']['attr'] = array('query' => array('path' => $variables['calendar_share_url']));

  }
}

/**
 * Preprocess the calendar settings page.
 *
 * @param type $variables
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_settings_page(&$variables) {
  global $base_url;
  $variables['back_to_calendar_url'] =  $base_url . '/culturefeed/calendar';
  $variables['calendar_settings_form'] = drupal_get_form('culturefeed_calendar_settings_form');
}

