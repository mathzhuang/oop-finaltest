#pragma once
#include "cocos2d.h"
#include "Item.h"
#include "GameMode.h"
#include "SelectScene.h"

class MapLayer;

class ItemManager : public cocos2d::Node
{
public:
    // --- 生命周期 ---
    static ItemManager* create(MapLayer* map);
    bool init(MapLayer* map);

    // --- 核心生成接口 ---

    // 在指定格子强制生成一个随机道具
    Item* spawnItemAt(int gx, int gy);

    // 尝试概率掉落道具 (常用于障碍物销毁时)
    // probability: 掉落几率 (0-100)
    Item* dropItem(int gx, int gy, int probability = 35);
    Item* dropItem(const cocos2d::Vec2& worldPos, int probability = 35);

    // --- 交互逻辑 ---

    // 每一帧检测：玩家是否碰到了道具
    void checkPlayerPickUp(class Player* player);

    // --- 查询与数据 ---

    // 检查该格子是否已经存在道具 (防止重叠)
    bool hasItemAtGrid(const cocos2d::Vec2& grid) const;

    // 获取当前活动道具列表
    cocos2d::Vector<Item*>& getItems() { return items; }

private:
    MapLayer* _mapLayer = nullptr;
    cocos2d::Vector<Item*> items; // 道具容器，负责内存管理
};