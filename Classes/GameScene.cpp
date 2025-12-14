#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Flame.h"
#include "ItemManager.h"
#include "GameBackground.h"

USING_NS_CC;

// -----------------------------
// 静态工厂函数
// -----------------------------
Scene* GameScene::createScene()
{
    return GameScene::create();
}

// -----------------------------
// 传递模式和角色ID
// -----------------------------
GameScene* GameScene::createWithMode(GameMode mode, int player1CharId, int player2CharId)
{
    auto scene = new GameScene();
    scene->_gameMode = mode;
    scene->_player1CharacterId = player1CharId;
    scene->_player2CharacterId = player2CharId;

    if (scene && scene->init())
    {
        scene->autorelease();
        return scene;
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

// -----------------------------
// 初始化
// -----------------------------
bool GameScene::init()
{

    if (!Scene::init())
        return false;
   

    // 1. 背景
    _gameBG = GameBackground::create();
    this->addChild(_gameBG, 0);

    // 2. 地图
    _mapLayer = MapLayer::create();
    this->addChild(_mapLayer);

    // 3. 道具
    _itemManager = ItemManager::create(_mapLayer);
    this->addChild(_itemManager);

    // 4. 创建玩家
    initPlayers();

    // 5. 初始化键盘监听
    initKeyboard();

    // 6. 开启 update
    scheduleUpdate();

    return true;
}

// -----------------------------
// 初始化玩家
// -----------------------------
void GameScene::initPlayers()
{
    _players.clear();

    switch (_gameMode)
    {
    case GameMode::SINGLE:
        CCLOG("Creating Player1 at (1,1) with ID=%d", _player1CharacterId, "Player1");
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        break;

    case GameMode::LOCAL_2P:
        CCLOG("Creating Player1 at (1,1) with ID=%d", _player1CharacterId, "Player1");
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");

        CCLOG("Creating Player2 at (11,11) with ID=%d", _player2CharacterId, "Player2");
        createLocalPlayer(Vec2(11, 11), _player2CharacterId, "Player2");
        break;

    case GameMode::SINGLE_AI:
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        // TODO: createAIPlayer(...)
        break;

    case GameMode::ONLINE:
        // TODO: 网络模式，由 NetworkManager 决定玩家
        break;
    }
}


// -----------------------------
// 创建本地玩家
// -----------------------------
void GameScene::createLocalPlayer(const Vec2& gridPos, int characterId, const std::string& name)
{
    auto player = Player::createPlayer();
    if (!player)
    {
        CCLOG("Error: Failed to create player %s", name.c_str());
        return;
    }

    // 转换格子坐标到世界坐标
    Vec2 worldPos = _mapLayer->gridToWorld(gridPos.x, gridPos.y);
    CCLOG("Creating Player at grid(%f,%f) -> world(%f,%f)", gridPos.x, gridPos.y, worldPos.x, worldPos.y);
    player->setPosition(worldPos);

    // 设置角色
    player->setCharacter(characterId);

    // 给玩家一个名字，方便调试
    player->setName(name);

    // 设置层级为 10，确保不会被背景或道具覆盖
    _gameBG->addChild(player, 10);

    // 确保可见
    player->setVisible(true);
    player->setOpacity(255);

    // 加入玩家容器
    _players.push_back(player);

    // 打印日志调试位置
    CCLOG("%s created at (%.2f, %.2f) with character ID=%d", name.c_str(), worldPos.x, worldPos.y, characterId);
   

}


// -----------------------------
// update
// -----------------------------
void GameScene::update(float dt)
{
    if (_gameBG && _gameBG->isGamePaused())
        return;

    handleInput(dt);

    for (auto player : _players)
    {
        if (!player || player->isDead) continue;

        checkFlameHit(player);
        checkItemPickup(player);
    }
}

// -----------------------------
// 输入处理
// -----------------------------
void GameScene::handleInput(float dt)
{
    if (_players.size() > 0)
        handlePlayerMove(_players[0], keyW, keyS, keyA, keyD, keyBomb1, dt);

    if (_gameMode == GameMode::LOCAL_2P && _players.size() > 1)
        handlePlayerMove(_players[1], keyUp, keyDown, keyLeft, keyRight, keyBomb2, dt);

    // 未来可增加 AI 或网络玩家的 handleInput
}

// -----------------------------
// 玩家移动通用处理
// -----------------------------
void GameScene::handlePlayerMove(
    Player* player,
    bool up, bool down, bool left, bool right,
    bool& bombKey,
    float dt)
{
    if (!player || player->isDead) return;

    Vec2 dir;
    if (up) dir.y += 1;
    if (down) dir.y -= 1;
    if (left) dir.x -= 1;
    if (right) dir.x += 1;

    if (dir.length() > 0.01f)
    {
        dir.normalize();
        player->move(dir * speed * dt, _mapLayer);
    }

    if (bombKey)
    {
        player->placeBomb(this, _mapLayer);
        bombKey = false;
    }
}

// -----------------------------
// 火焰判定
// -----------------------------
void GameScene::checkFlameHit(Player* player)
{
    Vec2 pGrid = _mapLayer->worldToGrid(player->getPosition());

    for (auto node : this->getChildren())
    {
        auto flame = dynamic_cast<Flame*>(node);
        if (flame && flame->gridPos.equals(pGrid))
        {
            player->takeDamage();
            break;
        }
    }
}

// -----------------------------
// 道具拾取
// -----------------------------
void GameScene::checkItemPickup(Player* player)
{
    for (auto& item : _itemManager->items)
    {
        if (item && player->getBoundingBox().intersectsRect(item->getBoundingBox()))
        {
            item->playPickAnimation([this, player, item]()
                {
                    player->pickItem(item);
                    _itemManager->items.eraseObject(item);
                    item->removeFromParent();
                });
            break;
        }
    }
}

// -----------------------------
// 键盘监听初始化
// -----------------------------
void GameScene::initKeyboard()
{
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [&](EventKeyboard::KeyCode key, Event*)
        {
            if (_gameBG && _gameBG->isGamePaused()) return;

            if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P)
            {
                if (key == EventKeyboard::KeyCode::KEY_W) keyW = true;
                if (key == EventKeyboard::KeyCode::KEY_S) keyS = true;
                if (key == EventKeyboard::KeyCode::KEY_A) keyA = true;
                if (key == EventKeyboard::KeyCode::KEY_D) keyD = true;
                if (key == EventKeyboard::KeyCode::KEY_SPACE) keyBomb1 = true;
            }

            if (_gameMode == GameMode::LOCAL_2P)
            {
                if (key == EventKeyboard::KeyCode::KEY_UP_ARROW) keyUp = true;
                if (key == EventKeyboard::KeyCode::KEY_DOWN_ARROW) keyDown = true;
                if (key == EventKeyboard::KeyCode::KEY_LEFT_ARROW) keyLeft = true;
                if (key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) keyRight = true;
                if (key == EventKeyboard::KeyCode::KEY_ENTER ||
                    key == EventKeyboard::KeyCode::KEY_KP_ENTER)
                    keyBomb2 = true;
            }
        };

    listener->onKeyReleased = [&](EventKeyboard::KeyCode key, Event*)
        {
            if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P)
            {
                if (key == EventKeyboard::KeyCode::KEY_W) keyW = false;
                if (key == EventKeyboard::KeyCode::KEY_S) keyS = false;
                if (key == EventKeyboard::KeyCode::KEY_A) keyA = false;
                if (key == EventKeyboard::KeyCode::KEY_D) keyD = false;
                if (key == EventKeyboard::KeyCode::KEY_SPACE) keyBomb1 = false;
            }

            if (_gameMode == GameMode::LOCAL_2P)
            {
                if (key == EventKeyboard::KeyCode::KEY_UP_ARROW) keyUp = false;
                if (key == EventKeyboard::KeyCode::KEY_DOWN_ARROW) keyDown = false;
                if (key == EventKeyboard::KeyCode::KEY_LEFT_ARROW) keyLeft = false;
                if (key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) keyRight = false;
                if (key == EventKeyboard::KeyCode::KEY_ENTER ||
                    key == EventKeyboard::KeyCode::KEY_KP_ENTER)
                    keyBomb2 = false;
            }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

