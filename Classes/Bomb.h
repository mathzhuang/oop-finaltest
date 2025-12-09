#pragma once
#include "cocos2d.h"

class MapLayer;
class GameScene; // 在文件顶部补充前置声明

class Bomb : public cocos2d::Sprite
{
public:
    static Bomb* createBomb(int range = 3);
    void startCountdown();
    void explode();

private:
    int range = 3;

    void createFlameAt(int gx, int gy, MapLayer* map, cocos2d::Node* parent, GameScene* scene); // 修正参数类型
};
