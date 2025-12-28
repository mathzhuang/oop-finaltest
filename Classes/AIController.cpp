// AIController.cpp
#include "AIController.h"
#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "ItemManager.h"
#include "Bomb.h"
#include "Flame.h"

using namespace cocos2d;

AIController::AIController(GameScene* scene)
    : _scene(scene)
{
}


// -----------------------------
// 热力图分值评估：分值越高越危险
// -----------------------------

float AIController::getHeatValue(const Vec2& grid)
{
    auto map = _scene->getMapLayer();

    // 1. 基础检查：如果是墙壁或越界，视为极度危险/不可通行
    if (!map || !map->isWalkable(grid.x, grid.y)) {
        return 999.0f;
    }

    float score = 0.0f;

    // 2. 物理火焰判定 (最高威胁)
    // 利用之前优化的 getTile 接口，如果是 TILE_FLAME (300) 则分值最大
    if (map->getTile(grid.x, grid.y) == 300) {
        score += 100.0f;
    }

    // 3. 动态炸弹预警评分
    // 遍历 GameScene 中的预警列表
    const auto& activeBombs = _scene->getActiveBombs();
    for (const auto& bomb : activeBombs)
    {
        if (bomb.willExplodeGrid(grid))
        {
            // 倒计时越短，分数越高。基础分 70，随时间线性增加到 100
            // 假设炸弹总时长为 2.0s
            float timeFactor = (2.0f - bomb.timeLeft) / 2.0f;
            score += (70.0f + (30.0f * timeFactor));
        }
    }

    // 4. 地形风险：死胡同惩罚
    // 如果一个格子周围只有一个出口，它在被追逐或有炸弹时极度危险
    int exits = 0;
    const Vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (const auto& d : dirs) {
        if (map->isWalkable(grid.x + d.x, grid.y + d.y)) {
            exits++;
        }
    }

    // 如果只有 1 个出口（死胡同），增加额外分值，防止 AI 钻进死角
    if (exits <= 1) {
        score += 25.0f;
    }

    return score;
}
// -----------------------------
// 主 AI 更新函数
// -----------------------------
void AIController::updateAI(float dt, Player* ai, AIState& state)
{
    if (!ai || ai->isDead) return;

    state.thinkCooldown -= dt;
    if (state.thinkCooldown > 0)
    {
        state.stateTime += dt;
        return;
    }
    state.thinkCooldown = 0.25f; // 模拟反应时间

    // 1. 优先处理生存：避灾逻辑
    if (tryEscapeDanger(ai, state))
    {
        state.state = AIStateType::EscapeDanger;
        state.stateTime = 0;
    }
    else
    {
        // 2. 状态切换逻辑
        if (state.stateTime >= state.minStateDuration || state.state == AIStateType::Idle)
        {
            bool chose = false;

            // 根据性格参数决策
            if (!chose && CCRANDOM_0_1() < state.curiosity && tryPickItem(ai, state))
            {
                state.state = AIStateType::PickItem;
                chose = true;
            }
            if (!chose && CCRANDOM_0_1() < state.aggressiveness && tryAttackPlayer(ai, state))
            {
                state.state = AIStateType::ChasePlayer;
                chose = true;
            }
            if (!chose && tryDestroyWall(ai, state))
            {
                state.state = AIStateType::BreakWall;
                chose = true;
            }

            if (!chose) state.state = AIStateType::Idle;

            state.minStateDuration = 1.0f + CCRANDOM_0_1();
            state.stateTime = 0;
        }
    }

    // 3. 执行具体动作
    switch (state.state)
    {
    case AIStateType::EscapeDanger: tryEscapeDanger(ai, state); break;
    case AIStateType::PickItem:     tryPickItem(ai, state);     break;
    case AIStateType::ChasePlayer:  tryAttackPlayer(ai, state);  break;
    case AIStateType::BreakWall:    tryDestroyWall(ai, state);  break;
    default:                        randomMove(ai, state);      break;
    }

    state.stateTime += dt;
}
// -----------------------------
// 逃离危险 (基于热力图)
// -----------------------------
bool AIController::tryEscapeDanger(Player* ai, AIState& state)
{
    Vec2 currentGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    float currentHeat = getHeatValue(currentGrid);

    // 如果当前格子完全安全，则无需逃跑
    if (currentHeat <= 0.1f) return false;

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
    return false;
}

