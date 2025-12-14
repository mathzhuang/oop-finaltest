#include "Bomb.h"
#include "GameScene.h" // 如果有需要
#include "MapLayer.h"
#include "Flame.h"
#include "Item.h"
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


void Bomb::createFlameAt(int gx, int gy, MapLayer* map, Node* parent)
{
    if (!map || !parent) return;

    Vec2 pos = map->gridToWorld(gx, gy);

    auto flame = Flame::createFlame();
    flame->gridPos = Vec2(gx, gy);

    if (!flame) return;

    flame->setPosition(pos);
    flame->setScale(2.5f);
    flame->setTag(300);

    parent->addChild(flame, 5);

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
    createFlameAt(gx, gy, map, parent);

    // 四个方向
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

            // 遇到墙体（1 是不可破坏墙）
            if (tile == 1)
                break;

            // 创建火焰
            createFlameAt(nx, ny, map, parent);

            // 遇到木箱（2 是可破坏箱子）
            if (tile == 2)
            {
                // 删除木箱
                map->setTile(nx, ny, 0);

                // ==============================
                // ⭐⭐⭐ 这里加：爆箱掉落道具 ⭐⭐⭐
                // ==============================
                float dropRate = 0.35f;   // 35% 掉落率，可改

                if (CCRANDOM_0_1() < dropRate)
                {
                    // 随机一个道具类型 (0~5)
                    int r = cocos2d::RandomHelper::random_int(0, 5);

                    // 创建道具
                    Item* item = Item::createItem(static_cast<Item::ItemType>(r));

                    if (item)
                    {
                        // 对齐到网格
                        Vec2 wpos = map->gridToWorld(nx, ny);
                        item->setPosition(wpos);

                        parent->addChild(item, 5);

                        // 出生动画
                        item->playSpawnAnimation();
                    }
                }
                // ==============================

                break; // 箱子挡住火焰传播
            }
        }
    }

    this->removeFromParent();
}
