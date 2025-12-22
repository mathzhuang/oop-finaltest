#pragma once
#include "cocos2d.h"
#include "Item.h"   // ★ 加这一行

class MapLayer;
class GameScene; // 前向声明


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

    float moveSpeed = 100.0f;         // 当前速度
    float defaultMoveSpeed = 100.0f;  // 恢复速度用
    float speedBoostTimer = 0.0f;     // 加速剩余时间
    float speedBoostMultiplier = 1.5f;

    // 👇 新增：格子制移动接口
    bool tryMoveTo(const cocos2d::Vec2& nextGrid, MapLayer* map);

    // ===== 状态 =====
    bool isMoving = false;
    cocos2d::Vec2 currentGrid;
    cocos2d::Vec2 targetGrid;


    //======================================================
    // 炸弹系统
    //======================================================
    void placeBomb(cocos2d::Node* scene, MapLayer* mapLayer);
    void resetBombCooldown();

    bool canPlaceBomb = true;
    float bombCooldown = 1.5f;

    int maxBombCount = 1;
    int currentBombCount = 0;

    int bombRange = 2;

    //======================================================
    // 生命系统
    //======================================================
    bool Player::init();
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
  // 道具效果接口
  void increaseBombRange();
  void heal();
  void activateShield(float duration);
  void speedUp(float duration, float factor);
  void blockOpponent();

  GameScene* _scene = nullptr;


  //======================================================
   // ai
   //======================================================
  bool isAI = false;
  // AI 性格
  float aiAggressive = 0.5f;  // 越高越喜欢攻击
  float aiCoward = 0.5f;      // 越高越容易逃跑
  float aiCuriosity = 0.5f;   // 越高越喜欢捡道具/拆墙
private:
    int _characterId = 1; // 默认角色编号

 
   
};
