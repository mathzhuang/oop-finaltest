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
    this->addChild(_mapLayer, 1); // z=1

    // 3. 道具
    _itemManager = ItemManager::create(_mapLayer);
    _itemManager->setName("ItemManager");
    this->addChild(_itemManager, 5); // z=5


    // 4. 创建玩家
    initPlayers();

    // 5. 初始化键盘监听
    initKeyboard();

    // 6. 开启 update
    scheduleUpdate();

    _gameOver = false;
    _canCheckGameOver = false;

    // 延迟 0.2 秒再允许判定
    this->runAction(Sequence::create(
        DelayTime::create(0.2f),
        CallFunc::create([this]() {
            _canCheckGameOver = true;
            }),
        nullptr
    ));


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
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");

        createAIPlayer(Vec2(11, 1), 2, "AI_1");
        createAIPlayer(Vec2(1, 11), 3, "AI_2");
        createAIPlayer(Vec2(11, 11), 4, "AI_3");

        _aiStates.resize(3); // 3 个 AI
        break;


    case GameMode::LOCAL_2P:
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        createLocalPlayer(Vec2(11, 11), _player2CharacterId, "Player2");

        createAIPlayer(Vec2(1, 11), 3, "AI_2");
        createAIPlayer(Vec2(11, 1), 4, "AI_3");

        _aiStates.resize(2);
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

    // 直接加到场景上，层级为 10
    this->addChild(player, 10);


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

    // ⭐ AI 只在有 AI 的模式运行
    if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P)
    {
        updateAI(dt);
    }

    for (auto player : _players)
    {
        if (!player || player->isDead) continue;
        checkFlameHit(player);
        checkItemPickup(player);
    }
        // ⭐⭐⭐ 加这一行 ⭐⭐⭐
        checkGameOver();
}

// -----------------------------
// ai
// -----------------------------
void GameScene::createAIPlayer(const Vec2& gridPos,
    int characterId,
    const std::string& name)
{
    auto player = Player::createPlayer();
    if (!player) return;

    Vec2 worldPos = _mapLayer->gridToWorld(gridPos.x, gridPos.y);
    player->setPosition(worldPos);

    player->setCharacter(characterId);
    player->setName(name);

    // ⭐⭐ 唯一区别 ⭐⭐
    player->isAI = true;

    this->addChild(player, 10);

    _players.push_back(player);
}

void GameScene::updateAI(float dt)
{
    int aiIndex = 0;

    for (int i = 0; i < _players.size(); ++i)
    {
        Player* p = _players[i];
        if (!p || p->isDead) continue;

        // 只处理 AI
        if (!p->isAI)
            continue;


        thinkForAI(aiIndex, p, dt);

        auto& s = _aiStates[aiIndex];

        if (!p->isMoving && s.nextDir != Vec2::ZERO)
        {
            Vec2 grid = _mapLayer->worldToGrid(p->getPosition());
            p->tryMoveTo(grid + s.nextDir, _mapLayer);
        }

        if (s.wantBomb)
        {
            p->placeBomb(this, _mapLayer);
            s.wantBomb = false;
        }

        aiIndex++;
    }
}
void GameScene::thinkForAI(int aiIndex, Player* ai, float dt)
{
    auto& s = _aiStates[aiIndex];

    s.thinkCooldown -= dt;
    if (s.thinkCooldown > 0)
        return;

    s.thinkCooldown = 0.4f;   // AI 每 0.4 秒思考一次
    s.nextDir = Vec2::ZERO;
    s.wantBomb = false;

    Vec2 grid = _mapLayer->worldToGrid(ai->getPosition());

    // 1️⃣ 脚下是否有危险（火焰 / 炸弹）
    if (isGridDanger(grid))
    {
        std::vector<Vec2> dirs = {
            Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1)
        };

        for (auto& d : dirs)
        {
            Vec2 next = grid + d;
            if (_mapLayer->isWalkable(next.x, next.y) && !isGridDanger(next))
            {
                s.nextDir = d;
                return;
            }
        }
    }

    // 2️⃣ 附近是否有人 → 放炸弹
    for (auto p : _players)
    {
        if (p == ai || p->isDead) continue;

        Vec2 pg = _mapLayer->worldToGrid(p->getPosition());
        if (pg.distance(grid) <= 1)
        {
            s.wantBomb = true;
            return;
        }
    }

    // 3️⃣ 默认：随机走（但只走合法格）
    std::vector<Vec2> validDirs;
    for (auto d : { Vec2(1,0),Vec2(-1,0),Vec2(0,1),Vec2(0,-1) })
    {
        Vec2 next = grid + d;
        if (_mapLayer->isWalkable(next.x, next.y))
            validDirs.push_back(d);
    }

    if (!validDirs.empty())
        s.nextDir = validDirs[random(0, (int)validDirs.size() - 1)];
}
bool GameScene::isGridDanger(const Vec2& grid)
{
    // 遍历当前场景所有子节点
    for (auto node : this->getChildren())
    {
        auto flame = dynamic_cast<Flame*>(node);
        if (!flame) continue;

        // Flame 已经有 gridPos
        if (flame->gridPos.equals(grid))
            return true;
    }

    return false;
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

    // ⭐ 关键：如果正在走，就不再处理输入
    if (!player->isMoving)
    {
        Vec2 dir = Vec2::ZERO;

        if (up)    dir = Vec2(0, 1);
        else if (down)  dir = Vec2(0, -1);
        else if (left)  dir = Vec2(-1, 0);
        else if (right) dir = Vec2(1, 0);

        if (dir != Vec2::ZERO)
        {
            Vec2 grid = _mapLayer->worldToGrid(player->getPosition());
            player->tryMoveTo(grid + dir, _mapLayer);
        }
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
//结束游戏检测
void GameScene::checkGameOver()
{
    if (!_canCheckGameOver) return;
    if (_gameOver) return;

    int aliveHumanCount = 0;
    int aliveAICount = 0;
    Player* lastHuman = nullptr;

    for (auto p : _players)
    {
        if (!p || p->isDead) continue;

        if (p->isAI)
            aliveAICount++;
        else
        {
            aliveHumanCount++;
            lastHuman = p;
        }
    }

    // ① 人类全灭 → 失败
    if (aliveHumanCount == 0)
    {
        _gameOver = true;
        onGameOver(nullptr);   // LOSE
        return;
    }

    // ② AI 全灭 → 胜利
    if (aliveAICount == 0)
    {
        _gameOver = true;
        onGameOver(lastHuman); // WIN
        return;
    }
}



void GameScene::onGameOver(Player* winner)
{
    unscheduleUpdate();

    for (auto p : _players)
        if (p) p->stopAllActions();

    if (winner)
        CCLOG("YOU WIN");
    else
        CCLOG("YOU LOSE");

    this->runAction(Sequence::create(
        DelayTime::create(2.0f),
        CallFunc::create([]() {
            Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, Scene::create())
            );
            }),
        nullptr
    ));
}


