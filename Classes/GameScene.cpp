#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Flame.h"
#include "ItemManager.h"
#include "GameBackground.h"
#include "AIController.h"
#include "FogManager.h"
#include"GameOverLayer.h"

#include <algorithm> 
#include <vector>
#include <queue>
#include <map>

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
    this->addChild(_mapLayer, 1);

    // 2.5 迷雾（仅 FOG 模式）
    // 创建 FogManager
    if (_gameMode == GameMode::FOG)
    {
        _fogManager = FogManager::create();
        this->addChild(_fogManager, 100); // 确保在最上层
        _fogManager->initFog(Director::getInstance()->getVisibleSize(), 150.0f);

    }

    // 3. 道具
    _itemManager = ItemManager::create(_mapLayer);
    _itemManager->setName("ItemManager");
    this->addChild(_itemManager, 5);

    _aiController = new AIController(this);

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

    // 1. 准备所有可用的角色 ID 池
    std::vector<int> availableIds = { 1, 2, 3, 4 };

    // 辅助 lambda：从池中移除指定 ID
    auto removeId = [&](int id) {
        availableIds.erase(std::remove(availableIds.begin(), availableIds.end(), id), availableIds.end());
        };

    switch (_gameMode)
    {
    case GameMode::SINGLE:
    {
        // --- 创建 P1 ---
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        removeId(_player1CharacterId); // 剔除 P1 的皮肤

        // --- 创建 AI (使用剩余皮肤) ---
        // 单人模式需要 3 个 AI
        if (availableIds.size() >= 3)
        {
            createAIPlayer(Vec2(11, 1), availableIds[0], "AI_1");
            createAIPlayer(Vec2(1, 11), availableIds[1], "AI_2");
            createAIPlayer(Vec2(11, 11), availableIds[2], "AI_3");
        }
        _aiStates.resize(3);
        break;
    }

    case GameMode::LOCAL_2P:
    {
        // --- 创建 P1 ---
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        removeId(_player1CharacterId);

        // --- 创建 P2 ---
        createLocalPlayer(Vec2(11, 11), _player2CharacterId, "Player2");
        removeId(_player2CharacterId);

        // --- 创建 AI (使用剩余皮肤) ---
        // 双人模式需要 2 个 AI
        if (availableIds.size() >= 2)
        {
            createAIPlayer(Vec2(1, 11), availableIds[0], "AI_2");
            createAIPlayer(Vec2(11, 1), availableIds[1], "AI_3");
        }
        _aiStates.resize(2);
        break;
    }

    case GameMode::FOG:
    {
        // --- 创建 P1 ---
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        removeId(_player1CharacterId);

        // --- 创建 AI ---
        // 迷雾模式需要 2 个 AI
        if (availableIds.size() >= 2)
        {
            createAIPlayer(Vec2(11, 1), availableIds[0], "AI_1");
            createAIPlayer(Vec2(1, 11), availableIds[1], "AI_2");
        }
        _aiStates.resize(2);
        break;
    }

    case GameMode::ONLINE:
        // TODO: 网络模式
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

    // AI 更新
    if ((_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P|| _gameMode == GameMode::FOG) && _aiController)
        updateAI(dt);

    // Fog 模式安全更新
    if (_gameMode == GameMode::FOG && _fogManager)
    {
        if (_fogManager->getParent() && !_players.empty() && _players[0] && !_players[0]->isDead)
        {
            _fogManager->updateFog(_players[0]);
        }
    }

    for (auto player : _players)
    {
        if (!player || player->isDead) continue;
        checkFlameHit(player);
        checkItemPickup(player);
    }

    if (_canCheckGameOver) checkGameOver();
}

// -----------------------------
// ai
// -----------------------------
Player* GameScene::findNearestPlayer(Player* self)
{
    Player* nearest = nullptr;
    float minDist = FLT_MAX;

    Vec2 selfGrid = _mapLayer->worldToGrid(self->getPosition());

    for (auto p : _players)
    {
        if (!p || p == self || p->isDead) continue;

        Vec2 pg = _mapLayer->worldToGrid(p->getPosition());
        float d = selfGrid.distance(pg);

        if (d < minDist)
        {
            minDist = d;
            nearest = p;
        }
    }
    return nearest;
}



