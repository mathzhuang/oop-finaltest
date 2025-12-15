#include "ItemManager.h"
USING_NS_CC;

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

// ===============================
// 随机选择一个道具类型（去掉 RandomBox）
// ===============================
Item::ItemType ItemManager::randomType()
{
    // 现在只生成 0~4 五种道具
    int r = RandomHelper::random_int(0, 4);
    return static_cast<Item::ItemType>(r);
}

// ===============================
// 在指定格子生成道具
// ===============================
void ItemManager::spawnItemAt(int gx, int gy)
{
    Item::ItemType t = randomType();

    auto item = Item::createItem(t);

    Vec2 world = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(world);

    this->addChild(item);
    items.pushBack(item);

    item->playSpawnAnimation();
}

// ===============================
// 地图破坏后掉落道具（50% 概率）
// ===============================
void ItemManager::dropItemFromTile(int gx, int gy)
{
    if (RandomHelper::random_int(0, 100) < 50)
        spawnItemAt(gx, gy);
}
