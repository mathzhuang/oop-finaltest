#include "GameScene.h"
#include "Player.h"

USING_NS_CC;

Player* player;

Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;

    // 创建玩家
    player = Player::createPlayer();
    player->setPosition(200, 200); // 玩家初始位置
    this->addChild(player);

    // 启用键盘监听
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [](EventKeyboard::KeyCode key, Event* event) {
        switch (key)
        {
        case EventKeyboard::KeyCode::KEY_W:
            player->moveUp();
            break;
        case EventKeyboard::KeyCode::KEY_S:
            player->moveDown();
            break;
        case EventKeyboard::KeyCode::KEY_A:
            player->moveLeft();
            break;
        case EventKeyboard::KeyCode::KEY_D:
            player->moveRight();
            break;
        default:
            break;
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}
