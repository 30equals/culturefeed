<?php

function culturefeed_ui_block_view_search_users() {
  $block['subject'] = t('Search users');
  $block['content'] = drupal_get_form('culturefeed_ui_search_users_form');

  return $block;
}

function culturefeed_ui_block_view_profile_box() {
  if (!user_is_anonymous() && !DrupalCultureFeed::isCultureFeedUser()) {
    return;
  }

  $nick = '';
  $picture = '';
  $url_profile = '';

  if (!user_is_anonymous()) {
    try {
      $cf_account = DrupalCultureFeed::getLoggedInUser();
    }
    catch (Exception $e) {
      culturefeed_log('culturefeed_ui', $e);
    }

    if (isset($cf_account)) {
      $nick = $cf_account->nick;
      $picture = $cf_account->depiction;
      $url_profile = 'user/' . culturefeed_get_uid_for_cf_uid($cf_account->id, $cf_account->nick);
    }
  }

  $block['subject'] = '';
  $block['content'] = array(
    '#theme' => 'culturefeed_ui_profile_box',
    '#nick' => $nick,
    '#picture' => $picture,
    '#url_profile' => $url_profile,
  );

  return $block;
}

function culturefeed_ui_block_view_connect() {
  if (!user_is_anonymous()) {
    return;
  }

  $block['subject'] = 'Maak zelf een UiTid';
  $block['content'] = array(
    '#theme' => 'culturefeed_ui_connect',
  );

  return $block;
}

