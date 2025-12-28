#include "AIController.h"
#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "ItemManager.h"
#include "Bomb.h"

using namespace cocos2d;

// --- 生命周期 ---

AIController::AIController(GameScene* scene)
    : _scene(scene)
{
}

AIController::~AIController()
{
}

// --- 核心逻辑 (决策与更新) ---

void AIController::updateAI(float dt, Player* ai, AIState& state)
{
    if (!ai || ai->isDead) return;

    // 1. 思考冷却 (模拟反应延迟)
    state.thinkCooldown -= dt;
    if (state.thinkCooldown > 0)
    {
        state.stateTime += dt;
        return;
    }
    state.thinkCooldown = 0.25f;

    // 2. 优先级 I: 生存 (检测到危险立即打断当前行为)
    if (tryEscapeDanger(ai, state))
    {
        state.state = AIStateType::EscapeDanger;
        state.stateTime = 0;
    }
    else
    {
        // 3. 优先级 II: 状态切换 (基于性格与环境)
        // 当状态维持足够久，或处于空闲时，重新决策
        if (state.stateTime >= state.minStateDuration || state.state == AIStateType::Idle)
        {
            bool decisionMade = false;

            // 贪婪判定：捡道具
            if (!decisionMade && CCRANDOM_0_1() < state.curiosity && tryPickItem(ai, state))
            {
                state.state = AIStateType::PickItem;
                decisionMade = true;
            }
            // 激进判定：攻击玩家
            if (!decisionMade && CCRANDOM_0_1() < state.aggressiveness && tryAttackPlayer(ai, state))
            {
                state.state = AIStateType::ChasePlayer;
                decisionMade = true;
            }
            // 破坏判定：炸墙
            if (!decisionMade && tryDestroyWall(ai, state))
            {
                state.state = AIStateType::BreakWall;
                decisionMade = true;
            }

            // 兜底：随机游荡
            if (!decisionMade)
            {
                state.state = AIStateType::Idle;
                randomMove(ai, state);
            }

            state.minStateDuration = 1.0f + CCRANDOM_0_1(); // 随机持续 1~2秒
            state.stateTime = 0;
        }
    }

    // 4. 执行状态 (持续更新路径或动作)
    switch (state.state)
    {
    case AIStateType::EscapeDanger: tryEscapeDanger(ai, state); break;
    case AIStateType::PickItem:     tryPickItem(ai, state);     break;
    case AIStateType::ChasePlayer:  tryAttackPlayer(ai, state); break;
    case AIStateType::BreakWall:    tryDestroyWall(ai, state);  break;
    case AIStateType::Idle:         /* Idle下通常不需要逐帧更新路径 */ break;
    }

    state.stateTime += dt;
}

float AIController::getHeatValue(const Vec2& grid)
{
    auto map = _scene->getMapLayer();
    if (!map || !map->isWalkable(grid.x, grid.y)) return 999.0f; // 墙壁不可行

    float score = 0.0f;

    // 1. 火焰判定 (极度危险)
    if (map->getTile(grid.x, grid.y) == 300) score += 100.0f;

    // 2. 炸弹预警 (倒计时越短越危险)
    const auto& activeBombs = _scene->getActiveBombs();
    for (const auto& bomb : activeBombs)
    {
        if (bomb.willExplodeGrid(grid))
        {
            // 基础分70 + 紧迫分30
            float timeFactor = (2.0f - bomb.timeLeft) / 2.0f;
            score += (70.0f + (30.0f * timeFactor));
        }
    }

    // 3. 地形风险 (死胡同惩罚)
    int exits = 0;
    const Vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (const auto& d : dirs) {
        if (map->isWalkable(grid.x + d.x, grid.y + d.y)) exits++;
    }
    if (exits <= 1) score += 25.0f; // 避免钻入死角

    return score;
}

// --- 具体行为逻辑 ---

