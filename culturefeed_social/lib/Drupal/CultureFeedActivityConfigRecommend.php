<?php

class CultureFeedActivityConfigRecommend extends CultureFeedActivityConfigBase {

  public $type = CultureFeed_Activity::TYPE_RECOMMEND;

  /**
   * Constructor to load default values.
   */
  public function __construct() {
    parent::__construct();

    $this->allowedTypes = array(
      CultureFeed_Activity::CONTENT_TYPE_EVENT,
      CultureFeed_Activity::CONTENT_TYPE_PRODUCTION,
      CultureFeed_Activity::CONTENT_TYPE_ACTOR,
      CultureFeed_Activity::CONTENT_TYPE_ACTIVITY,
      CultureFeed_Activity::CONTENT_TYPE_BOOK,
      CultureFeed_Activity::CONTENT_TYPE_NODE,
      CultureFeed_Activity::CONTENT_TYPE_CULTUREFEED_PAGE,
    );

    $this->subject = t('Recommended by');
    $this->subjectDo = t('Recommended activity');
    $this->subjectUndo = t('Undo recommend');
    $this->titleDo = t('Also recommend');
    $this->titleDoFirst = t('Be the first to recommend');
    $this->titleShowAll = t('View all users who recommend this');
    $this->linkClassDo = 'recommend-link';
    $this->linkClassUndo = 'unrecommend-link';
    $this->viewPrefix = t('has');
    $this->viewSuffix = t('recommended');
    $this->label = t('Recommend');
    $this->loginRequiredMessage = t('You must be <a href="/autenticated">logged in</a> to recommend this item');
    $this->pointsOverviewSuffix = t('recommended');
  }

}