// -----------------------------
// 攻击玩家 (包含模拟预判)
// -----------------------------
bool AIController::tryAttackPlayer(Player* ai, AIState& state)
{
    Player* target = _scene->findNearestPlayer(ai);
    if (!target) return false;

    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    auto path = _scene->findPathToPlayer(aiGrid, target);

    if (!path.empty()) state.nextDir = path[0];

    // 决定是否放炸弹：在目标附近且“放弹后能跑掉”
    if (ai->canPlaceBomb && aiGrid.distance(_scene->getMapLayer()->worldToGrid(target->getPosition())) <= 1.5f)
    {
        // 模拟放弹后的安全性：如果周围有低热值格子才放弹
        if (_scene->hasSafeEscape(aiGrid, ai)) {
            state.wantBomb = true;
        }
    }
    return true;
}
// -----------------------------
// 捡道具
// -----------------------------
bool AIController::tryPickItem(Player* ai, AIState& state)
{
    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    auto path = _scene->findPathToItem(aiGrid);

    if (!path.empty())
    {
        state.nextDir = path[0];

        // 仅在安全情况下考虑放炸弹（例如有障碍炸墙或攻击玩家时才放）
        state.wantBomb = false;
        return true;
    }

    return false;
}

// -----------------------------
// 炸软墙（安全放炸弹）
// -----------------------------
bool AIController::tryDestroyWall(Player* ai, AIState& state)
{
    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    auto path = _scene->findPathToSoftWall(aiGrid);

    if (!path.empty())
    {
        state.nextDir = path[0];

        // 如果靠近软墙且可放炸弹，并且有逃生路径
        Vec2 nextGrid = aiGrid + path[0];
        if (ai->canPlaceBomb &&
            _scene->hasSafeEscape(nextGrid, ai))
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

// -----------------------------
// 安全放炸弹函数（结合你的接口）
// -----------------------------
bool AIController::tryPlaceBombSafely(Player* ai)
{
    if (!ai->canPlaceBomb || ai->isDead || ai->isMoving)
        return false;

    Vec2 grid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    int range = ai->bombRange; // 火焰范围
    std::vector<Vec2> safeDirs;

    // 四个方向尝试寻找安全格
    for (auto d : { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) })
    {
        Vec2 next = grid;
        bool safe = true;

        for (int i = 1; i <= range; ++i)
        {
            next += d;
            if (!_scene->getMapLayer()->isWalkable(next.x, next.y) ||
                _scene->isGridDangerPublic(grid))
            {
                safe = false;
                break;
            }
        }

        if (safe)
            safeDirs.push_back(d);
    }

    if (!safeDirs.empty())
    {
        // 放炸弹
        ai->placeBomb(_scene, _scene->getMapLayer());

        // 选择安全方向移动
        int idx = rand() % safeDirs.size();
        ai->tryMoveTo(grid + safeDirs[idx], _scene->getMapLayer());

        return true;
    }

    return false; // 没找到安全格，不放炸弹
}
// -----------------------------
// 随机移动
// -----------------------------
void AIController::randomMove(Player* ai, AIState& state)
{
    std::vector<Vec2> dirs = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };
    Vec2 grid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    std::vector<Vec2> valid;

    for (auto d : dirs)
    {
        Vec2 next = grid + d;
        if (_scene->getMapLayer()->isWalkable(next.x, next.y) && !_scene->isGridDangerPublic(next))
            valid.push_back(d);
    }

    if (!valid.empty())
    {
        int idx = rand() % valid.size();
        state.nextDir = valid[idx];
    }
}