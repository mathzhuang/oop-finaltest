#pragma once
#include "cocos2d.h"

// 前向声明
class Player;
class MapLayer;
class ItemManager;
class GameScene;

// --- 类型定义 ---

// AI 智商等级
enum class AIDifficulty {
    SIMPLE, // 简单：反应慢，随机性强，容易自杀
    HARD    // 困难：反应快，会预判逃生，战术封路
};

// AI 当前行为状态
enum class AIStateType
{
    Idle,           // 闲逛/发呆
    ChasePlayer,    // 追击玩家
    EscapeDanger,   // 紧急避险 (躲炸弹/火焰)
    PickItem,       // 拾取道具
    BreakWall       // 炸毁障碍
};

// AI 运行时数据 (每个 AI Player 持有一份)
struct AIState
{
    AIDifficulty difficulty = AIDifficulty::SIMPLE;
    AIStateType state = AIStateType::Idle;

    cocos2d::Vec2 nextDir = cocos2d::Vec2::ZERO; // 下一步移动方向
    bool wantBomb = false;               // 是否有放炸弹的意图

    float thinkCooldown = 0.0f;                // 思考冷却 (模拟反应延迟)
    float stateTime = 0.0f;                // 当前状态持续时长
    float minStateDuration = 1.0f;                // 状态最小保持时间 (防抖动)

    // 性格参数 (0.0 ~ 1.0)
    float aggressiveness = 0.5f;                // 攻击欲望
    float curiosity = 0.5f;                // 拾取欲望
};

// --- 控制器类 ---

class AIController
{
public:
    // --- 生命周期 ---
    AIController(GameScene* scene);
    virtual ~AIController(); // 建议加上析构函数

    // --- 核心接口 ---

    // 每帧更新 AI 逻辑 (传入 AI 玩家实体及其状态数据)
    void updateAI(float dt, Player* ai, AIState& state);

    // 获取某坐标的危险热力值 (GameScene 也会调用此函数用于调试绘制)
    // 返回值越高越危险 (如火焰、炸弹爆炸范围)
    float getHeatValue(const cocos2d::Vec2& grid);

private:
    // --- 决策子模块 ---

    // 尝试规避危险 (优先级最高)
    bool tryEscapeDanger(Player* ai, AIState& state);

    // 尝试寻找并拾取道具
    bool tryPickItem(Player* ai, AIState& state);

    // 尝试攻击最近的敌人
    bool tryAttackPlayer(Player* ai, AIState& state);

    // 尝试炸毁软墙 (获取道具或开路)
    bool tryDestroyWall(Player* ai, AIState& state);

    // 安全放炸弹检测 (预演放弹后是否有逃生路径)
    bool tryPlaceBombSafely(Player* ai);

    // 随机漫步 (无事可做时的兜底逻辑)
    void randomMove(Player* ai, AIState& state);

private:
    GameScene* _scene = nullptr;
};