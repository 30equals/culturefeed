<?php

/**
 * @file
 * Theming / preprocess functions for culturefeed_calendar.
 */

/**
 * Preprocess the variables for the calendar buttons.
 * @see culturefeed-calendar-button.tpl.php
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_button(&$variables) {

  if ($variables['button']['action'] == 'view') {
    $variables['button']['url'] = 'culturefeed/calendar';
    if ($variables['button']['location'] == 'content') {
      $variables['button']['description'] = t('This activity is added to your OuTcalendar.');
      $variables['button']['text'] = t('View OuTcalendar');
    }
    elseif ($variables['button']['location'] == 'footer') {
      $variables['button']['text'] = t('To your OuTcalendar');
    }
  }
  elseif ($variables['button']['action'] == 'add') {
    $variables['button']['url'] = 'culturefeed/calendar/add/' . $variables['button']['item_id'] . '/ajax';
    if ($variables['button']['location'] == 'content') {
      $variables['button']['text'] = t('Add to my OuTcalendar');
    }
    elseif ($variables['button']['location'] == 'footer') {
      $variables['button']['text'] = t('Add');
    }
  }

}

/**
 * Preprocess the variables for the calendar activities on my-calendar page.
 *
 * @see culturefeed-calendar-activity.tpl.php
 */
function culturefeed_calendar_preprocess_culturefeed_calendar_activity(&$variables) {

  if (isset($variables['activity'])) {
    $activity = $variables['activity'];

    $variables['event']['title'] = $activity->nodeTitle;
    $variables['event']['date'] = '';
    if(!empty($activity->value)) {
      $variables['event']['date'] = date('d-m-Y', strtotime(json_decode($activity->value)->date[0]));
    }
  }
}