#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#define TAG_FLAME 300

USING_NS_CC;

Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;

    // map
    _mapLayer = MapLayer::create();
    this->addChild(_mapLayer);

    // player
    _player = Player::createPlayer();
    Vec2 startGrid(1, 1);
    Vec2 worldPos = _mapLayer->gridToWorld(startGrid.x, startGrid.y);
   
    _player->setPosition(worldPos);
    this->addChild(_player, 10);

    // keyboard
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [&](EventKeyboard::KeyCode key, Event* event)
        {
            if (key == EventKeyboard::KeyCode::KEY_W) keyW = true;
            if (key == EventKeyboard::KeyCode::KEY_S) keyS = true;
            if (key == EventKeyboard::KeyCode::KEY_A) keyA = true;
            if (key == EventKeyboard::KeyCode::KEY_D) keyD = true;

            // 放炸弹在 Player 里
            if (key == EventKeyboard::KeyCode::KEY_SPACE)
                _player->placeBomb(this, _mapLayer);
        };

    listener->onKeyReleased = [&](EventKeyboard::KeyCode key, Event* event)
        {
            if (key == EventKeyboard::KeyCode::KEY_W) keyW = false;
            if (key == EventKeyboard::KeyCode::KEY_S) keyS = false;
            if (key == EventKeyboard::KeyCode::KEY_A) keyA = false;
            if (key == EventKeyboard::KeyCode::KEY_D) keyD = false;
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    scheduleUpdate();

    return true;
}

void GameScene::update(float dt)
{
    handleInput(dt);

    if (!_player || _player->isDead)
        return;

    // 检查火焰伤害
    for (auto node : this->getChildren())
    {
        if (!node) continue;

        if (node->getTag() == TAG_FLAME)
        {
            auto flame = dynamic_cast<Sprite*>(node);
            if (!flame) continue;

            // 玩家与火焰碰撞
            if (_player->getBoundingBox().intersectsRect(flame->getBoundingBox()))
            {
                _player->takeDamage();
            }
        }
    }
}


void GameScene::handleInput(float dt)
{
    if (!_player || _player->isDead) return;

    Vec2 dir(0, 0);

    if (keyW) dir.y += 1;
    if (keyS) dir.y -= 1;
    if (keyA) dir.x -= 1;
    if (keyD) dir.x += 1;

    if (dir.length() > 0.01f)
    {
        dir.normalize();
        Vec2 delta = dir * speed * dt;   // 带 dt 的速度
        _player->move(delta, _mapLayer); // 精准移动
    }
}

