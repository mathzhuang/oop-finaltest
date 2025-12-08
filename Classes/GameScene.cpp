#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "cocos2d.h"


USING_NS_CC;


Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;

    // 地图
    _mapLayer = MapLayer::create();
    this->addChild(_mapLayer);

    // 玩家（位置放在网格中心）
    _player = Player::createPlayer();
    Vec2 startGrid(1, 1);
    Vec2 startWorld = _mapLayer->gridToWorld((int)startGrid.x, (int)startGrid.y);
    // 加上地图在屏幕中的偏移量
    startWorld += _mapLayer->getPosition();
    _player->setPosition(startWorld);
    this->addChild(_player);

    // 键盘监听（设置按键状态）
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [=](EventKeyboard::KeyCode key, Event* event) {
        if (key == EventKeyboard::KeyCode::KEY_W||
            key== EventKeyboard::KeyCode::KEY_UP_ARROW||
            key==EventKeyboard::KeyCode::KEY_DPAD_UP) keyW = true;
        if (key == EventKeyboard::KeyCode::KEY_S||
            key == EventKeyboard::KeyCode::KEY_DOWN_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_DOWN) keyS = true;
        if (key == EventKeyboard::KeyCode::KEY_A||
            key == EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_LEFT) keyA = true;
        if (key == EventKeyboard::KeyCode::KEY_D||
            key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_RIGHT) keyD = true;

        // 空格放炸弹
        if (key == EventKeyboard::KeyCode::KEY_SPACE)
            _player->placeBomb();
        };

    listener->onKeyReleased = [=](EventKeyboard::KeyCode key, Event* event) {
        if (key == EventKeyboard::KeyCode::KEY_W ||
            key == EventKeyboard::KeyCode::KEY_UP_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_UP) keyW = false;
        if (key == EventKeyboard::KeyCode::KEY_S ||
            key == EventKeyboard::KeyCode::KEY_DOWN_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_DOWN) keyS = false;
        if (key == EventKeyboard::KeyCode::KEY_A ||
            key == EventKeyboard::KeyCode::KEY_LEFT_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_LEFT) keyA = false;
        if (key == EventKeyboard::KeyCode::KEY_D ||
            key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW ||
            key == EventKeyboard::KeyCode::KEY_DPAD_RIGHT) keyD = false;

        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    scheduleUpdate();

    return true;
}

void GameScene::update(float dt)
{
    // 玩家移动（如果你有）
    handleInput(dt);


    // --- 检查火焰伤害 ---
    auto children = this->getChildren();
    for (auto c : children)
    {
        if (c->getTag() == 300)   // Flame
        {
            if (_player && !_player->isDead)
            {
                if (_player->getBoundingBox().intersectsRect(c->getBoundingBox()))
                {
                    _player->takeDamage();   // 改这里！
                }
            }

        }
    }
}


void GameScene::handleInput(float dt)
{
    Vec2 dir(0, 0);
    if (keyW) dir.y += 1;
    if (keyS) dir.y -= 1;
    if (keyA) dir.x -= 1;
    if (keyD) dir.x += 1;

    if (dir.x != 0 || dir.y != 0)
    {
        dir.normalize();
        _player->move(dir, _mapLayer);
    }
}

void GameScene::placeBomb()
{
    auto bomb = Sprite::create("bomb(1).png");
    bomb->setPosition(_player->getPosition());
    this->addChild(bomb);

    bomb->runAction(
        Sequence::create(
            DelayTime::create(2.0f),
            CallFunc::create([this, bomb]() {

                auto boom = Sprite::create("explosion(1).png");
                boom->setPosition(bomb->getPosition());
                this->addChild(boom);

                boom->runAction(
                    Sequence::create(
                        FadeOut::create(0.4f),
                        RemoveSelf::create(),
                        nullptr
                    )
                );

                bomb->removeFromParent();
                }),
            nullptr
        )
    );
}
