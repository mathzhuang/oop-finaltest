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
void ItemManager::spawnItemAt(int gx, int gy)
{
    Item::ItemType t = randomType();
    auto item = Item::createItem(t);
    if (!item) return;

    Vec2 world = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(world);

    this->addChild(item);
    items.pushBack(item);

    item->playSpawnAnimation();
}

// 地图破坏后掉落道具（概率控制）
void ItemManager::dropItemFromTile(int gx, int gy, int probability)
{
    if (RandomHelper::random_int(0, 100) < probability)
        spawnItemAt(gx, gy);
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
