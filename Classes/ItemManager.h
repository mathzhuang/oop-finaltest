#pragma once
#include "cocos2d.h"
#include "Item.h"
#include "MapLayer.h"

class ItemManager : public cocos2d::Node
{
public:
    static ItemManager* create(MapLayer* map);

    bool init(MapLayer* map);

    // 在地图某个格子创建道具
    void spawnItemAt(int gx, int gy);

    // 地图破坏后掉落道具
    void dropItemFromTile(int gx, int gy);

    // 道具池（用于 GameScene 遍历检查碰撞）
    cocos2d::Vector<Item*> items;

private:
    MapLayer* _mapLayer = nullptr;

    // 随机生成一个道具类型
    Item::ItemType randomType();
};