std::vector<Vec2> GameScene::findPathBFS(
    const Vec2& start,
    std::function<bool(const Vec2&)> isTarget,
    bool avoidDanger)
{
    struct BFSNode {
        Vec2 pos;
        std::vector<Vec2> path;
    };

    std::queue<BFSNode> q;
    auto cmp = [](const Vec2& a, const Vec2& b) {
        return a.x == b.x ? a.y < b.y : a.x < b.x;
        };
    std::map<Vec2, bool, decltype(cmp)> visited(cmp);

    q.push({ start, {} });
    visited[start] = true;

    std::vector<Vec2> dirs = {
        Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1)
    };

    while (!q.empty())
    {
        BFSNode node = q.front(); q.pop();

        if (isTarget(node.pos))
            return node.path;

        for (auto d : dirs)
        {
            Vec2 next(node.pos.x + d.x, node.pos.y + d.y);

            if (!_mapLayer->isWalkable(next.x, next.y)) continue;
            if (visited.find(next) != visited.end()) continue;
            if (avoidDanger && isGridDanger(next)) continue;

            visited[next] = true;

            BFSNode nextNode = { next, node.path };
            nextNode.path.push_back(d);
            q.push(nextNode);
        }
    }


    return {};
}
std::vector<Vec2> GameScene::findSafePathBFS(const Vec2& start)
{
    return findPathBFS(
        start,
        [&](const Vec2& p) {
            return !isGridDanger(p);
        },
        true
    );
}
std::vector<Vec2> GameScene::findPathToPlayer(const Vec2& start, Player* target)
{
    Vec2 targetGrid = _mapLayer->worldToGrid(target->getPosition());

    return findPathBFS(
        start,
        [&](const Vec2& p) {
            return p == targetGrid;
        },
        true
    );
}
std::vector<Vec2> GameScene::findPathToItem(const Vec2& start)
{
    return findPathBFS(
        start,
        [&](const Vec2& p) {
            return _itemManager->hasItemAtGrid(p);
        },
        true
    );
}
std::vector<Vec2> GameScene::findPathToSoftWall(const Vec2& start)
{
    return findPathBFS(
        start,
        [&](const Vec2& p) {
            return _mapLayer->isNearSoftWall(p);
        },
        true
    );
}




void GameScene::createAIPlayer(const Vec2& gridPos,
    int characterId,
    const std::string& name)
{
    auto player = Player::createPlayer();
    if (!player) return;

    // -------------------
    // 设置位置与角色
    // -------------------
    Vec2 worldPos = _mapLayer->gridToWorld(gridPos.x, gridPos.y);
    player->setPosition(worldPos);
    player->setCharacter(characterId);
    player->setName(name);

    // -------------------
    // AI 专属标记
    // -------------------
    player->isAI = true;

    // -------------------
    // 🔥 初始化 AI 性格参数
    // -------------------
    player->aiAggressive = 0.3f + CCRANDOM_0_1() * 0.6f;  // 0.3~0.9
    player->aiCoward = 0.1f + CCRANDOM_0_1() * 0.6f;      // 0.1~0.7
    player->aiCuriosity = 0.3f + CCRANDOM_0_1() * 0.6f;   // 0.3~0.9

    // -------------------
    // 添加到场景与玩家列表
    // -------------------
    this->addChild(player, 10);
    _players.push_back(player);

    CCLOG("AI Player '%s' created at (%f,%f) Agg=%.2f Coward=%.2f Curious=%.2f",
        name.c_str(), worldPos.x, worldPos.y,
        player->aiAggressive, player->aiCoward, player->aiCuriosity);
}

void GameScene::updateAI(float dt)
{
    int aiIndex = 0;

    for (auto p : _players)
    {
        if (!p || p->isDead || !p->isAI) continue;

        _aiController->updateAI(dt, p, _aiStates[aiIndex]);

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




bool GameScene::hasSafeEscape(const Vec2& grid, Player* ai)
{
    for (auto d : { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) })
    {
        Vec2 next = grid + d;
        if (_mapLayer->isWalkable(next.x, next.y) && !isGridDanger(next))
            return true;
    }
    return false;
}
bool GameScene::isGridDanger(const cocos2d::Vec2& grid)
{
    for (auto node : this->getChildren())
    {
        auto flame = dynamic_cast<Flame*>(node);
        if (flame && flame->gridPos.equals(grid))
            return true;

        auto bomb = dynamic_cast<Bomb*>(node);
        if (bomb && bomb->willExplodeGrid(grid))
            return true;
    }

    return false;
}

bool GameScene::isPlayerCornered(Player* player)
{
    Vec2 grid = _mapLayer->worldToGrid(player->getPosition());
    int blocked = 0;

    for (auto d : { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) })
    {
        Vec2 next = grid + d;
        if (!_mapLayer->isWalkable(next.x, next.y) || isGridDangerPublic(next))
            blocked++;
    }

    return blocked >= 2; // 两边被堵就算角落
}

