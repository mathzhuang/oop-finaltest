#include "GameScene.h"
USING_NS_CC;

Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;

    // ´´½¨Íæ¼Ò
    player = Player::createPlayer();
    player->setPosition(200, 200);
    this->addChild(player);

    // ¼üÅÌ¼àÌý
    auto listener = EventListenerKeyboard::create();

    // °´ÏÂ
    listener->onKeyPressed = [=](EventKeyboard::KeyCode key, Event* event) {
        if (key == EventKeyboard::KeyCode::KEY_W) keyW = true;
        if (key == EventKeyboard::KeyCode::KEY_S) keyS = true;
        if (key == EventKeyboard::KeyCode::KEY_A) keyA = true;
        if (key == EventKeyboard::KeyCode::KEY_D) keyD = true;

        if (key == EventKeyboard::KeyCode::KEY_SPACE)
            placeBomb();
        };

    // ËÉ¿ª
    listener->onKeyReleased = [=](EventKeyboard::KeyCode key, Event* event) {
        if (key == EventKeyboard::KeyCode::KEY_W) keyW = false;
        if (key == EventKeyboard::KeyCode::KEY_S) keyS = false;
        if (key == EventKeyboard::KeyCode::KEY_A) keyA = false;
        if (key == EventKeyboard::KeyCode::KEY_D) keyD = false;
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // ¿ªÆô update
    scheduleUpdate();

    return true;
}

void GameScene::update(float dt)
{
    float dx = 0;
    float dy = 0;

    if (keyW) dy += speed * dt;
    if (keyS) dy -= speed * dt;
    if (keyA) dx -= speed * dt;
    if (keyD) dx += speed * dt;

    if (dx != 0 || dy != 0)
    {
        player->setPosition(player->getPosition() + Vec2(dx, dy));
    }
}
void GameScene::placeBomb()
{
    auto bomb = Sprite::create("bomb(1).png");   // »»³ÉÄãµÄÕ¨µ¯Í¼
    bomb->setPosition(player->getPosition());
    this->addChild(bomb);

    bomb->runAction(
        Sequence::create(
            DelayTime::create(2.0f),
            CallFunc::create([=]() {

                auto boom = Sprite::create("explosion(1).png"); // ±¬Õ¨Í¼
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
