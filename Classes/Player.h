#pragma once
#include "cocos2d.h"

class MapLayer;

class Player : public cocos2d::Sprite
{
public:
    static Player* createPlayer();

    // 基于地图的移动（dir 要规范化：上下左右）
    void move(const cocos2d::Vec2& dir, MapLayer* mapLayer);

    // 兼容你之前的接口（保留）
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    // 放炸弹（会调用 Bomb）
    void placeBomb();
	// 玩家死亡
    int hp = 3;
    bool invincible = false;

    void takeDamage();
    void die();

    bool isDead = false;

};
