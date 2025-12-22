#include "Bomb.h"
#include "GameScene.h" // 如果有需要
#include "MapLayer.h"
#include "Flame.h"
#include "Item.h"
#include "cocos2d.h"
#include "ItemManager.h"


USING_NS_CC;

Bomb* Bomb::createBomb(int range)
{
    Bomb* b = new (std::nothrow) Bomb();
    if (b && b->initWithFile("bomb(1).png"))
    {
        b->autorelease();
        b->range = range;
        return b;
    }
    CC_SAFE_DELETE(b);
    return nullptr;
}

void Bomb::startCountdown(const std::function<void()>& onExplode)
{
    this->runAction(Sequence::create(
        DelayTime::create(2.0f),
        CallFunc::create([=]() {
            this->explode();

            // 玩家回调
            if (onExplode) onExplode();
            }),
        nullptr
    ));
}

// -------------------------
// createFlameAt 重载，增加 z 参数
// -------------------------
void Bomb::createFlameAt(int gx, int gy, MapLayer* map, int zOrder = 15)
{
    if (!map) return;

    auto flame = Flame::createFlame();
    if (!flame) return;

    flame->gridPos = Vec2(gx, gy);
    // ✅ 将世界坐标转换到 MapLayer 局部坐标
    Vec2 worldPos = map->gridToWorld(gx, gy);
    Vec2 localPos = map->convertToNodeSpace(worldPos);
    flame->setPosition(localPos);

    flame->setScale(2.5f);
    flame->setTag(300);

    map->addChild(flame, zOrder);
    flame->runAction(Sequence::create(
        DelayTime::create(0.25f),
        RemoveSelf::create(),
        nullptr
    ));
}


void Bomb::explode()
{
    auto parent = this->getParent();
    if (!parent) return;

    // 找到 MapLayer
    MapLayer* map = nullptr;
    for (auto child : parent->getChildren())
    {
        map = dynamic_cast<MapLayer*>(child);
        if (map) break;
    }
    if (!map) return;

    Vec2 grid = map->worldToGrid(this->getPosition());
    int gx = (int)grid.x;
    int gy = (int)grid.y;

    // 中心火焰
    createFlameAt(gx, gy, map, 15);

    const Vec2 dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };

    for (int d = 0; d < 4; d++)
    {
        int dx = dirs[d].x;
        int dy = dirs[d].y;

        for (int i = 1; i <= range; i++)
        {
            int nx = gx + dx * i;
            int ny = gy + dy * i;

            int tile = map->getTile(nx, ny);

            // 遇到铁墙，火焰不能穿过
            if (tile == 1) break;

            // 创建火焰
            createFlameAt(nx, ny, map, 15);

            // 遇到软墙
            if (tile == 2) // 木箱/软墙
            {
                map->setTile(nx, ny, 0);
                map->removeWallAt(nx, ny);

                // 通过 ItemManager 掉落
                auto scene = dynamic_cast<GameScene*>(this->getParent());
                if (scene && scene->getItemManager())
                    scene->getItemManager()->dropItem(nx, ny, 35);

                break; // 阻挡火焰传播
            }
        }
    }

    // 移除炸弹自身
    this->removeFromParent();

    CCLOG("Bomb exploded at grid (%d,%d), world pos (%f,%f)", gx, gy, this->getPosition().x, this->getPosition().y);
}


bool Bomb::willExplodeGrid(const Vec2& targetGrid) const
{
    // 同行或同列，且在威力范围内
    if (targetGrid.x == gridPos.x && abs(targetGrid.y - gridPos.y) <= range)
        return true;
    if (targetGrid.y == gridPos.y && abs(targetGrid.x - gridPos.x) <= range)
        return true;
    return false;
}

