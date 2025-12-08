#pragma once
#include "cocos2d.h"

class MapLayer; // 前向声明

class Bomb : public cocos2d::Sprite
{
public:
    // 创建炸弹，range 默认 1（你可以改）
    static Bomb* createBomb(int range = 3);

    // 开始倒计时并在结束时 explode()
    void startCountdown();

    // 直接触发爆炸（测试用）
    void explode();

private:
    int range = 3;
    // 私有帮助函数：在指定网格位置生成火焰（并自动消失）
    void createFlameAt(int gx, int gy, MapLayer* map, cocos2d::Node* parent);
};
