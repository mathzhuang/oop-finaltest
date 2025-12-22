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

// 随机道具类型
Item::ItemType ItemManager::randomType()
{
    // 生成 0~4 五种道具
    int r = RandomHelper::random_int(0, 4);
    return static_cast<Item::ItemType>(r);
}

// 在指定格子生成道具
Item* ItemManager::spawnItemAt(int gx, int gy)
{
    if (!_mapLayer) return nullptr;

    // 随机生成一个道具类型
    Item::ItemType type = static_cast<Item::ItemType>(RandomHelper::random_int(0, 4));
    auto item = Item::createItem(type);
    if (!item) return nullptr;

    // 直接挂在 GameScene 下
    auto parentScene = this->getParent();
    if (parentScene)
    {
        Vec2 worldPos = _mapLayer->gridToWorld(gx, gy);
        item->setPosition(worldPos);
        parentScene->addChild(item, 10); // 层级随意，保证可见

        items.pushBack(item);

        CCLOG("Dropped item type=%d at grid (%d,%d), world=(%.2f,%.2f)",
            static_cast<int>(type), gx, gy, worldPos.x, worldPos.y);
    }

    item->playSpawnAnimation();
    return item;
}
// 地图破坏后掉落道具（概率控制）
// 核心掉落逻辑
Item* ItemManager::dropItem(int gx, int gy, int probability)
{
    if (!_mapLayer) return nullptr;

    if (RandomHelper::random_int(0, 100) >= probability)
        return nullptr;

    // 随机生成道具类型  
    Item::ItemType type = static_cast<Item::ItemType>(
        RandomHelper::random_int(0, 4) // 0~4 五种道具
        );
    Item* item = Item::createItem(type);
    if (!item) return nullptr;

    // 坐标转换到世界坐标
    Vec2 worldPos = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(worldPos);

    // 添加到 ItemManager 节点下
    this->addChild(item, 5);
    item->playSpawnAnimation();

    items.pushBack(item);

    CCLOG("Dropped item type=%d at grid (%d,%d), world=(%.2f,%.2f)",
        static_cast<int>(type), gx, gy, worldPos.x, worldPos.y);

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
