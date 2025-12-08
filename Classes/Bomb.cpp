#include "Bomb.h"
#include "MapLayer.h"
USING_NS_CC;

Bomb* Bomb::createBomb(int range)
{
    Bomb* b = new (std::nothrow) Bomb();
    if (b && b->initWithFile("bomb(1).png"))   // 请把 bomb.png 放到 Resources
    {
        b->autorelease();
        b->range = range;
        return b;
    }
    CC_SAFE_DELETE(b);
    return nullptr;
}

void Bomb::startCountdown()
{
    // 2 秒后爆炸
    this->runAction(
        Sequence::create(
            DelayTime::create(2.0f),
            CallFunc::create([=]() { this->explode(); }),
            nullptr
        )
    );
}
void Bomb::createFlameAt(int gx, int gy, MapLayer* map, Node* parent)
{
    // 网格 → 世界坐标
    Vec2 pos = map->gridToWorld(gx, gy);

    auto flame = Sprite::create("flame.png");
    flame->setPosition(pos);
    flame->setScale(2.5f);
    flame->setTag(300); // Flame tag

    parent->addChild(flame, 5);

    // 0.25 秒后自动消失
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

    // 找 MapLayer
    MapLayer* map = nullptr;
    for (auto child : parent->getChildren())
    {
        map = dynamic_cast<MapLayer*>(child);
        if (map) break;
    }
    if (!map) return;

    // 1. 计算炸弹所在格子
    Vec2 grid = map->worldToGrid(this->getPosition());
    int gx = grid.x;
    int gy = grid.y;

    // 2. 爆中心
    createFlameAt(gx, gy, map, parent);

    // 3. 四个方向延伸
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

            // 遇到墙（不可破坏）
            if (tile == 1)
                break;

            // 可走或软砖都要显示火焰
            createFlameAt(nx, ny, map, parent);

            // 遇到软砖（可破坏）→ 删除砖块并停止继续延伸
            if (tile == 2)
            {
                map->setTile(nx, ny, 0);
                break;
            }
        }
    }

    // 最终把炸弹自身删掉
    this->removeFromParent();
}
