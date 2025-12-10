#include "Bomb.h"
#include "GameScene.h" // 如果有需要
#include "MapLayer.h"
#include "Flame.h"
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

    // 中心
    createFlameAt(gx, gy, map, parent);

    // 四方向
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

            if (tile == 1) break;
            createFlameAt(nx, ny, map, parent);

            if (tile == 2)
            {
                map->setTile(nx, ny, 0);
                break;
            }
        }
    }

    this->removeFromParent();
}