function culturefeed_ui_block_view_most_active_user() {
  $sort = variable_get('culturefeed_ui_block_most_active_user_sort', CultureFeed_SearchUsersQuery::SORT_NUMBEROFACTIVEACTIVITIES);

  $query = new CultureFeed_SearchUsersQuery();
  $query->max = 1;
  $query->order = CultureFeed_SearchUsersQuery::SORT_ORDER_DESCENDING;
  $query->sort = $sort;

  try {
    $account = DrupalCultureFeed::searchUsers($query)->objects[0];
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  $block['subject'] = 'UiTslover van de week';
  $block['content'] = array(
    '#theme' => 'culturefeed_ui_most_active_user',
    '#account' => $account,
    '#sort' => $sort,
    '#sort_value' => $account->sortValue,
    '#url_view_all' => 'culturefeed/users/mostactive',
  );

  return $block;
}

function culturefeed_ui_block_view_top_events() {
  $sort = variable_get('culturefeed_ui_block_top_events_sort', CultureFeed::TOP_EVENTS_SORT_ACTIVE);

  try {
    $events = DrupalCultureFeed::getTopEvents($sort, 5); // @todo make max configurable
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  $items = array();

  $events_info = culturefeed_get_nodes_for_cdbids('event', $events);

  foreach ($events as $cdbid) {
    if (!isset($events_info[$cdbid]) || !is_array($events_info[$cdbid])) {
      continue;
    }

    $event = $events_info[$cdbid];

    $request = array('event' => $cdbid, 'title' => $event['title']);

    $items[] = cnapi_url_dp2dul($event['title'], $request);
  }

  // @todo make this themeable
  $block['subject'] = t('Top events');
  $block['content'] = array(
    '#theme' => 'item_list',
    '#items' => $items,
    '#type' => 'ol',
  );

  return $block;
}

function culturefeed_ui_block_view_similar_users() {
  $account = menu_get_object('user');

  if (!$account || !DrupalCultureFeed::isCultureFeedUser($account->uid)) {
    return;
  }

  try {
    $cf_account = DrupalCultureFeed::getConsumerInstance()->getUser($account->culturefeed_uid);

    $users = DrupalCultureFeed::getSimilarUsers($cf_account->id);
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  if (empty($users->objects)) {
    return;
  }

  $items = array();
  
  $accounts = culturefeed_get_uids_for_users($users->objects);

  foreach ($users->objects as $user) {
    if (!isset($accounts[$user->id])) {
      continue;
    }
    
    $items[] = theme('culturefeed_ui_user_summary', array('user' => $user, 'uid' => $accounts[$user->id]));
  }

  $block['subject'] = t('Users similar to !name', array('!name' => $cf_account->nick));
  $block['content'] = array(
    '#theme' => 'item_list',
    '#items' => $items,
  );

  return $block;
}

function culturefeed_ui_block_view_user_picture() {
  global $user;

  $account = menu_get_object('user');

  if (!$account || !DrupalCultureFeed::isCultureFeedUser($account->uid)) {
    return;
  }

  try {
    if ($user->uid == $account->uid) {
      $cf_account = DrupalCultureFeed::getLoggedInUser();
    }
    else {
      $cf_account = DrupalCultureFeed::getConsumerInstance()->getUser($account->culturefeed_uid);
    }
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  $path = $cf_account->depiction;

  $block['subject'] = '';
  $block['content'] = array(
    array(
      '#theme' => 'image',
      '#path' => $path,
    ),
  );

  if ($user->uid == $account->uid) {
    $block['content'][] = array('#markup' => l(t('Change picture'), 'culturefeed/profile/edit'));
  }

  return $block;
}

function culturefeed_ui_block_view_profile_menu() {
  if (!DrupalCultureFeed::isCultureFeedUser()) {
    return;
  }

  $items = array();

  $title = 'Mijn UiTid';
  $url = 'user/' . culturefeed_get_or_create_user(DrupalCultureFeed::getLoggedInUserId())->uid;
  $description = 'Zo zien anderen jouw profiel.';

  $items[] = theme('culturefeed_ui_profile_menu_item', array('title' => $title, 'url' => $url, 'description' => $description));

  $title = 'Profiel bewerken';
  $url = 'culturefeed/profile/edit';
  $description = 'Wijzig naam, adres, favoriet categoriëen, ...';

  $items[] = theme('culturefeed_ui_profile_menu_item', array('title' => $title, 'url' => $url, 'description' => $description));

  $title = 'Account bewerken';
  $url = 'culturefeed/account/edit';
  $description = 'Wijzig paswoord, e-mailadres, toepassingen verbonden met jouw account, ...';

  $items[] = theme('culturefeed_ui_profile_menu_item', array('title' => $title, 'url' => $url, 'description' => $description));

  $block['subject'] = t('Profile');
  $block['content'] = array(
    '#theme' => 'item_list',
    '#items' => $items,
  );

  return $block;
}

function culturefeed_ui_block_view_user_activities() {
  global $user;

  $account = menu_get_object('user');

  if (!$account || !DrupalCultureFeed::isCultureFeedUser($account->uid)) {
    return;
  }

  $query = new CultureFeed_SearchActivitiesQuery();
  $query->max = 200;
  $query->contentType = CultureFeed_Activity::CONTENT_TYPE_EVENT;
  $query->type = array(
    CultureFeed_Activity::TYPE_LIKE,
    CultureFeed_Activity::TYPE_FACEBOOK,
    CultureFeed_Activity::TYPE_TWITTER,
    CultureFeed_Activity::TYPE_IK_GA
  );
  $query->userId = $account->culturefeed_uid;
  if ($user->uid == $account->uid) {
    $query->private = TRUE;
  }

  try {
    $activities = DrupalCultureFeed::searchActivities($query);
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  $items = array();

  $events = culturefeed_get_nodes_for_activities($activities->objects);
  
  $accounts = culturefeed_get_uids_for_activities($activities->objects);

  foreach ($activities->objects as $activity) {
    if (!isset($accounts[$activity->userId]) || !isset($events[$activity->nodeId])) {
      continue;
    }

    $items[] = theme('culturefeed_ui_activity_summary', array('activity' => $activity, 'node' => $events[$activity->nodeId], 'uid' => $accounts[$activity->userId]));
  }

  $block['subject'] = t('User activities');
  $block['content'] = array(
    array(
      '#theme' => 'item_list',
      '#items' => $items,
    ),
  );

  return $block;
}

function culturefeed_ui_block_view_recent_activities() {
  $query = new CultureFeed_SearchActivitiesQuery();
  $query->max = 5; // @todo make this configurable
  $query->contentType = CultureFeed_Activity::CONTENT_TYPE_EVENT;
  $query->type = array(
    CultureFeed_Activity::TYPE_LIKE,
    CultureFeed_Activity::TYPE_FACEBOOK,
    CultureFeed_Activity::TYPE_TWITTER,
    CultureFeed_Activity::TYPE_IK_GA
  );

  try {
    $activities = DrupalCultureFeed::searchActivities($query);
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  $items = array();

  $events = culturefeed_get_nodes_for_activities($activities->objects);
  
  $accounts = culturefeed_get_uids_for_activities($activities->objects);

  foreach ($activities->objects as $activity) {
    if (!isset($accounts[$activity->userId]) || !isset($events[$activity->nodeId])) {
      continue;
    }

    $items[] = theme('culturefeed_ui_activity_summary', array('activity' => $activity, 'node' => $events[$activity->nodeId], 'uid' => $accounts[$activity->userId]));
  }

  $block['subject'] = t('Recent activities');
  $block['content'] = array(
    array(
      '#theme' => 'item_list',
      '#items' => $items,
    ),
    array(
      '#markup' => l(t('Older activities'), 'culturefeed/activities/recent'),
    ),
  );

  return $block;
}

function culturefeed_ui_block_view_event_like() {
  $request = cnapi_ui_get_active_request();

  if (!isset($request['event'])) {
    return;
  }

  $query = new CultureFeed_SearchActivitiesQuery();
  $query->type = CultureFeed_Activity::TYPE_LIKE;
  $query->nodeId = $request['event'];

  try {
    $activities = DrupalCultureFeed::searchActivities($query);
  }
  catch (Exception $e) {
    culturefeed_log('culturefeed_ui', $e);
    return;
  }

  $items = array();
  foreach ($activities->objects as $activity) {
    $items[] = $activity->nick . theme('image', array('path' => $activity->depiction));
  }

  /*$cf = DrupalCultureFeed::getLoggedInUserInstance();

  $activity = new CultureFeed_Activity();
  $activity->contentType = 'event';
  $activity->type = CultureFeed_Activity::TYPE_LIKE;
  $activity->nodeId = $request['event'];
  $activity->userId = $account->culturefeed_uid;

  $cf->createActivity($activity);*/

  $block['subject'] = t('Likes');
  $block['content'] = array(
    '#theme' => 'item_list',
    '#items' => $items,
  );

  return $block;
}