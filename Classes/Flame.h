#pragma once
#include "cocos2d.h"

// 前向声明
class Player;

class Flame : public cocos2d::Sprite
{
public:
    // --- 工厂方法 ---
    static Flame* createFlame();

    // --- 属性访问 ---
    void setOwner(Player* p) { _owner = p; }
    Player* getOwner() const { return _owner; }

public:
    // --- 公有变量 ---
    cocos2d::Vec2 gridPos;  // 🔥 用于精准碰撞检测的网格坐标

private:
    Player* _owner = nullptr;
};