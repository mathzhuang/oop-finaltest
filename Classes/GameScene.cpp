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
#include "AudioEngine.h"

#include <algorithm> 
#include <vector>
#include <queue>
#include <map>

USING_NS_CC;
using namespace cocos2d::experimental;

// --- 初始化静态变量 ---
bool GameScene::s_isAudioOn = true;
int GameScene::s_menuAudioID = AudioEngine::INVALID_AUDIO_ID;
int GameScene::s_gameAudioID = AudioEngine::INVALID_AUDIO_ID;

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
GameScene* GameScene::createWithMode(GameMode mode, int p1Face, int p2Face, GameDifficulty diff)
{
    auto scene = new GameScene();
    scene->_gameMode = mode;
    scene->_player1CharacterId = p1Face;
    scene->_player2CharacterId = p2Face;
    scene->_difficulty = diff; // ⭐ 保存难度

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

    // --- 音频处理 ---
    // 1. 停止 StartSound (如果在播放)
    if (s_menuAudioID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(s_menuAudioID);
        s_menuAudioID = AudioEngine::INVALID_AUDIO_ID;
    }

    // 2. 播放游戏背景音乐 (GameBackgroundSound)
    // 只有当全局音效开启时才播放
    if (s_isAudioOn) {
        // 播放 GameBackgroundSound.mp3，循环=true，并保存 ID 到 s_gameAudioID
        if (s_gameAudioID == AudioEngine::INVALID_AUDIO_ID) {
            s_gameAudioID = AudioEngine::play2d("Sound/GameBackgroundSound.mp3", true, 0.5f);
        }
    }

    // 1. 背景
    _gameBG = GameBackground::create();
    this->addChild(_gameBG, 0);

    // 2. 地图
    _mapLayer = MapLayer::create();
    // ✅ 关键：把 GameScene 的模式同步给 MapLayer
    _mapLayer->setGameMode(_gameMode);
    this->addChild(_mapLayer, 1);

    // 2.5 迷雾（仅 FOG 模式）
    // 创建 FogManager
    if (_gameMode == GameMode::FOG)
    {
        _fogManager = FogManager::create();
        this->addChild(_fogManager, 100); // 确保在最上层
        _fogManager->initFog(Director::getInstance()->getVisibleSize());

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

    // 统一分配 Index
    for (int i = 0; i < _players.size(); ++i) {
        if (_players[i]) {
            _players[i]->setPlayerIndex(i);
            // 初始化 UI 为 0
            if (_gameBG) _gameBG->updatePlayerStat(i, 0, 0);
        }
    }
}

// 实现 updateUIForPlayer
void GameScene::updateUIForPlayer(Player* p)
{
    if (!p || !_gameBG) return;
    _gameBG->updatePlayerStat(p->getPlayerIndex(), p->getScore(), p->getItemCount());
}

//  修改 handlePlayerMove 中的 placeBomb 调用
// 其实 Player::placeBomb 内部创建 Bomb，我们需要在那里设置 setOwner

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
    player->_scene = this;
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
    if ((_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P || _gameMode == GameMode::FOG) && _aiController)
        updateAI(dt);

    // --------------------------------------------------------
    // 【新添加】：更新所有玩家的视野倒计时逻辑
    // --------------------------------------------------------
    for (auto p : _players)
    {
        if (p && !p->isDead) {
            p->updateVision(dt); // 这里会处理 10秒倒计时并修改 _visionRadius
        }
    }

    // Fog 模式安全更新
    if (_gameMode == GameMode::FOG && _fogManager)
    {
        if (_fogManager->getParent() && !_players.empty() && _players[0] && !_players[0]->isDead)
        {
            // 这里虽然还是传入 _players[0]，但因为 FogManager 内部
            // 我们已经改成了使用 player->getVisionRadius()，所以它会自动变大
            _fogManager->updateFog(_players[0]);
        }
    }

    for (auto player : _players)
    {
        if (!player || player->isDead) continue;
        checkFlameHit(player);
        checkItemPickup(player);
    }

    updateBombDangers(dt);

    if (_canCheckGameOver) checkGameOver();
}