bool AIController::tryEscapeDanger(Player* ai, AIState& state)
{
    Vec2 currentGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    float currentHeat = getHeatValue(currentGrid);

    // 安全则无需操作
    if (currentHeat <= 0.1f) return false;

    // 寻找周围热值最低的格子
    Vec2 bestDir = Vec2::ZERO;
    float minHeat = currentHeat;

    const Vec2 dirs[4] = { Vec2(0,1), Vec2(0,-1), Vec2(-1,0), Vec2(1,0) };
    for (const auto& d : dirs)
    {
        float h = getHeatValue(currentGrid + d);
        if (h < minHeat)
        {
            minHeat = h;
            bestDir = d;
        }
    }

    if (bestDir != Vec2::ZERO)
    {
        state.nextDir = bestDir;
        return true;
    }
    return false; // 无路可逃
}

bool AIController::tryPickItem(Player* ai, AIState& state)
{
    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    auto path = _scene->findPathToItem(aiGrid);

    if (!path.empty())
    {
        state.nextDir = path[0];
        state.wantBomb = false; // 捡东西时不放炸弹
        return true;
    }
    return false;
}

bool AIController::tryAttackPlayer(Player* ai, AIState& state)
{
    Player* target = _scene->findNearestPlayer(ai);
    if (!target) return false;

    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    auto path = _scene->findPathToPlayer(aiGrid, target);

    if (!path.empty()) state.nextDir = path[0];

    // 攻击决策：距离近 + 有弹药 + 此时放弹后能跑掉
    float dist = aiGrid.distance(_scene->getMapLayer()->worldToGrid(target->getPosition()));
    if (ai->canPlaceBomb && dist <= 1.5f)
    {
        if (_scene->hasSafeEscape(aiGrid, ai)) {
            state.wantBomb = true;
        }
    }
    return true;
}

bool AIController::tryDestroyWall(Player* ai, AIState& state)
{
    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    auto path = _scene->findPathToSoftWall(aiGrid);

    if (!path.empty())
    {
        state.nextDir = path[0];

        // 如果紧邻软墙，尝试放炸弹
        Vec2 nextGrid = aiGrid + path[0];
        // 检查：有弹药 + 放弹后有路可退
        if (ai->canPlaceBomb && _scene->hasSafeEscape(nextGrid, ai))
        {
            state.wantBomb = true;
        }
        else
        {
            state.wantBomb = false;
        }
        return true;
    }
    return false;
}

void AIController::randomMove(Player* ai, AIState& state)
{
    std::vector<Vec2> dirs = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };
    Vec2 grid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    std::vector<Vec2> valid;

    for (auto d : dirs)
    {
        Vec2 next = grid + d;
        // 仅移动到可行走且无危险的区域
        if (_scene->getMapLayer()->isWalkable(next.x, next.y) && !_scene->isGridDangerPublic(next))
            valid.push_back(d);
    }

    if (!valid.empty())
    {
        int idx = rand() % valid.size();
        state.nextDir = valid[idx];
    }
}

// --- 辅助功能 ---

bool AIController::tryPlaceBombSafely(Player* ai)
{
    if (!ai->canPlaceBomb || ai->isDead || ai->isMoving) return false;

    Vec2 grid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    int range = ai->bombRange;
    std::vector<Vec2> safeDirs;

    // 预演：如果我现在放炸弹，有没有方向可以跑？
    for (auto d : { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) })
    {
        Vec2 next = grid;
        bool safe = true;

        // 检查该方向是否被未来的爆炸覆盖或受阻
        for (int i = 1; i <= range; ++i)
        {
            next += d;
            if (!_scene->getMapLayer()->isWalkable(next.x, next.y) || _scene->isGridDangerPublic(grid))
            {
                safe = false;
                break;
            }
        }
        if (safe) safeDirs.push_back(d);
    }

    if (!safeDirs.empty())
    {
        // 确认安全，执行放弹
        ai->placeBomb(_scene, _scene->getMapLayer());

        // 立即向安全方向移动
        int idx = rand() % safeDirs.size();
        ai->tryMoveTo(grid + safeDirs[idx], _scene->getMapLayer());
        return true;
    }

    return false;
}