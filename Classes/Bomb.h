#pragma once
#include "cocos2d.h"
#include <functional>

class MapLayer;

class Bomb : public cocos2d::Sprite
{
public:
    // 静态创建函数
    static Bomb* createBomb(int range = 2);

    // 倒计时与爆炸
    void startCountdown(const std::function<void()>& onExplode = nullptr);
    void explode();

    // 炸弹威力
    int range = 2;

    // 炸弹所在格子
    cocos2d::Vec2 gridPos;

    // 判断某个格子是否在炸弹威力范围内
    bool willExplodeGrid(const cocos2d::Vec2& targetGrid) const;

private:
    void createFlameAt(int gx, int gy, MapLayer* map, int zOrder /*=15*/);
};
