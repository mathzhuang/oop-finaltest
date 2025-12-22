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
    Item* spawnItemAt(int gx, int gy);

    // 统一掉落道具接口
    Item* dropItem(int gx, int gy, int probability = 35);           // 通过格子坐标
    Item* dropItem(const cocos2d::Vec2& worldPos, int probability = 35); // 通过世界坐标
    // 遍历检查玩家是否捡到道具
    void checkPlayerPickUp(class Player* player);

  

    bool hasItemAtGrid(const cocos2d::Vec2& grid) const;

 

    cocos2d::Vector<Item*>& getItems() { return items; } // 外部可访问
   

private:
    MapLayer* _mapLayer = nullptr;

    // 随机生成道具类型
    Item::ItemType randomType();

    // 道具池（GameScene遍历）
    cocos2d::Vector<Item*> items;
};
