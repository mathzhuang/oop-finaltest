#pragma once
#include "cocos2d.h"
#include "Bomb.h"
#include "Item.h"

class MapLayer;

class Player : public cocos2d::Sprite
{
public:
    static Player* createPlayer();

    //======================================================
    // 移动系统
    //======================================================
    // dir 是单位方向（如 Vec2(1,0)），dt 在 GameScene 中计算
    void move(const cocos2d::Vec2& dir, MapLayer* mapLayer);

    // 判断某点能否移动（整合封装）
    bool canMoveTo(const cocos2d::Vec2& newPos, MapLayer* mapLayer);

    float moveSpeed = 120.0f;

    //======================================================
    // 炸弹系统
    //======================================================
    // 主接口：由 GameScene 调用
    void placeBomb(cocos2d::Node* scene, MapLayer* mapLayer);

    bool canPlaceBomb = true;        // 是否可放置炸弹
    float bombCooldown = 0.5f;       // 500ms CD

    int maxBombCount = 2;            // 最大可同时存在的炸弹
    int currentBombCount = 0;        // 当前场上属于玩家的炸弹数量

    // 冷却工具函数
    void resetBombCooldown();

    //======================================================
    // 玩家生命系统
    //======================================================
    int hp = 3;
    bool invincible = false;        // 吃到一次火焰后短暂无敌
    bool isDead = false;

    void takeDamage();
    void die();

    // 开启短暂无敌（避免连续受伤）
    void startInvincible(float time = 1.0f);

    // 道具相关（新增）
    int bombRange = 2;         // 默认炸弹范围
    float speed = 120.0f;      // 兼容你之前用的 speed 名称（或使用 moveSpeed，两者任选其一）
    // 如果你之前已经有 moveSpeed，请保持一致：可以用 moveSpeed 而不是 speed

// 道具拾取接口
    void pickItem(class Item* item);

};
