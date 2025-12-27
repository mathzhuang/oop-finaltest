#include "ItemManager.h"
#include "Player.h"
#include "MapLayer.h"
#include "base/ccRandom.h" // 替换为 cocos2d-x 官方随机头文件


USING_NS_CC;

// 创建实例
ItemManager* ItemManager::create(MapLayer* map)
{
    ItemManager* ret = new ItemManager();
    if (ret && ret->init(map))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ItemManager::init(MapLayer* map)
{
    _mapLayer = map;
    return true;
}



// 在指定格子生成道具
Item* ItemManager::spawnItemAt(int gx, int gy)
{
    if (!_mapLayer) return nullptr;

    // 统一使用 Item 类的工厂方法
    auto item = Item::createRandom();
    if (!item) return nullptr;

    // 建议：统一添加到 ItemManager 下，或者统一添加到 _mapLayer 下
    // 如果添加到 ItemManager(this)，请确保 ItemManager 本身在场景里
    Vec2 worldPos = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(worldPos);

    // 如果 this 是一个没有设置大小的 Layer，直接 addChild 即可
    this->addChild(item, 10);

    items.pushBack(item);

    CCLOG("SpawnItem: type=%d at grid(%d,%d)", static_cast<int>(item->getType()), gx, gy);
    item->playSpawnAnimation();
    return item;
}
// 地图破坏后掉落道具（概率控制）
// 核心掉落逻辑（两种）
Item* ItemManager::dropItem(int gx, int gy, int probability)
{
    bool isFog = (_mapLayer->getGameMode() == GameMode::FOG);
    Item* item = Item::createRandom(isFog); // 传入当前的迷雾模式状态

    if (!_mapLayer) return nullptr;
    if (RandomHelper::random_int(0, 100) >= probability) return nullptr;

   
    if (!item) return nullptr;

    // --- 新增：迷雾道具过滤 ---
    // 检查：如果抽到了灯光，但当前不是迷雾模式
    if (item->getType() == Item::ItemType::Light) {
        if (_mapLayer->getGameMode() != GameMode::FOG) {
            // 销毁并替换为其他道具，比如加速
            item = Item::createItem(Item::ItemType::SpeedUp);
        }
    }
    // ------------------------

    Vec2 worldPos = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(worldPos);
    this->addChild(item, 10);
    items.pushBack(item);
    item->playSpawnAnimation();
    return item;
}
Item* ItemManager::dropItem(const Vec2& worldPos, int probability)
{
    if (!_mapLayer) return nullptr;

    // 转换世界坐标为格子坐标
    Vec2 grid = _mapLayer->worldToGrid(worldPos);
    int gx = static_cast<int>(grid.x);
    int gy = static_cast<int>(grid.y);

    return dropItem(gx, gy, probability); // 调用核心逻辑
}


// 遍历道具池，检查玩家是否碰到
void ItemManager::checkPlayerPickUp(Player* player)
{
    if (!player) return;

    auto playerPos = player->getPosition();

    // 使用Vector迭代器遍历
    for (auto it = items.begin(); it != items.end(); )
    {
        Item* item = *it;
        if (!item) { ++it; continue; }

        // 简单碰撞检测（可改成更精确的）
        if (player->getBoundingBox().intersectsRect(item->getBoundingBox()))
        {
            // 玩家触发道具
            player->pickItem(item);

            // 移除道具节点
            item->removeFromParent();
            it = items.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
bool ItemManager::hasItemAtGrid(const Vec2& grid) const
{
    for (auto item : items)
    {
        if (!item) continue;

        Vec2 g = _mapLayer->worldToGrid(item->getPosition());
        if (g == grid)
            return true;
    }
    return false;
}
// ItemManager.cpp
