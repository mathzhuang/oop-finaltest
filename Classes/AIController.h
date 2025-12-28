#pragma once
#include "cocos2d.h"
#include<vector>

class Player;
class MapLayer;
class ItemManager;
class GameScene;

enum class AIDifficulty {
    SIMPLE, // 简单：反应慢，随机性强，容易自杀
    HARD    // 困难：反应快，会预判逃生，战术封路
};
enum class AIStateType
{
    Idle,           // 闲逛
    ChasePlayer,    // 追击玩家
    EscapeDanger,   // 躲炸弹
    PickItem,       // 捡道具
    BreakWall       // 炸箱子
};

// AI 内部状态
struct AIState
{
    AIDifficulty difficulty = AIDifficulty::SIMPLE; // 默认为简单   
    AIStateType state = AIStateType::Idle;

    cocos2d::Vec2 nextDir = cocos2d::Vec2::ZERO;
    bool wantBomb = false;

    float thinkCooldown = 0.0f;  // 每帧思考冷却
    float stateTime = 0.0f;       // 当前状态持续时间

    float minStateDuration = 1.0f + CCRANDOM_0_1(); // 每个状态至少持续时间，增加个随机性
    float aggressiveness = 0.5f;   // 攻击意愿 (0~1)
    float curiosity = 0.5f;        // 捡道具意愿 (0~1)

    //cocos2d::Vec2 lastGridPos = cocos2d::Vec2(-1, -1); // 上一次所在的格子
    std::vector<cocos2d::Vec2> positionHistory;
    float stuckTimer = 0.0f;       // 待在同一个格子的时间
    bool isStuck = false;          // 是否判定为卡死
};

class Player;
class MapLayer;
class ItemManager;
class GameScene;

class AIController
{
public:
    AIController(GameScene* scene);

    void updateAI(float dt, Player* ai, AIState& state);

    // 公开 getHeatValue 以便 GameScene 调用
    float getHeatValue(const cocos2d::Vec2& grid, bool isSmart = true);

private:
    void randomMove(Player* ai, AIState& state);

    GameScene* _scene = nullptr;

    // 决策子模块
    bool tryEscapeDanger(Player* ai, AIState& state);
    bool tryPickItem(Player* ai, AIState& state);
    bool tryAttackPlayer(Player* ai, AIState& state);
    bool tryDestroyWall(Player* ai, AIState& state);

    // 安全放炸弹
    bool tryPlaceBombSafely(Player* ai);

    //处理卡死
    void handleStuck(Player* ai, AIState& state);

    // 工具函数
    Player* findNearestPlayer(Player* ai);
    bool hasSafeEscape(const cocos2d::Vec2& grid);
};