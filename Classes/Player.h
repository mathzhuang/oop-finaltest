#pragma once
#include "cocos2d.h"
#include "Item.h"   // ★ 加这一行

class MapLayer;


class Player : public cocos2d::Sprite
{
public:
    static Player* createPlayer();
    void setCharacter(int characterId);

    //======================================================
    // 移动系统
    //======================================================
    void move(const cocos2d::Vec2& dir, MapLayer* mapLayer);
    bool canMoveTo(const cocos2d::Vec2& newPos, MapLayer* mapLayer);

    float moveSpeed = 120.0f;         // 当前速度
    float defaultMoveSpeed = 120.0f;  // 恢复速度用
    float speedBoostTimer = 0.0f;     // 加速剩余时间
    float speedBoostMultiplier = 1.5f;

    //======================================================
    // 炸弹系统
    //======================================================
    void placeBomb(cocos2d::Node* scene, MapLayer* mapLayer);
    void resetBombCooldown();

    bool canPlaceBomb = true;
    float bombCooldown = 0.5f;

    int maxBombCount = 2;
    int currentBombCount = 0;

    int bombRange = 2;

    //======================================================
    // 生命系统
    //======================================================
    int hp = 3;
    int maxHp = 5;

    bool invincible = false;          // 受伤保护
    bool hasShield = false;           // 安全帽
    bool isDead = false;

    void takeDamage();
    void die();
    void startInvincible(float time = 1.0f);

    //======================================================
    // 异常状态
    //======================================================
    bool stunned = false;             // 路障效果
    float stunTimer = 0.0f;

    //======================================================
    // 道具处理
    //======================================================
    // 道具拾取接口
void pickItem(class Item* item);

// ★ 新增这一行
void applyItemEffect(Item::ItemType type);
private:
    int _characterId = 1; // 默认角色编号
};
