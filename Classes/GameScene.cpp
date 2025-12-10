#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Flame.h"
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

    // 1. 添加 GameBackground 层
    // GameBackground 内部 ZOrder: 背景(0), 网格(1), UI(2)
    _gameBG = GameBackground::create();
    this->addChild(_gameBG, 0);

    // map
    _mapLayer = MapLayer::create();
    this->addChild(_mapLayer);

    // player
    _player = Player::createPlayer();
    Vec2 startGrid(1, 1);
    Vec2 worldPos = _mapLayer->gridToWorld(startGrid.x, startGrid.y);
   
    _player->setPosition(worldPos);
    // [修改] 将 Player 添加到 _gameBG 中，ZOrder=1
    // 确保玩家被 UI 覆盖（如暂停按钮），但显示在背景和网格之上
    _gameBG->addChild(_player, 1);
    //this->addChild(_player, 10);

    // keyboard
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [&](EventKeyboard::KeyCode key, Event* event)
        {
            //如果暂停，不处理输入
            if (_gameBG && _gameBG->isGamePaused()) return;

            if (key == EventKeyboard::KeyCode::KEY_W|| key == EventKeyboard::KeyCode::KEY_UP_ARROW) keyW = true;
            if (key == EventKeyboard::KeyCode::KEY_S || key == EventKeyboard::KeyCode::KEY_DOWN_ARROW) keyS = true;
            if (key == EventKeyboard::KeyCode::KEY_A || key == EventKeyboard::KeyCode::KEY_LEFT_ARROW) keyA = true;
            if (key == EventKeyboard::KeyCode::KEY_D || key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) keyD = true;

            // 放炸弹在 Player 里
            if (key == EventKeyboard::KeyCode::KEY_SPACE)
                _player->placeBomb(this, _mapLayer);
        };

    listener->onKeyReleased = [&](EventKeyboard::KeyCode key, Event* event)
        {
            if (key == EventKeyboard::KeyCode::KEY_W || key == EventKeyboard::KeyCode::KEY_UP_ARROW) keyW = false;
            if (key == EventKeyboard::KeyCode::KEY_S || key == EventKeyboard::KeyCode::KEY_DOWN_ARROW) keyS = false;
            if (key == EventKeyboard::KeyCode::KEY_A || key == EventKeyboard::KeyCode::KEY_LEFT_ARROW) keyA = false;
            if (key == EventKeyboard::KeyCode::KEY_D || key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) keyD = false;
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    scheduleUpdate();

    return true;
}

void GameScene::update(float dt)
{
    // 暂停判断
    if (_gameBG && _gameBG->isGamePaused())
        return;

    handleInput(dt);

    if (!_player || _player->isDead)
        return;

    //
    // 🔥【精准火焰伤害判定：基于网格】
    //
    // 玩家当前所在网格
    Vec2 pGrid = _mapLayer->worldToGrid(_player->getPosition());

    // 遍历所有火焰节点
    for (auto node : this->getChildren())
    {
        Flame* flame = dynamic_cast<Flame*>(node);
        if (!flame) continue;

        // 如果火焰所在格子 = 玩家所在格子 → 命中
        if (flame->gridPos.equals(pGrid))
        {
            _player->takeDamage();
            break;
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

