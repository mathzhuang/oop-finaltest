// AIController.cpp
#include "AIController.h"
#include "GameScene.h"
#include "Player.h"
#include "MapLayer.h"
#include "ItemManager.h"
#include "Bomb.h"
#include "Flame.h"
#include "AIState.h"

using namespace cocos2d;

AIController::AIController(GameScene* scene)
    : _scene(scene)
{
}

// -----------------------------
// 主 AI 更新函数（阶段 1+2+3）
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
    state.thinkCooldown = 0.25f;

    bool urgent = tryEscapeDanger(ai, state);
    if (urgent)
    {
        state.state = AIStateType::EscapeDanger;
        state.stateTime = 0;
    }
    else
    {
        if (state.stateTime >= state.minStateDuration || state.state == AIStateType::Idle)
        {
            bool chose = false;

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
            if (!chose)
                state.state = AIStateType::Idle;

            state.minStateDuration = 1.0f + CCRANDOM_0_1();
            state.stateTime = 0;
        }
    }

    // 执行当前状态动作
    switch (state.state)
    {
    case AIStateType::EscapeDanger:
        tryEscapeDanger(ai, state);
        break;
    case AIStateType::PickItem:
        tryPickItem(ai, state);
        break;
    case AIStateType::ChasePlayer:
        tryAttackPlayer(ai, state);
        break;
    case AIStateType::BreakWall:
        tryDestroyWall(ai, state);
        break;
    case AIStateType::Idle:
    default:
        randomMove(ai, state);
        break;
    }

    state.stateTime += dt;
}
// -----------------------------
// 逃离危险
// -----------------------------
bool AIController::tryEscapeDanger(Player* ai, AIState& state)
{
    Vec2 grid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    if (!_scene->isGridDangerPublic(grid)) return false;

    auto path = _scene->findSafePathBFS(grid);
    if (!path.empty())
    {
        state.nextDir = path[0];
        return true;
    }
    return false;
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
// 攻击玩家（安全放炸弹）
// -----------------------------
bool AIController::tryAttackPlayer(Player* ai, AIState& state)
{
    Player* target = _scene->findNearestPlayer(ai);
    if (!target) return false;

    Vec2 aiGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    Vec2 targetGrid = _scene->getMapLayer()->worldToGrid(target->getPosition());

    // 计算路径
    auto path = _scene->findPathToPlayer(aiGrid, target);
    if (!path.empty())
    {
        state.nextDir = path[0]; // 移动到路径第一步
    }

    // 决定是否放炸弹：在目标附近并且可以放炸弹
    if (ai->canPlaceBomb &&
        aiGrid.distance(targetGrid) <= 1.5f &&
        _scene->hasSafeEscape(aiGrid, ai)) // 确保周围有安全格
    {
        state.wantBomb = true;
    }
    else
    {
        state.wantBomb = false;
    }

    return true;
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
