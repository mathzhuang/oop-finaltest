#pragma once
#include "cocos2d.h"
#include "Item.h"

class MapLayer; // 前向声明

class ItemManager : public cocos2d::Node
{
public:
    // 创建管理器实例
    static ItemManager* create(MapLayer* map);

    bool init(MapLayer* map);

    // 在地图某个格子生成道具
    void spawnItemAt(int gx, int gy);

    // 地图破坏后掉落道具（可控制概率）
    void dropItemFromTile(int gx, int gy, int probability = 50);

    // 遍历检查玩家是否捡到道具
    void checkPlayerPickUp(class Player* player);

    // 道具池（GameScene遍历）
    cocos2d::Vector<Item*> items;

private:
    MapLayer* _mapLayer = nullptr;

    // 随机生成道具类型
    Item::ItemType randomType();
};
