#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Flame.h"
#include "ItemManager.h"
#include "GameBackground.h"
#include "AIController.h"
#include "FogManager.h"
#include "GameOverLayer.h"
#include "AudioEngine.h"

#include <algorithm>
#include <vector>
#include <queue>
#include <map>

USING_NS_CC;
using namespace cocos2d::experimental;

// --- 静态变量初始化 ---
bool GameScene::s_isAudioOn = true;
int GameScene::s_menuAudioID = AudioEngine::INVALID_AUDIO_ID;
int GameScene::s_gameAudioID = AudioEngine::INVALID_AUDIO_ID;

// --- 生命周期与工厂 ---

Scene* GameScene::createScene()
{
    return GameScene::create();
}

GameScene* GameScene::createWithMode(GameMode mode, int p1Face, int p2Face, GameDifficulty diff)
{
    auto scene = new (std::nothrow) GameScene();
    if (scene)
    {
        scene->_gameMode = mode;
        scene->_player1CharacterId = p1Face;
        scene->_player2CharacterId = p2Face;
        scene->_difficulty = diff;

        if (scene->init())
        {
            scene->autorelease();
            return scene;
        }
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

bool GameScene::init()
{
    if (!Scene::init()) return false;

    // 1. 音频管理 (切换背景音乐)
    if (s_menuAudioID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(s_menuAudioID);
        s_menuAudioID = AudioEngine::INVALID_AUDIO_ID;
    }
    if (s_isAudioOn && s_gameAudioID == AudioEngine::INVALID_AUDIO_ID) {
        s_gameAudioID = AudioEngine::play2d("Sound/GameBackgroundSound.mp3", true, 0.5f);
    }

    // 2. 场景组件初始化
    _gameBG = GameBackground::create();
    this->addChild(_gameBG, 0);

    _mapLayer = MapLayer::create();
    _mapLayer->setGameMode(_gameMode);
    this->addChild(_mapLayer, 1);

    if (_gameMode == GameMode::FOG) {
        _fogManager = FogManager::create();
        this->addChild(_fogManager, 100);
        _fogManager->initFog(Director::getInstance()->getVisibleSize());
    }

    _itemManager = ItemManager::create(_mapLayer);
    _itemManager->setName("ItemManager");
    this->addChild(_itemManager, 5);

    _aiController = new AIController(this);

    // 3. 实体与输入初始化
    initPlayers();
    initKeyboard();

    // 4. 启动主循环
    scheduleUpdate();

    // 延迟开启结束检测 (避免初始化时的误判)
    _gameOver = false;
    _canCheckGameOver = false;
    this->runAction(Sequence::create(
        DelayTime::create(0.2f),
        CallFunc::create([this]() { _canCheckGameOver = true; }),
        nullptr
    ));

    return true;
}

void GameScene::onExit()
{
    if (_fogManager) {
        _fogManager->removeFromParent();
        _fogManager = nullptr;
    }

    CC_SAFE_DELETE(_aiController);

    if (s_gameAudioID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(s_gameAudioID);
        s_gameAudioID = AudioEngine::INVALID_AUDIO_ID;
    }

    Scene::onExit();
}

// --- 初始化辅助 ---

void GameScene::initPlayers()
{
    _players.clear();
    std::vector<int> availableIds = { 1, 2, 3, 4 };
    auto removeId = [&](int id) {
        availableIds.erase(std::remove(availableIds.begin(), availableIds.end(), id), availableIds.end());
        };

    // 根据模式生成玩家和AI
    switch (_gameMode)
    {
    case GameMode::SINGLE:
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        removeId(_player1CharacterId);
        if (availableIds.size() >= 3) {
            createAIPlayer(Vec2(11, 1), availableIds[0], "AI_1");
            createAIPlayer(Vec2(1, 11), availableIds[1], "AI_2");
            createAIPlayer(Vec2(11, 11), availableIds[2], "AI_3");
        }
        _aiStates.resize(3);
        break;

    case GameMode::LOCAL_2P:
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        removeId(_player1CharacterId);
        createLocalPlayer(Vec2(11, 11), _player2CharacterId, "Player2");
        removeId(_player2CharacterId);
        if (availableIds.size() >= 2) {
            createAIPlayer(Vec2(1, 11), availableIds[0], "AI_2");
            createAIPlayer(Vec2(11, 1), availableIds[1], "AI_3");
        }
        _aiStates.resize(2);
        break;

    case GameMode::FOG:
        createLocalPlayer(Vec2(1, 1), _player1CharacterId, "Player1");
        removeId(_player1CharacterId);
        if (availableIds.size() >= 2) {
            createAIPlayer(Vec2(11, 1), availableIds[0], "AI_1");
            createAIPlayer(Vec2(1, 11), availableIds[1], "AI_2");
        }
        _aiStates.resize(2);
        break;

    case GameMode::ONLINE:
        break;
    }

    // 初始化 UI 面板
    for (int i = 0; i < _players.size(); ++i) {
        if (_players[i]) {
            _players[i]->setPlayerIndex(i);
            if (_gameBG) _gameBG->updatePlayerStat(i, 0, 0);
        }
    }
}

void GameScene::createLocalPlayer(const Vec2& gridPos, int characterId, const std::string& name)
{
    auto player = Player::createPlayer();
    if (!player) return;

    Vec2 worldPos = _mapLayer->gridToWorld(gridPos.x, gridPos.y);
    player->setPosition(worldPos);
    player->setCharacter(characterId);
    player->setName(name);
    player->_scene = this;

    this->addChild(player, 10);
    _players.push_back(player);

    CCLOG("%s created at (%.2f, %.2f)", name.c_str(), worldPos.x, worldPos.y);
}

void GameScene::createAIPlayer(const Vec2& gridPos, int characterId, const std::string& name)
{
    auto player = Player::createPlayer();
    if (!player) return;

    Vec2 worldPos = _mapLayer->gridToWorld(gridPos.x, gridPos.y);
    player->setPosition(worldPos);
    player->setCharacter(characterId);
    player->setName(name);
    player->isAI = true;
    player->_scene = this;

    // 难度配置
    if (_difficulty == GameDifficulty::EASY) {
        player->aiAggressive = 0.2f;
        player->aiCuriosity = 0.8f;
        player->aiCoward = 0.8f;
        player->defaultMoveSpeed *= 0.5f;
        player->moveSpeed = player->defaultMoveSpeed;
    }
    else if (_difficulty == GameDifficulty::HARD) {
        player->aiAggressive = 0.9f;
        player->aiCuriosity = 0.4f;
        player->aiCoward = 0.2f;
    }

    this->addChild(player, 10);
    _players.push_back(player);

    CCLOG("AI %s created (Diff:%d)", name.c_str(), (int)_difficulty);
}

void GameScene::initKeyboard()
{
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [&](EventKeyboard::KeyCode key, Event*) {
        if (_gameBG && _gameBG->isGamePaused()) return;
        // P1
        if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P || _gameMode == GameMode::FOG) {
            if (key == EventKeyboard::KeyCode::KEY_W) keyW = true;
            if (key == EventKeyboard::KeyCode::KEY_S) keyS = true;
            if (key == EventKeyboard::KeyCode::KEY_A) keyA = true;
            if (key == EventKeyboard::KeyCode::KEY_D) keyD = true;
            if (key == EventKeyboard::KeyCode::KEY_SPACE) keyBomb1 = true;
        }
        // P2
        if (_gameMode == GameMode::LOCAL_2P) {
            if (key == EventKeyboard::KeyCode::KEY_UP_ARROW) keyUp = true;
            if (key == EventKeyboard::KeyCode::KEY_DOWN_ARROW) keyDown = true;
            if (key == EventKeyboard::KeyCode::KEY_LEFT_ARROW) keyLeft = true;
            if (key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) keyRight = true;
            if (key == EventKeyboard::KeyCode::KEY_ENTER || key == EventKeyboard::KeyCode::KEY_KP_ENTER) keyBomb2 = true;
        }
        };

    listener->onKeyReleased = [&](EventKeyboard::KeyCode key, Event*) {
        // P1
        if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P || _gameMode == GameMode::FOG) {
            if (key == EventKeyboard::KeyCode::KEY_W) keyW = false;
            if (key == EventKeyboard::KeyCode::KEY_S) keyS = false;
            if (key == EventKeyboard::KeyCode::KEY_A) keyA = false;
            if (key == EventKeyboard::KeyCode::KEY_D) keyD = false;
            if (key == EventKeyboard::KeyCode::KEY_SPACE) keyBomb1 = false;
        }
        // P2
        if (_gameMode == GameMode::LOCAL_2P) {
            if (key == EventKeyboard::KeyCode::KEY_UP_ARROW) keyUp = false;
            if (key == EventKeyboard::KeyCode::KEY_DOWN_ARROW) keyDown = false;
            if (key == EventKeyboard::KeyCode::KEY_LEFT_ARROW) keyLeft = false;
            if (key == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) keyRight = false;
            if (key == EventKeyboard::KeyCode::KEY_ENTER || key == EventKeyboard::KeyCode::KEY_KP_ENTER) keyBomb2 = false;
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// --- 核心帧循环 (Update) ---

void GameScene::update(float dt)
{
    if (_gameBG && _gameBG->isGamePaused()) return;

    // 1. 输入处理
    handleInput(dt);

    // 2. AI 思考
    if ((_gameMode != GameMode::ONLINE) && _aiController) {
        updateAI(dt);
    }

    // 3. 视野与迷雾
    for (auto p : _players) {
        if (p && !p->isDead) p->updateVision(dt);
    }
    if (_gameMode == GameMode::FOG && _fogManager && !_players.empty() && _players[0] && !_players[0]->isDead) {
        _fogManager->updateFog(_players[0]);
    }

    // 4. 碰撞与交互
    for (auto player : _players) {
        if (!player || player->isDead) continue;
        checkFlameHit(player);
        checkItemPickup(player);
    }

    // 5. 炸弹预警计时
    updateBombDangers(dt);

    // 6. 胜负检测
    if (_canCheckGameOver) checkGameOver();
}

void GameScene::updateBombDangers(float dt)
{
    for (auto it = _bombDangers.begin(); it != _bombDangers.end(); ) {
        it->timeLeft -= dt;
        if (it->timeLeft <= 0)
            it = _bombDangers.erase(it);
        else
            ++it;
    }
}

void GameScene::updateUIForPlayer(Player* p)
{
    if (!p || !_gameBG) return;
    _gameBG->updatePlayerStat(p->getPlayerIndex(), p->getScore(), p->getItemCount());
}

// --- 输入与移动处理 ---

void GameScene::handleInput(float dt)
{
    if (_players.size() > 0)
        handlePlayerMove(_players[0], keyW, keyS, keyA, keyD, keyBomb1, dt);

    if (_gameMode == GameMode::LOCAL_2P && _players.size() > 1)
        handlePlayerMove(_players[1], keyUp, keyDown, keyLeft, keyRight, keyBomb2, dt);
}

void GameScene::handlePlayerMove(Player* player, bool up, bool down, bool left, bool right, bool& bombKey, float dt)
{
    if (!player || player->isDead) return;

    if (!player->isMoving) {
        Vec2 dir = Vec2::ZERO;
        if (up)    dir = Vec2(0, 1);
        else if (down)  dir = Vec2(0, -1);
        else if (left)  dir = Vec2(-1, 0);
        else if (right) dir = Vec2(1, 0);

        if (dir != Vec2::ZERO) {
            Vec2 grid = _mapLayer->worldToGrid(player->getPosition());
            player->tryMoveTo(grid + dir, _mapLayer);
        }
    }

    if (bombKey) {
        player->placeBomb(this, _mapLayer);
        bombKey = false;
    }
}

// --- 碰撞与交互检测 ---

void GameScene::checkFlameHit(Player* player)
{
    Vec2 pGrid = _mapLayer->worldToGrid(player->getPosition());

    for (auto node : _mapLayer->getChildren()) {
        auto flame = dynamic_cast<Flame*>(node);
        if (flame && flame->gridPos.equals(pGrid)) {
            if (!player->invincible && !player->isDead) {
                player->takeDamage();
                // 击杀判定
                if (player->isDead) {
                    Player* killer = flame->getOwner();
                    if (killer && killer != player) killer->addScore(500);
                    else if (killer == player) killer->addScore(-500);
                }
            }
            break;
        }
    }
}

void GameScene::checkItemPickup(Player* player)
{
    if (!player || !_itemManager) return;

    auto& items = _itemManager->getItems();
    for (int i = items.size() - 1; i >= 0; --i) {
        Item* item = items.at(i);
        if (!item) continue;

        // 获取世界坐标包围盒
        Rect playerRect = player->getBoundingBox();
        playerRect.origin = player->getParent()->convertToWorldSpace(playerRect.origin);
        Rect itemRect = item->getBoundingBox();
        itemRect.origin = item->getParent()->convertToWorldSpace(itemRect.origin);

        if (playerRect.intersectsRect(itemRect)) {
            CCLOG("Item picked up! Type=%d", static_cast<int>(item->getType()));
            items.erase(i);
            item->playPickAnimation([player, item]() {
                player->pickItem(item);
                item->removeFromParent();
                });
            break;
        }
    }
}

// --- 游戏规则与逻辑判定 ---

void GameScene::registerBomb(const Vec2& grid, int range)
{
    for (const auto& b : _bombDangers) {
        if (b.bombGrid == grid) return;
    }
    _bombDangers.push_back({ grid, range, 2.1f });
    CCLOG("Bomb registered at Grid(%.0f, %.0f)", grid.x, grid.y);
}

bool GameScene::isGridDanger(const Vec2& grid)
{
    // 物理火焰
    if (_mapLayer->getTile(grid.x, grid.y) == 300) return true;
    // 炸弹预警
    for (const auto& b : _bombDangers) {
        if (b.willExplodeGrid(grid)) return true;
    }
    return false;
}

bool GameScene::hasSafeEscape(const Vec2& grid, Player* ai)
{
    for (auto d : { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) }) {
        Vec2 next = grid + d;
        if (_mapLayer->isWalkable(next.x, next.y) && !isGridDanger(next))
            return true;
    }
    return false;
}

bool GameScene::isPlayerCornered(Player* player)
{
    Vec2 grid = _mapLayer->worldToGrid(player->getPosition());
    int blocked = 0;
    for (auto d : { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) }) {
        Vec2 next = grid + d;
        if (!_mapLayer->isWalkable(next.x, next.y) || isGridDanger(next))
            blocked++;
    }
    return blocked >= 2;
}

bool GameScene::willBombTrapPlayer(const Vec2& bombGrid, Player* target, int bombRange)
{
    Vec2 tg = _mapLayer->worldToGrid(target->getPosition());
    // 简单判定：同行列且在范围内
    if (bombGrid.x == tg.x && abs(bombGrid.y - tg.y) <= bombRange) return true;
    if (bombGrid.y == tg.y && abs(bombGrid.x - tg.x) <= bombRange) return true;
    return false;
}

Player* GameScene::findNearestPlayer(Player* self)
{
    Player* nearest = nullptr;
    float minDist = FLT_MAX;
    Vec2 selfGrid = _mapLayer->worldToGrid(self->getPosition());

    for (auto p : _players) {
        if (!p || p == self || p->isDead) continue;
        if (self->isAI && p->isAI) continue; // AI不互殴

        Vec2 pg = _mapLayer->worldToGrid(p->getPosition());
        float d = selfGrid.distance(pg);

        if (d < minDist) {
            minDist = d;
            nearest = p;
        }
    }
    return nearest;
}

// --- AI 系统与寻路算法 ---

void GameScene::updateAI(float dt)
{
    int aiIndex = 0;
    for (size_t i = 0; i < _players.size(); ++i) {
        auto p = _players[i];
        if (!p || p->isDead || !p->isAI) continue;
        if (aiIndex >= (int)_aiStates.size()) break;

        // 决策
        _aiController->updateAI(dt, p, _aiStates[aiIndex]);
        auto& s = _aiStates[aiIndex];

        // 执行移动
        if (!p->isMoving && s.nextDir != Vec2::ZERO) {
            Vec2 grid = _mapLayer->worldToGrid(p->getPosition());
            p->tryMoveTo(grid + s.nextDir, _mapLayer);
        }
        // 执行放雷
        if (s.wantBomb) {
            p->placeBomb(this, _mapLayer);
            s.wantBomb = false;
        }
        aiIndex++;
    }
}

// A* 核心寻路算法
std::vector<Vec2> GameScene::findSmartPath(const Vec2& start, const Vec2& target, bool avoidDanger)
{
    struct AStarNode {
        Vec2 pos;
        float gScore;
        float fScore;
        std::vector<Vec2> path;
        bool operator>(const AStarNode& other) const { return fScore > other.fScore; }
    };

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openList;
    auto cmp = [](const Vec2& a, const Vec2& b) { return a.x == b.x ? a.y < b.y : a.x < b.x; };
    std::map<Vec2, float, decltype(cmp)> gScores(cmp);

    float startH = start.distance(target);
    openList.push({ start, 0.0f, startH, {} });
    gScores[start] = 0.0f;

    const Vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    while (!openList.empty()) {
        AStarNode current = openList.top();
        openList.pop();

        if (current.pos == target) return current.path;
        if (current.gScore > gScores[current.pos]) continue;

        for (const auto& d : dirs) {
            Vec2 next = current.pos + d;
            if (!_mapLayer || !_mapLayer->isWalkable(next.x, next.y)) continue;

            float moveCost = 1.0f;
            if (avoidDanger && _aiController) {
                float heat = _aiController->getHeatValue(next);
                if (heat > 90.0f) continue; // 极度危险
                moveCost += heat * 0.5f;
            }

            float tentativeG = current.gScore + moveCost;
            if (gScores.find(next) == gScores.end() || tentativeG < gScores[next]) {
                gScores[next] = tentativeG;
                float h = next.distance(target);
                std::vector<Vec2> nextPath = current.path;
                nextPath.push_back(d);
                openList.push({ next, tentativeG, tentativeG + h, nextPath });
            }
        }
    }
    return {};
}

// 寻路封装：找玩家
std::vector<Vec2> GameScene::findPathToPlayer(const Vec2& start, Player* target)
{
    if (!target) return {};
    Vec2 targetGrid = _mapLayer->worldToGrid(target->getPosition());
    return findSmartPath(start, targetGrid, true);
}

// 寻路封装：找道具
std::vector<Vec2> GameScene::findPathToItem(const Vec2& start)
{
    Vec2 bestItemGrid = Vec2(-1, -1);
    float minDist = FLT_MAX;

    for (auto item : _itemManager->getItems()) {
        Vec2 ig = _mapLayer->worldToGrid(item->getPosition());
        float d = start.distance(ig);
        if (d < minDist) {
            minDist = d;
            bestItemGrid = ig;
        }
    }
    if (bestItemGrid != Vec2(-1, -1)) {
        return findSmartPath(start, bestItemGrid, true);
    }
    return {};
}

// 寻路封装：BFS找安全点
std::vector<Vec2> GameScene::findSafePathBFS(const Vec2& start)
{
    std::queue<Vec2> q;
    q.push(start);
    std::set<std::pair<int, int>> visited;
    visited.insert({ (int)start.x, (int)start.y });

    Vec2 safeTarget = start;
    bool found = false;

    while (!q.empty()) {
        Vec2 curr = q.front(); q.pop();
        if (_aiController->getHeatValue(curr) < 0.1f) {
            safeTarget = curr;
            found = true;
            break;
        }
        for (auto d : { Vec2(1,0),Vec2(-1,0),Vec2(0,1),Vec2(0,-1) }) {
            Vec2 n = curr + d;
            if (_mapLayer->isWalkable(n.x, n.y) && visited.find({ (int)n.x, (int)n.y }) == visited.end()) {
                visited.insert({ (int)n.x, (int)n.y });
                q.push(n);
            }
        }
        if (visited.size() > 50) break;
    }
    if (found) return findSmartPath(start, safeTarget, true);
    return {};
}

// 寻路封装：找软墙
std::vector<Vec2> GameScene::findPathToSoftWall(const Vec2& start)
{
    std::queue<Vec2> q;
    q.push(start);
    auto cmp = [](const Vec2& a, const Vec2& b) { return a.x == b.x ? a.y < b.y : a.x < b.x; };
    std::map<Vec2, bool, decltype(cmp)> visited(cmp);
    visited[start] = true;

    Vec2 targetGrid = Vec2(-1, -1);
    int searchCount = 0;

    while (!q.empty() && searchCount < 300) {
        Vec2 curr = q.front(); q.pop();
        searchCount++;

        bool nearRealSoftWall = false;
        const Vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

        // 检查四周是否有墙
        for (const auto& d : dirs) {
            Vec2 neighbor = curr + d;
            if (_mapLayer->getTile(neighbor.x, neighbor.y) == MapLayer::TILE_SOFT_WALL) {
                nearRealSoftWall = true;
                break;
            }
        }

        if (nearRealSoftWall) {
            targetGrid = curr;
            break;
        }

        // 扩散
        for (const auto& d : dirs) {
            Vec2 next = curr + d;
            if (_mapLayer->isWalkable(next.x, next.y) && visited.find(next) == visited.end()) {
                visited[next] = true;
                q.push(next);
            }
        }
    }

    if (targetGrid != Vec2(-1, -1)) {
        return findSmartPath(start, targetGrid, true);
    }
    return {};
}

// --- 游戏流程 (结束判定) ---

void GameScene::checkGameOver()
{
    if (!_canCheckGameOver || _gameOver) return;

    int aliveHumanCount = 0;
    int aliveAICount = 0;
    Player* lastHuman = nullptr;

    for (auto p : _players) {
        if (!p || p->isDead) continue;
        if (p->isAI) aliveAICount++;
        else {
            aliveHumanCount++;
            lastHuman = p;
        }
    }

    if (aliveHumanCount == 0) {
        _gameOver = true;
        onGameOver(nullptr); // 失败
    }
    else if (aliveAICount == 0) {
        _gameOver = true;
        onGameOver(lastHuman); // 胜利
    }
}

void GameScene::onGameOver(Player* winner)
{
    this->unscheduleUpdate();
    for (auto p : _players) if (p) p->stopAllActions();

    bool isWin = (winner != nullptr && !winner->isAI);
    CCLOG("Game Over. Win: %d", isWin);

    int score = 0;
    if (!_players.empty() && _players[0]) {
        score = _players[0]->getScore();
    }

    // 显示结算弹窗
    auto layer = GameOverLayer::create(isWin, _gameMode, _player1CharacterId, _player2CharacterId, score);

    // 居中适配
    Vec2 camPos = this->getDefaultCamera()->getPosition();
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 screenOrigin = camPos - Vec2(visibleSize.width / 2, visibleSize.height / 2);

    layer->setPosition(screenOrigin);
    this->addChild(layer, 9999);
}