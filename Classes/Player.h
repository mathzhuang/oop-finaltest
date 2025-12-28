#pragma once
#include "cocos2d.h"
#include "Item.h"

// 前向声明
class MapLayer;
class GameScene;

class Player : public cocos2d::Sprite
{
public:
    // --- 工厂与生命周期 ---
    static Player* createPlayer();
    virtual bool init() override;
    virtual void onExit() override;

    // 设置角色外观 ID
    void setCharacter(int characterId);

    // 设置/获取玩家索引 (0~3, 用于UI更新)
    void setPlayerIndex(int idx) { _playerIndex = idx; }
    int getPlayerIndex() const { return _playerIndex; }

    // --- 移动系统 ---

    // 持续移动 (由键盘输入调用, 平滑移动)
    void move(const cocos2d::Vec2& dir, MapLayer* mapLayer);

    // 目标移动 (由 AI 调用, 点对点移动)
    bool tryMoveTo(const cocos2d::Vec2& nextGrid, MapLayer* map);

    // 碰撞检测预判
    bool canMoveTo(const cocos2d::Vec2& newPos, MapLayer* mapLayer);

    // 移动属性
    float moveSpeed = 100.0f;        // 当前速度
    float defaultMoveSpeed = 100.0f; // 基础速度
    float speedBoostTimer = 0.0f;    // 加速剩余时间
    float speedBoostMultiplier = 1.5f;

    // 移动状态
    bool isMoving = false;
    cocos2d::Vec2 currentGrid;
    cocos2d::Vec2 targetGrid;

    // --- 炸弹系统 ---

    // 放置炸弹
    void placeBomb(cocos2d::Node* scene, MapLayer* mapLayer);
    // 重置冷却
    void resetBombCooldown();

    // 炸弹属性
    bool canPlaceBomb = true;
    float bombCooldown = 1.5f;
    int maxBombCount = 3;
    int currentBombCount = 0;

    int bombRange = 2;              // 当前威力
    int _defaultBombRange = 2;      // 基础威力
    int _enhancedBombsLeft = 0;     // 加强炸弹剩余次数

    // --- 生存与状态 ---

    // 受伤与死亡
    void takeDamage();
    void die();

    // 状态标记
    int hp = 3;
    int maxHp = 5;
    bool isDead = false;
    bool invincible = false;    // 无敌状态 (受伤后闪烁)
    bool hasShield = false;     // 护盾道具状态

    // 异常状态 (路障定身)
    bool stunned = false;
    float stunTimer = 0.0f;

    // 开启无敌 (受击保护)
    void startInvincible(float time = 1.0f);

    // --- 道具与效果 ---

    // 拾取道具入口
    void pickItem(class Item* item);

    // 具体效果实现
    void increaseBombRange();                   // 增加威力
    void heal();                                // 治疗
    void activateShield(float duration);        // 护盾
    void speedUp(float duration, float factor); // 加速
    void blockOpponent();                       // 阻碍敌人
    void activateLightEffect(float duration);   // 灯光(迷雾)
    void showBlockEffect(float duration);       // 播放被阻碍特效

    // 视野逻辑 (迷雾模式)
    void updateVision(float dt);
    float getVisionRadius() const { return _visionRadius; }

    // 辅助查找：寻找最近敌人
    Player* findNearestEnemy(float minValidDist = 10.0f);

    // --- 数据与积分 ---

    int getScore() const { return _score; }
    void addScore(int value);

    int getItemCount() const { return _itemHoldCount; }
    void changeItemCount(int delta);

    // --- AI 配置 ---
    bool isAI = false;
    float aiAggressive = 0.5f;  // 攻击欲望
    float aiCoward = 0.5f;      // 逃跑倾向
    float aiCuriosity = 0.5f;   // 探索倾向

    // 持有场景引用 (方便交互)
    GameScene* _scene = nullptr;

private:
    // --- 内部数据 ---
    int _characterId = 1;
    int _playerIndex = -1;
    int _score = 0;
    int _itemHoldCount = 0;

    // 视野内部变量
    float _visionRadius = 150.0f;
    float _lightTimer = 0.0f;

    // --- 动画系统 ---
    enum class Direction { None, Up, Down, Left, Right };
    Direction _currentDirection = Direction::None;
    static const int ACTION_TAG_WALK = 999;

    void updateWalkAnimation(const cocos2d::Vec2& dir);
    void stopWalkAnimation();
    cocos2d::Action* createWalkAction(Direction dir);

    // --- 音效 ---
    int _walkAudioID = -1;
};