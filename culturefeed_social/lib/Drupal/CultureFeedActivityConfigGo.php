<?php

class CultureFeedActivityConfigGo extends CultureFeedActivityConfigBase {

  public $type = CultureFeed_Activity::TYPE_IK_GA;

  /**
   * Constructor to load default values.
   */
  public function __construct() {
    parent::__construct();

    $this->allowedTypes = array(
      CultureFeed_Activity::CONTENT_TYPE_EVENT,
      CultureFeed_Activity::CONTENT_TYPE_PRODUCTION,
    );

    $this->titleDo = t('Attend');
    $this->titleDoFirst = t('Attend');
    $this->subjectUndo = t('Not attending');
    $this->viewPrefix = t('will be attending');
    $this->label = t('Will be attending');
    $this->loginRequiredMessage = t('You must be <a href="/culturefeed/oauth/connect">logged in</a> to indicate that you will be  attending');
    $this->pointsOverviewPrefix = t('Will be attending');

  }

}