bool GameScene::willBombTrapPlayer(const Vec2& bombGrid, Player* target, int bombRange)
{
    Vec2 tg = _mapLayer->worldToGrid(target->getPosition());

    // 简单启发式：目标与炸弹在同一行或同一列，并且在炸弹范围内
    if (bombGrid.x == tg.x && abs(bombGrid.y - tg.y) <= bombRange) return true;
    if (bombGrid.y == tg.y && abs(bombGrid.x - tg.x) <= bombRange) return true;

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

    for (auto node : _mapLayer->getChildren()) // 🔹 遍历 MapLayer 的子节点
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
    if (!player || !_itemManager) return;

    auto& items = _itemManager->getItems();
    for (int i = items.size() - 1; i >= 0; --i)
    {
        Item* item = items.at(i);
        if (!item) continue;

        // 输出调试信息
        CCLOG("Player: %s pos=(%.2f, %.2f) parent=%s",
            player->getName().c_str(),
            player->getPositionX(), player->getPositionY(),
            player->getParent() ? player->getParent()->getName().c_str() : "NULL");

        CCLOG("Item: type=%d pos=(%.2f, %.2f) parent=%s",
            static_cast<int>(item->getType()),
            item->getPositionX(), item->getPositionY(),
            item->getParent() ? item->getParent()->getName().c_str() : "NULL");

        Rect playerRect = player->getBoundingBox();
        playerRect.origin = player->getParent()->convertToWorldSpace(playerRect.origin);

        Rect itemRect = item->getBoundingBox();
        itemRect.origin = item->getParent()->convertToWorldSpace(itemRect.origin);

        if (playerRect.intersectsRect(itemRect))
        {
            CCLOG("Item picked up! Type=%d", static_cast<int>(item->getType()));

            items.erase(i);

            item->playPickAnimation([player, item]()
                {
                    player->pickItem(item);
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
        { CCLOG("Key Pressed: %d", (int)key); // <--- 添加这里
            if (_gameBG && _gameBG->isGamePaused()) return;

            if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P || _gameMode == GameMode::FOG)
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
            if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P|| _gameMode == GameMode::FOG)
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

void GameScene::onExit()
{
    // 安全释放 FogManager
    if (_fogManager)
    {
        _fogManager->removeFromParent();
        _fogManager = nullptr;
    }

    // 清理 AIController
    delete _aiController;
    _aiController = nullptr;

    Scene::onExit();
}


void GameScene::onGameOver(Player* winner)
{
    //if (_gameOver) return; // 防止重复调用
    //_gameOver = true;

    // 停止所有更新
    this->unscheduleUpdate();

    //停止所有角色动作
    for (auto p : _players)
        if (p) p->stopAllActions();

    // 判断输赢
    bool isWin = false;
   
        // 如果赢家是 AI，那玩家就输了；如果赢家是人类，那就是赢了
        // 注意：你之前的 checkGameOver 逻辑里，AI 全灭传递的是 lastHuman (赢)，人类全灭传递 nullptr (输)
        // 所以只要 winner 不为空，且不是 AI，就是玩家赢
        if (winner != nullptr && !winner->isAI) isWin = true;
    

    CCLOG("Game Over. Win: %d", isWin);

    // ⭐ 创建并显示结算弹窗
    // 传入当前的模式和角色ID，让 Layer 记住它们
    auto layer = GameOverLayer::create(isWin, _gameMode, _player1CharacterId, _player2CharacterId);
    Vec2 camPos = this->getDefaultCamera()->getPosition();
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 计算屏幕左下角的坐标
    // (摄像机在中心，减去一半宽/高就是左下角)
    Vec2 screenOrigin = camPos - Vec2(visibleSize.width / 2, visibleSize.height / 2);

    // 设置 Layer 的位置，让它刚好覆盖当前屏幕
    layer->setPosition(screenOrigin);

    // 设置一个很高的层级，确保遮住地图和 UI
    this->addChild(layer, 9999);

    /*if (winner)
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
    ));*/
}



