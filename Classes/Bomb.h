#pragma once
#include "cocos2d.h"
#include <functional>

class MapLayer;

class Bomb : public cocos2d::Sprite
{
public:
    static Bomb* createBomb(int range = 2);
    void startCountdown(const std::function<void()>& onExplode = nullptr);
    void explode();
    int range = 2;
private:
   

    void createFlameAt(int gx, int gy, MapLayer* map, cocos2d::Node* parent);
};