void GameScene::updateBombDangers(float dt)
{
    for (auto it = _bombDangers.begin(); it != _bombDangers.end(); )
    {
        it->timeLeft -= dt;
        if (it->timeLeft <= 0)
            it = _bombDangers.erase(it);
        else
            ++it;
    }
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

        // --- 核心修正：如果自己是 AI，且目标也是 AI，则跳过（不互相攻击） ---
        if (self->isAI && p->isAI) continue;

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

// -----------------------------
// A* 智能寻路：结合距离与热力值 (危险度)
// -----------------------------
std::vector<Vec2> GameScene::findSmartPath(const Vec2& start, const Vec2& target, bool avoidDanger)
{
    struct AStarNode {
        Vec2 pos;
        float gScore; // 实际代价
        float fScore; // 预估总代价
        std::vector<Vec2> path;
        // 优先队列比较
        bool operator>(const AStarNode& other) const { return fScore > other.fScore; }
    };

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openList;

    auto cmp = [](const Vec2& a, const Vec2& b) {
        return a.x == b.x ? a.y < b.y : a.x < b.x;
        };
    std::map<Vec2, float, decltype(cmp)> gScores(cmp);

    // 初始节点
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

            // --- 核心：计算移动代价 ---
            float moveCost = 1.0f;
            if (avoidDanger && _aiController) {
                float heat = _aiController->getHeatValue(next);
                if (heat > 90.0f) continue; // 必死无疑的路，直接剪枝
                moveCost += heat * 0.5f;    // 危险路段加权，让 AI 宁愿绕远也不踩火
            }

            float tentativeG = current.gScore + moveCost;

            if (gScores.find(next) == gScores.end() || tentativeG < gScores[next]) {
                gScores[next] = tentativeG;
                float h = next.distance(target); // 启发式：曼哈顿距离

                std::vector<Vec2> nextPath = current.path;
                nextPath.push_back(d);

                openList.push({ next, tentativeG, tentativeG + h, nextPath });
            }
        }
    }
    return {};
}
std::vector<Vec2> GameScene::findPathToPlayer(const Vec2& start, Player* target)
{
    if (!target) return {};
    Vec2 targetGrid = _mapLayer->worldToGrid(target->getPosition());
    // 追踪玩家时，开启避险模式
    return findSmartPath(start, targetGrid, true);
}

std::vector<Vec2> GameScene::findPathToItem(const Vec2& start)
{
    // 这里如果想极致性能，可以先用 BFS 找最近的 Item 位置，再用 A* 过去
    // 为了简单，我们直接找最近的 Item 格子
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

// 安全逃生：寻找最近的热力值为 0 的格子
std::vector<Vec2> GameScene::findSafePathBFS(const Vec2& start)
{
    // 先通过简单的 BFS 泛洪找周围最近的安全点 (Heat == 0)
    // 然后用 A* 走过去
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
        if (visited.size() > 50) break; // 搜索范围限制
    }

    if (found) return findSmartPath(start, safeTarget, true);
    return {};
}
std::vector<Vec2> GameScene::findPathToSoftWall(const Vec2& start)
{
    // --- 1. 使用简易 BFS 扫描寻找最近的“有效软墙格” ---
    std::queue<Vec2> q;
    q.push(start);

    auto cmp = [](const Vec2& a, const Vec2& b) {
        return a.x == b.x ? a.y < b.y : a.x < b.x;
        };
    std::map<Vec2, bool, decltype(cmp)> visited(cmp);
    visited[start] = true;

    Vec2 targetGrid = Vec2(-1, -1);
    int searchCount = 0;

    while (!q.empty() && searchCount < 300) { // 稍微扩大搜索范围
        Vec2 curr = q.front(); q.pop();
        searchCount++;

        // --- 核心修正：手动检查四周，确保真的挨着软墙 ---
        bool nearRealSoftWall = false;
        const Vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

        for (const auto& d : dirs) {
            Vec2 neighbor = curr + d;
            // 必须明确指定判断软墙的 ID (假设 MapLayer::TILE_SOFT_WALL 是软墙)
            // 请根据你 MapLayer 里的定义修改这个 TILE_SOFT_WALL
            if (_mapLayer->getTile(neighbor.x, neighbor.y) == MapLayer::TILE_SOFT_WALL) {
                nearRealSoftWall = true;
                break;
            }
        }

        if (nearRealSoftWall) {
            targetGrid = curr;
            break;
        }

        // 继续扩散搜索
        for (const auto& d : dirs) {
            Vec2 next = curr + d;
            // A* 寻路会处理避险，但 BFS 找点阶段也要排除掉“死火”上的点
            if (_mapLayer->isWalkable(next.x, next.y) && visited.find(next) == visited.end()) {
                visited[next] = true;
                q.push(next);
            }
        }
    }

    // --- 2. 找到目标后，调用 A* 算法计算路径 ---
    if (targetGrid != Vec2(-1, -1)) {
        // 使用 A* 绕过炸弹前往该点
        return findSmartPath(start, targetGrid, true);
    }

    return {};
}
// GameScene::registerBomb
void GameScene::registerBomb(const Vec2& grid, int range)
{
    // 检查是否已经存在该位置的预警，避免重复冗余
    for (const auto& b : _bombDangers) {
        if (b.bombGrid == grid) return;
    }

    _bombDangers.push_back({
        grid,
        range,
        2.1f   // 略微多出 0.1s，确保火焰生成前预警不消失
        });

    CCLOG("Bomb registered at Grid(%.0f, %.0f) with range %d", grid.x, grid.y, range);
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
    // 初始化 AI 性格参数
    // -------------------
    //player->aiAggressive = 0.3f + CCRANDOM_0_1() * 0.6f;  // 0.3~0.9
    //player->aiCoward = 0.1f + CCRANDOM_0_1() * 0.6f;      // 0.1~0.7
    //player->aiCuriosity = 0.3f + CCRANDOM_0_1() * 0.6f;   // 0.3~0.9

    //难度扩展接口
    if (_difficulty == GameDifficulty::EASY)
    {
        // 简单模式：AI 比较笨，攻击性低，反应慢
        player->aiAggressive = 0.2f; // 很少主动攻击
        player->aiCuriosity = 0.8f;  // 喜欢乱跑捡道具
        player->aiCoward = 0.8f;     // 很怕死

        // 如果你的 AIController 有思考间隔，也可以在这里设置
        // player->thinkInterval = 1.0f; // 思考慢
    }
    else if (_difficulty == GameDifficulty::HARD)
    {
        // 困难模式：AI 疯狗模式
        player->aiAggressive = 0.9f; // 疯狂放炸弹
        player->aiCuriosity = 0.4f;
        player->aiCoward = 0.2f;     // 激进

        // player->thinkInterval = 0.2f; // 反应极快
    }


    // -------------------
    // 添加到场景与玩家列表
    // -------------------
    player->_scene = this;
    this->addChild(player, 10);
    _players.push_back(player);

    CCLOG("AI Player '%s' created at (%f,%f) Agg=%.2f Coward=%.2f Curious=%.2f",
        name.c_str(), worldPos.x, worldPos.y,
        player->aiAggressive, player->aiCoward, player->aiCuriosity);
}

