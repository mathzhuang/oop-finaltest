#pragma once
#include "cocos2d.h"

class Flame : public cocos2d::Sprite
{
public:
    static Flame* createFlame();

    cocos2d::Vec2 gridPos;  // 🔥 精准判定核心
};
