<?php

/**
 * @file
 * Contains Drupal\culturefeed\Controller\UserRestController.
 */

namespace Drupal\culturefeed_udb3\Controller;

use CultuurNet\UDB3\Symfony\JsonLdResponse;
use Drupal\Core\Controller\ControllerBase;
use CultureFeed_User;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Access\AccessResult;

class UserRestController extends ControllerBase {

  /**
   * The culturefeed user service.
   *
   * @var CultureFeed_User;
   */
  protected $user;

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('culturefeed.current_user')
    );
  }

  /**
   * Constructs a RestController.
   *
   * @param CultureFeed_User $user
   *   A culturefeed user object.
   */
  public function __construct(CultureFeed_User $user) {
    $this->user = $user;
  }

  /**
   * Returns culturefeed user data.
   *
   * @return JsonLdResponse
   *   A json response.
   */
  public function info() {

    $response = JsonLdResponse::create()
      ->setData($this->user)
      ->setPublic()
      ->setClientTtl(60 * 30)
      ->setTtl(60 * 5);

    return $response;

  }

  /**
   * Checks access.
   *
   * @return AccessResult
   *   Access or not.
   */
  public function access() {
    return AccessResult::allowedIf($this->user->id)->setCacheable(FALSE);
  }

}