void GameScene::updateAI(float dt)
{
    int aiIndex = 0;
    for (size_t i = 0; i < _players.size(); ++i)
    {
        auto p = _players[i];
        if (!p || p->isDead || !p->isAI) continue;

        // 确保 aiIndex 不会超过 _aiStates 的大小
        if (aiIndex >= (int)_aiStates.size()) break;

        _aiController->updateAI(dt, p, _aiStates[aiIndex]);

        auto& s = _aiStates[aiIndex];

        // 移动执行
        if (!p->isMoving && s.nextDir != Vec2::ZERO)
        {
            Vec2 grid = _mapLayer->worldToGrid(p->getPosition());
            p->tryMoveTo(grid + s.nextDir, _mapLayer);
        }

        // 放弹执行
        if (s.wantBomb)
        {
            // 在放弹的同时，由 Player 内部或此处调用 registerBomb
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
// 1. 统一的危险判定：不再写复杂的循环，直接调用结构体方法
bool GameScene::isGridDanger(const Vec2& grid)
{
    // A. 检查物理火焰 (O(1) 判定，基于我们之前的 mapData 优化)
    if (_mapLayer->getTile(grid.x, grid.y) == 300) {
        return true;
    }

    // B. 检查炸弹预测危险
    for (const auto& b : _bombDangers)
    {

        if (b.willExplodeGrid(grid)) return true;
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
// GameScene.cpp
//void GameScene::checkFlameHit(Player* player) {
//    if (!player || player->isDead) return;
//
//    // 将玩家世界坐标转换为格子坐标
//    Vec2 pGrid = _mapLayer->worldToGrid(player->getPosition());
//
//    // 直接从地图数据读取，不需要 dynamic_cast 遍历
//    if (_mapLayer->getTile(pGrid.x, pGrid.y) == MapLayer::TILE_FLAME) {
//        player->takeDamage(); // 触发伤害
//    }
//}
void GameScene::checkFlameHit(Player* player)
{
    Vec2 pGrid = _mapLayer->worldToGrid(player->getPosition());

    for (auto node : _mapLayer->getChildren())
    {
        auto flame = dynamic_cast<Flame*>(node);
        // 必须加上 flame->gridPos.equals(pGrid)，否则所有火焰都算
        if (flame && flame->gridPos.equals(pGrid))
        {
            // 玩家受伤
            if (!player->invincible && !player->isDead)
            {
                player->takeDamage(); // 内部会扣血

                // 如果玩家死了，处理得分
                if (player->isDead)
                {
                    Player* killer = flame->getOwner();
                    if (killer && killer != player)
                    {
                        // ⭐ 规则：对手死亡 +500
                        // 这里简化为：只要不是自杀，就是杀敌
                        killer->addScore(500);
                    }
                    else if (killer == player)
                    {
                        // ⭐ 规则：队友(自己)死亡 -500
                        killer->addScore(-500);
                    }
                    // 如果有队友系统 (TeamID)，在这里进一步判断
                }
            }
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
            if (_gameMode == GameMode::SINGLE || _gameMode == GameMode::LOCAL_2P || _gameMode == GameMode::FOG)
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

    // 离开游戏场景时，停止游戏背景音乐和所有音效
    //AudioEngine::stopAll();
    // 停止游戏背景音乐
    if (s_gameAudioID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(s_gameAudioID);
        s_gameAudioID = AudioEngine::INVALID_AUDIO_ID;
    }
    s_gameAudioID = AudioEngine::INVALID_AUDIO_ID;

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
