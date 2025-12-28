#include "ItemManager.h"
#include "Player.h"
#include "MapLayer.h"
#include "base/ccRandom.h"

USING_NS_CC;

// --- 生命周期 ---

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

// --- 道具生成逻辑 ---

Item* ItemManager::dropItem(const Vec2& worldPos, int probability)
{
    if (!_mapLayer) return nullptr;

    // 坐标转换：世界坐标 -> 逻辑网格
    Vec2 grid = _mapLayer->worldToGrid(worldPos);
    int gx = static_cast<int>(grid.x);
    int gy = static_cast<int>(grid.y);

    // 转发给核心逻辑
    return dropItem(gx, gy, probability);
}

Item* ItemManager::dropItem(int gx, int gy, int probability)
{
    if (!_mapLayer) return nullptr;

    // 1. 概率判定 (未命中直接返回)
    if (RandomHelper::random_int(0, 100) >= probability) return nullptr;

    // 2. 根据当前游戏模式预生成道具
    bool isFog = (_mapLayer->getGameMode() == GameMode::FOG);
    Item* item = Item::createRandom(isFog);

    if (!item) return nullptr;

    // 3. 逻辑修正：非迷雾模式下屏蔽 Light 道具
    // 如果随机到了 Light 但当前不是迷雾模式，强制替换为 SpeedUp
    if (item->getType() == Item::ItemType::Light && !isFog) {
        item = Item::createItem(Item::ItemType::SpeedUp);
    }

    // 4. 放置道具
    Vec2 worldPos = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(worldPos);
    this->addChild(item, 10);
    items.pushBack(item);

    // 5. 播放出生动画
    item->playSpawnAnimation();

    return item;
}

Item* ItemManager::spawnItemAt(int gx, int gy)
{
    if (!_mapLayer) return nullptr;

    // 强制在指定位置生成一个随机道具 (调试或特殊逻辑用)
    auto item = Item::createRandom();
    if (!item) return nullptr;

    Vec2 worldPos = _mapLayer->gridToWorld(gx, gy);
    item->setPosition(worldPos);
    this->addChild(item, 10);
    items.pushBack(item);

    CCLOG("Force SpawnItem: type=%d at grid(%d,%d)", static_cast<int>(item->getType()), gx, gy);
    item->playSpawnAnimation();

    return item;
}

// --- 交互与检测 ---

void ItemManager::checkPlayerPickUp(Player* player)
{
    if (!player) return;

    // 遍历道具池 (使用迭代器以支持安全删除)
    for (auto it = items.begin(); it != items.end(); )
    {
        Item* item = *it;
        if (!item) { ++it; continue; }

        // 碰撞检测：判断玩家与道具矩形是否相交
        if (player->getBoundingBox().intersectsRect(item->getBoundingBox()))
        {
            // 触发拾取逻辑 (Player类处理属性加成)
            player->pickItem(item);

            // 移除道具节点并从队列删除
            item->removeFromParent();
            it = items.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// --- 查询接口 ---

bool ItemManager::hasItemAtGrid(const Vec2& grid) const
{
    // 检查指定格子上是否已有道具 (防止重复生成堆叠)
    for (auto item : items)
    {
        if (!item) continue;
        Vec2 g = _mapLayer->worldToGrid(item->getPosition());
        if (g == grid) return true;
    }
    return false;
}