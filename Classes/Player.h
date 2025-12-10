#pragma once
#include "cocos2d.h"
#include "Bomb.h"

class MapLayer;

class Player : public cocos2d::Sprite
{
public:
    static Player* createPlayer();

    //------------------------------------------------------
    // 移动（现在使用 delta = 速度 * dt）
    //------------------------------------------------------
    void move(const cocos2d::Vec2& dir, class MapLayer* mapLayer);

    

    //------------------------------------------------------
    // 炸弹
    //------------------------------------------------------
    // 主接口（推荐使用）
    void placeBomb(cocos2d::Node* scene, MapLayer* mapLayer);
    void takeDamage();
    void die();
   

    // 炸弹冷却
    bool canPlaceBomb = true;
    float bombCooldown = 0.25f;   // 250ms

    //------------------------------------------------------
    // 玩家生命系统
    //------------------------------------------------------
    int hp = 3;
    bool invincible = false;
    bool isDead = false;
    float moveSpeed = 120.0f;
    int maxBombCount = 1;   // 最大炸弹数量
    int currentBombCount = 0;   // 场上已有几颗炸弹
   
    bool canMoveTo(const cocos2d::Vec2& newPos, MapLayer* mapLayer);
    void resetBombCooldown();
    // 开启短暂无敌（避免吃多次火焰）
    void startInvincible(float time = 1.0f);
};
