#pragma once
#include "cocos2d.h"
#include <functional>

class MapLayer;
class Player;

class Bomb : public cocos2d::Sprite
{
public:
    // --- 工厂方法 ---
    static Bomb* createBomb(int range = 2);

    // --- 核心逻辑 ---

    // 开始倒计时 (onExplode: 爆炸回调)
    void startCountdown(const std::function<void()>& onExplode = nullptr);

    // 执行爆炸
    void explode();

    // 判断目标格子是否在当前炸弹威力范围内
    bool willExplodeGrid(const cocos2d::Vec2& targetGrid) const;

    // --- 属性访问 ---

    void setOwner(Player* p) { _owner = p; }
    Player* getOwner() const { return _owner; }

public:
    // --- 公有变量 ---
    int range = 2;          // 炸弹威力
    cocos2d::Vec2 gridPos;  // 炸弹所在格子坐标

private:
    // --- 内部实现 ---
    void createFlameAt(int gx, int gy, MapLayer* map, int zOrder = 15);

private:
    Player* _owner = nullptr; // 放置者
};