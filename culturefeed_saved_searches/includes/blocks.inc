<?php
/**
 * @file
 * Block callbacks for the saved searches module.
 */

/**
 * CTA block to save current search.
 */
function culturefeed_saved_searches_block_cta() {

  if ($query = culturefeed_saved_searches_get_cta_query()) {
    return theme('culturefeed_saved_searches_cta', array('text' => t('and receive an email when there are new results'), 'query' => $query));
  }

}

/**
 * CTA block to save current search, at the bottom of search results.
 */
function culturefeed_saved_searches_block_cta_bottom() {

  $results = culturefeed_get_search_result();

  if ($results && $results->getCurrentCount() > 0) {
    if ($query = culturefeed_saved_searches_get_cta_query()) {
      return theme('culturefeed_saved_searches_cta', array('text' => t('to search faster next time'), 'query' => $query));
    }
  }

}

/**
 * Helper function that constructs the 'save search' query string.
 */
function culturefeed_saved_searches_get_cta_query() {

  $query = drupal_static(__FUNCTION__, NULL);
  if ($query != NULL) {
    return $query;
  }

  $query = array();

  if ($page = culturefeed_search_get_active_search_page()) {

    $option_active = FALSE;

    // Check if a custom filter option is active.
    $filter_options = variable_get('culturefeed_search_filter_options', culturefeed_search_ui_default_filter_options());
    foreach ($filter_options as $option) {
      if (isset($_GET[$option['query-string']])) {
        $option_active = TRUE;
      }
    }

    // Check if we are searching.
    if (!$option_active && !isset($_GET['facet']) && !isset($_GET['location']) && !isset($_GET['search']) && !isset($_GET['regId']) && !isset($_GET['actor'])) {
      return $query;
    }

    // Add page title.
    $query['title'] = drupal_get_title();

    // Add search parameters in string.
    $supported_parameters = array('fq', 'q', 'zipcode');
    $parameters = $page->getParameters();

    $has_date_filter = FALSE;
    $params = array();
    foreach ($parameters as $param) {
      $key = $param->getKey();

      if (in_array($key, $supported_parameters)) {
        $params[] = $key . '=' . $param->getValue();
      }
      elseif ($key == 'datetype') {
        $has_date_filter = TRUE;
      }
    }

    $query['query'] = implode('&', $params);
    if ($has_date_filter) {
      $query['datefilter'] = TRUE;
    }

    if (!culturefeed_is_culturefeed_user()) {
      return $query;
    }

    // We need to fetch the whole list of saved searches since we don't have ID.
    $savedSearchList = DrupalCultureFeed::getLoggedInUserInstance()->savedSearches()->getList();
    $exists = FALSE;
    foreach ($savedSearchList as $savedSearch) {
      if ($savedSearch->query == $query['query']) {
        $exists = TRUE;
      }
    }

    if (!$exists) {
      return $query;
    }

  }
}

/**
 * Render a block with a (short) list of saved searches.
 */
function culturefeed_saved_searches_block_teaser() {

  if (!culturefeed_is_culturefeed_user()) {
    return;
  }

  $block = array();

  try {

    $list = DrupalCultureFeed::getLoggedInUserInstance()->savedSearches()->getList();
    if (empty($list)) {
      return $block;
    }

    $block['content'] = array(
      '#theme' => 'culturefeed_saved_searches_list',
      '#searches' => $list,
    );

  }
  catch (Exception $e) {
    drupal_set_message(t('A problem occurred while loading your searches. Please try again later.'), 'error');
  }

  return $block;

}
