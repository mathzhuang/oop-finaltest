#pragma once
#include "cocos2d.h"

class Player;

class Flame : public cocos2d::Sprite
{
public:
    static Flame* createFlame();

    cocos2d::Vec2 gridPos;  // 🔥 精准判定核心

    void setOwner(Player* p) { _owner = p; }
    Player* getOwner() const { return _owner; }

private:
    Player* _owner = nullptr;
};
