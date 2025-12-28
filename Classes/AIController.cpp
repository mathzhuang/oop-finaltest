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

// ⭐ 修改函数定义，增加 bool isSmart
float AIController::getHeatValue(const Vec2& grid, bool isSmart)
{
    auto map = _scene->getMapLayer();

    // 1. 【基础生存本能】墙壁判定 (无论笨还是聪明都要判断)
    if (!map || !map->isWalkable(grid.x, grid.y)) {
        return 999.0f; // 绝对不可走
    }

    float score = 0.0f;

    // 2. 【基础生存本能】物理火焰 (踩上去就死，傻瓜也知道躲)
    if (map->getTile(grid.x, grid.y) == 300) {
        score += 1000.0f; // 致命危险
    }

    // 3. 【炸弹预警】
    // 简单 AI：只看眼前有没有炸弹，对即将爆炸的才敏感
    // 困难 AI：能看到更远的连锁反应（这里简化处理）
    // 遍历 GameScene 中的预警
    for (const auto& danger : _scene->getBombDangers())
    {
        if (danger.willExplodeGrid(grid))
        {
            // 如果是简单AI，它可能对“还有很久才爆炸”的炸弹不敏感
            if (!isSmart && danger.timeLeft > 1.5f) {
                continue; // 笨蛋觉得还早，不用躲
            }

            // 距离爆炸越近，分数越高
            float urgency = 4.0f - danger.timeLeft;
            if (urgency < 0) urgency = 0;
            score += (50.0f * urgency);
        }
    }

    // 4. 【高级智商】地形风险评估 (只有 Smart 才会思考)
    if (isSmart)
    {
        // 判定死胡同：周围有3面墙的地方很危险，容易被堵死
        int wallCount = 0;
        Vec2 dirs[] = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };
        for (auto d : dirs) {
            Vec2 neighbor = grid + d;
            if (!map->isWalkable(neighbor.x, neighbor.y)) {
                wallCount++;
            }
        }

        // 如果是死胡同 (3面墙)，由于容易被堵住，增加一点危险分
        if (wallCount >= 3) {
            score += 5.0f;
        }
        // 如果是狭窄通道 (2面墙)，稍微加一点分
        else if (wallCount == 2) {
            score += 5.0f;
        }
    }

    return score;
}
// -----------------------------
// 主 AI 更新函数
// -----------------------------
void AIController::updateAI(float dt, Player* ai, AIState& state)
{
    if (!ai || ai->isDead) return;

    // 1. --- 卡死检测 ---
    Vec2 currentGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());
    //if (currentGrid == state.lastGridPos)
    //{
    //    state.stuckTimer += dt;
    //    // 如果在非躲避状态下原地不动超过 1.5秒，视为卡死
    //    if (state.stuckTimer > 1.5f && state.state != AIStateType::EscapeDanger)
    //    {
    //        state.isStuck = true;
    //    }
    //}
    //else
    //{
    //    state.lastGridPos = currentGrid;
    //    state.stuckTimer = 0.0f;
    //    state.isStuck = false;
    //}
    state.stuckTimer += dt;
    if (state.stuckTimer > 0.2f) // 每 0.2 秒记录一次
    {
        state.stuckTimer = 0.0f;
        state.positionHistory.push_back(currentGrid);

        // 保持最近 10 个记录
        if (state.positionHistory.size() > 6) {
            state.positionHistory.erase(state.positionHistory.begin());
        }

        // 2. 分析历史记录
        // 如果最近 10 次记录里，包含的“不同格子数量”<= 2，说明在反复横跳
        if (state.positionHistory.size() >= 8) // 记录足够多时才判断
        {
            std::vector<Vec2> uniquePos;
            for (auto& pos : state.positionHistory) {
                bool exists = false;
                for (auto& u : uniquePos) if (u == pos) exists = true;
                if (!exists) uniquePos.push_back(pos);
            }

            // 如果一直在 1个 或 2个 格子之间徘徊，判定为卡死
            if (uniquePos.size() <= 4) {
                state.isStuck = true;
                handleStuck(ai, state);
                state.stuckTimer = 0.0f; // 重置计时
                state.positionHistory.clear(); // 清空历史，防止下一帧连续触发
                CCLOG("AI Stuck Detected: Oscillation!");
                state.isStuck = false;
            }
        }
    }

    //冷却
    state.thinkCooldown -= dt;
    if (state.thinkCooldown > 0)
    {
        state.stateTime += dt;
        return;
    }
    // 修改开始：根据难度设置反应时间
    if (state.difficulty == AIDifficulty::HARD)
    {
        // 困难：0.1秒思考一次，反应极快
        state.thinkCooldown = 0.1f;
    }
    else
    {
        // 简单：0.5 ~ 0.8秒才动一下脑子，显得迟钝
        state.thinkCooldown = 0.5f + CCRANDOM_0_1() * 0.3f;
    }
    // ⭐ 修改结束

    //// 优先处理卡死
    //if (state.isStuck)
    //{
    //    handleStuck(ai, state);
    //    state.positionHistory.clear();
    //    state.isStuck = false;
    //    state.stuckTimer = 0.0f;
    //    return;
    //}

    // 处理生存：避灾逻辑
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
    // ⭐ 核心修改：根据难度决定是否使用“聪明模式”
    bool isSmart = (state.difficulty == AIDifficulty::HARD);
    // 获取当前格子的危险值
    float currentHeat = getHeatValue(currentGrid, isSmart);
    // 简单 AI 对危险容忍度高（迟钝），困难 AI 容忍度低（敏感）
    float threshold = isSmart ? 0.1f : 30.0f;

   
    // 如果当前格子完全安全，则无需逃跑
    if (currentHeat <= 0.1f) return false;

    Vec2 bestDir = Vec2::ZERO;
    float minHeat = currentHeat;

    const Vec2 dirs[4] = { Vec2(0,1), Vec2(0,-1), Vec2(-1,0), Vec2(1,0) };
    for (auto d : dirs) {
        Vec2 next = currentGrid + d;
        // ⭐ 这里也要传 isSmart，否则它不知道哪里是安全的
        float h = getHeatValue(next, isSmart);
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
// 处理卡死 (强制脱困)
// -----------------------------
void AIController::handleStuck(Player* ai, AIState& state)
{
    Vec2 currentGrid = _scene->getMapLayer()->worldToGrid(ai->getPosition());

    // 策略 A: 如果周围安全，尝试放炸弹 (可能是被软墙困住)
    if (ai->canPlaceBomb && getHeatValue(currentGrid, false) < 100.0f)
    {
        // 只有有退路才放，防止自杀
        if (_scene->hasSafeEscape(currentGrid, ai)) {
            state.wantBomb = true;
            CCLOG("AI Stuck: Placing bomb to clear path.");
            return;
        }
    }

    // 2. 智能换向逻辑
    std::vector<Vec2> dirs = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };

    std::vector<Vec2> newTerritoryDirs; // 没去过的新方向
    std::vector<Vec2> oldTerritoryDirs; // 刚去过的老方向

    for (auto d : dirs)
    {
        Vec2 next = currentGrid + d;

        // A. 物理检测
        if (!_scene->getMapLayer()->isWalkable(next.x, next.y)) continue;

        // B. 危险检测 (只避开必死格，忽略普通危险)
        if (getHeatValue(next, false) >= 800.0f) continue;

        // C. 历史记录比对
        bool visitedRecently = false;
        for (const auto& historyPos : state.positionHistory) {
            if (historyPos == next) {
                visitedRecently = true;
                break;
            }
        }

        if (!visitedRecently) {
            newTerritoryDirs.push_back(d);
        }
        else {
            oldTerritoryDirs.push_back(d);
        }
    }

    // 3. 决策：优先去没去过的地方
    if (!newTerritoryDirs.empty()) {
        int idx = rand() % newTerritoryDirs.size();
        state.nextDir = newTerritoryDirs[idx];
        CCLOG("AI Stuck: Moving to NEW territory to break loop.");
    }
    else if (!oldTerritoryDirs.empty()) {
        // 如果四周都去过了(比如被围在死胡同)，那就随机选一个老路硬着头皮走
        int idx = rand() % oldTerritoryDirs.size();
        state.nextDir = oldTerritoryDirs[idx];
        CCLOG("AI Stuck: No new path, forcing random backtrack.");
    }
    else {
        // 彻底无路可走，原地放雷自爆或等待
        if (ai->canPlaceBomb) state.wantBomb = true;
    }
  
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

    //if (!path.empty()) state.nextDir = path[0];

    //// 决定是否放炸弹
    //// 判定条件：距离目标足够近
    //if (ai->canPlaceBomb && aiGrid.distance(_scene->getMapLayer()->worldToGrid(target->getPosition())) <= 1.5f)
    //{
    //    // ⭐ 修改点：根据难度决定是否检查退路
    //    bool checkSafety = true;

    //    if (state.difficulty == AIDifficulty::SIMPLE) {
    //        // 简单 AI：50% 的概率不检查退路（容易自杀）
    //        if (CCRANDOM_0_1() < 0.5f) checkSafety = false;
    //    }

    //    if (checkSafety) {
    //        // 只有在必须检查安全，且确实安全时，才放
    //        if (_scene->hasSafeEscape(aiGrid, ai)) {
    //            state.wantBomb = true;
    //        }
    //    }
    //    else {
    //        // 笨蛋模式：不管安不安全，直接放！
    //        state.wantBomb = true;
    //    }
    //}
    if (!path.empty())
    {
        state.nextDir = path[0];

        // 距离近尝试放炸弹
        if (ai->canPlaceBomb && aiGrid.distance(_scene->getMapLayer()->worldToGrid(target->getPosition())) <= 2.0f) {
            bool checkSafety = (state.difficulty == AIDifficulty::HARD);
            // 简单 AI 一半概率乱放
            if (state.difficulty == AIDifficulty::SIMPLE && CCRANDOM_0_1() < 0.5f) checkSafety = false;

            if (!checkSafety || _scene->hasSafeEscape(aiGrid, ai)) {
                state.wantBomb = true;
            }
        }
    }
    else
    {
        // 想追人但路不通，尝试炸开路
        return tryDestroyWall(ai, state);
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

        // ⭐ 修改点 1：检查前方是不是就是软墙
        // path[0] 是方向向量 (比如 1,0)，我们要看 (当前格 + 方向) 是不是软墙
        Vec2 nextGrid = aiGrid + path[0];
        auto map = _scene->getMapLayer();

        // 假设软墙的 Tile ID 是某个范围，或者用 isSoftWall 判断
        // 这里假设 path 既然找到了，说明 nextGrid 可能是墙，或者仅仅是路
        // 通常只有当 AI 被墙挡住（距离墙很近）才炸

        // ⭐ 优化：不仅仅检查 getTile==2，只要前方不可走，就视为需要炸开
        bool isWalkable = map->isWalkable(nextGrid.x, nextGrid.y);
        bool isHardWall = (map->getTile(nextGrid.x, nextGrid.y) == 1); // 假设1是硬墙
        if (!isWalkable && !isHardWall && ai->canPlaceBomb)
        {
            bool checkSafety = (state.difficulty == AIDifficulty::HARD);
            if (state.difficulty == AIDifficulty::SIMPLE && CCRANDOM_0_1() < 0.3f) checkSafety = true;

            if (!checkSafety || _scene->hasSafeEscape(aiGrid, ai)) {
                state.wantBomb = true;
            }
        }
        else {
            state.wantBomb = false;
        }

        //bool isWallAhead = (map->getTile(nextGrid.x, nextGrid.y) == /*软墙ID, 比如2*/ 2);

        //if (isWallAhead && ai->canPlaceBomb)
        //{
        //    // ⭐ 修改点 2：难度区分
        //    bool checkSafety = (state.difficulty == AIDifficulty::HARD); // 困难必须检查
        //    // 简单模式稍微检查一下，不然炸墙就把自己炸死太蠢了，即使是简单AI
        //    if (state.difficulty == AIDifficulty::SIMPLE && CCRANDOM_0_1() < 0.3f) checkSafety = true;

        //    if (checkSafety) {
        //        if (_scene->hasSafeEscape(aiGrid, ai)) state.wantBomb = true;
        //    }
        //    else {
        //        state.wantBomb = true;
        //    }
        //}
        //else {
        //    state.wantBomb = false;
        //}
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

    // 确定是否开启“聪明模式”
    bool isSmart = (state.difficulty == AIDifficulty::HARD);

    for (auto d : dirs)
    {
        Vec2 next = grid + d;
        // 1. 必须能走
        if (!_scene->getMapLayer()->isWalkable(next.x, next.y)) continue;

        // 2. ⭐ 修改点：使用统一的热力值判断
        // 如果热力值超过 0 (或者一个很小的阈值)，说明有危险
        float heat = getHeatValue(next, isSmart);

        // 简单 AI 容忍度高一点(比如10)，困难 AI 容忍度低(0.1)
        float tolerance = isSmart ? 0.1f : 10.0f;

        if (heat <= tolerance) {
            valid.push_back(d);
        }
    }

    if (!valid.empty())
    {
        int idx = rand() % valid.size();
        state.nextDir = valid[idx];
    }
}