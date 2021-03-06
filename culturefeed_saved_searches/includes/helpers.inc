<?php

/**
 * Helper functions for culturefeed_saved_searches.
 */

/**
 * Get the frequency options for saved searches.
 */
function culturefeed_saved_searches_frequency_options() {

  return array(
    CultureFeed_SavedSearches_SavedSearch::ASAP => t('immediately'),
    CultureFeed_SavedSearches_SavedSearch::DAILY => t('daily overview'),
    CultureFeed_SavedSearches_SavedSearch::WEEKLY => t('weekly overview'),
    CultureFeed_SavedSearches_SavedSearch::NEVER => t('never'),
  );

}

/**
 * Get the query for the search url of a saved search.
 */
function culturefeed_saved_search_get_search_url_query($search) {

  // Check for custom filter options.
  $filter_options = variable_get('culturefeed_search_filter_options', culturefeed_search_ui_default_filter_options());
  $filter_options += variable_get('culturefeed_search_filter_options_2', culturefeed_search_ui_default_filter_options());
  $known_queries = array();
  foreach ($filter_options as $filter) {
    $known_queries[$filter['api-filter-query']] = $filter['query-string'];
  }

  $query = array();
  foreach (explode('&', $search->query) as $key => $value) {

    $param = explode('=', $value);
    if ($param[0] == 'group') {
      continue;
    }

    // Param q = the search
    if ($param[0] == 'q') {
      if ($param[1] == '*:*') {
        continue;
      }
      $query['search'] = $param[1];
    }
    // Search on location
    elseif ($param[0] == 'zipcode') {
      $query['location'] = $param[1];
    }
    // Filterqueries are most of the time facets.
    elseif ($param[0] == 'fq') {

      // First check if the query is a custom filter option.
      if (isset($known_queries[$param[1]])) {
        $query[$known_queries[$param[1]]] = TRUE;
        continue;
      }

      $conditions = explode(' OR ', $param[1]);
      foreach ($conditions as $condition) {

        // Also check for every part of the condition.
        if (isset($known_queries[$condition])) {
          $query[$known_queries[$condition]] = TRUE;
          continue;
        }

        $search_on = explode(':', $condition);

        $facet_field = $search_on[0];
        $facet_field = str_replace(array('(', ')', '"'), '', $facet_field);

        $facet_value = isset($search_on[1]) ? $search_on[1] : '';
        $facet_value = str_replace(array('(', ')', '"'), '', $facet_value);

        // Check what type of filter it is.
        switch ($facet_field) {

          // Agefrom facet.
          case 'agefrom':
            $query['min_age'] = culturefeed_saved_search_get_agefrom_value($facet_value);
            break;

          // Filtering on an actor.
          case 'actor':
            $query['actor'] = $facet_value;

          // Filtering on location.
          case 'category_flandersregion_name':
            $query['location'] = $facet_value;
            break;

          // Normal facets.
          case 'category_id':
          case 'type':

            $values = explode(',', $facet_value);

            // Region facets are location searches.
            if (substr($facet_value, 0, 4) == 'reg.') {

              $term = culturefeed_search_get_term_by_id($facet_value, LANGUAGE_NONE);
              $term_parts = explode(' ', $term->name);

              // If first part is numeric: do location search.
              if (is_numeric($term_parts[0])) {
                $query['regId'] = $facet_value;
                $query['location'] = culturefeed_search_get_term_translation($facet_value);
              }
              // Otherwise set flandersregion facet active.
              else {
                $query['facet']['flandersregion'][0] = $facet_value;
              }
            }
            else {
              if (!isset($query['facet'][$search_on[0]])) {
                $query['facet'][$search_on[0]] = array();
              }
              $query['facet'][$search_on[0]] = array_merge($query['facet'][$search_on[0]], $values);
            }

            if (isset($query['facet']['type'])) {
              foreach ($query['facet']['type'] as $key => $content_type) {
                if (in_array($content_type, array('production', 'event'))) {
                  unset($query['facet']['type'][$key]);
                }
              }

            }

            break;

          default:
            // unsupported filter options.

        }


      }

    }

  }

  // Get the correct categorie facets.
  if (isset($query['facet']['category_id'])) {
    $terms = db_select('culturefeed_search_terms', 't')
      ->condition('tid', $query['facet']['category_id'])
      ->condition('language', LANGUAGE_NONE)
      ->fields('t', array('tid', 'did'))->execute();

    foreach ($terms as $term) {
      $query['facet']['category_' . $term->did . '_id'][] = $term->tid;
      unset($query['facet']['category_id']);
    }

  }

  return $query;

}

/**
 * Get the agefrom querystring to be set active.
 */
function culturefeed_saved_search_get_agefrom_value($facet_value) {

  $facet_value = 'agefrom:' . $facet_value;

  $options = culturefeed_search_ui_get_age_range_facet_options();
  foreach ($options as $key => $option) {
    $option['query'] = str_replace(array('(', ')', '"'), '', $option['query']);
    if ($option['query'] == $facet_value) {
      return $key;
    }
  }
}