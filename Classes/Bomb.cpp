#include "Bomb.h"
#include "MapLayer.h"
#include "GameScene.h"

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

void Bomb::startCountdown()
{
    this->runAction(Sequence::create(
        DelayTime::create(2.0f),
        CallFunc::create([=]() { this->explode(); }),
        nullptr
    ));
}

// 新的 createFlameAt，加入 GameScene 管理
void Bomb::createFlameAt(int gx, int gy, MapLayer* map, Node* parent, GameScene* scene)
{
    if (!map || !parent) return;

    Vec2 pos = map->gridToWorld(gx, gy);

    auto flame = Sprite::create("explosion(1).png");
    if (!flame) return;

    flame->setPosition(pos);
    flame->setScale(2.5f);
    flame->setTag(300);

    parent->addChild(flame, 5);

    // 加入火焰列表
    if (scene)
        scene->flames.push_back(flame);

    // 火焰消失后，从列表中删除
    flame->runAction(Sequence::create(
        DelayTime::create(0.25f),
        CallFunc::create([flame, scene]() {
            if (scene)
            {
                auto& fList = scene->flames;
                fList.erase(std::remove(fList.begin(), fList.end(), flame), fList.end());
            }
            flame->removeFromParent();
            }),
        nullptr
    ));
}

void Bomb::explode()
{
    auto parent = this->getParent();
    if (!parent) return;

    MapLayer* map = nullptr;
    GameScene* scene = nullptr;

    // 找 MapLayer 和 GameScene
    for (auto child : parent->getChildren())
    {
        if (!map)
            map = dynamic_cast<MapLayer*>(child);
        if (!scene)
            scene = dynamic_cast<GameScene*>(child);

        if (map && scene) break;
    }

    if (!map) return;

    Vec2 grid = map->worldToGrid(this->getPosition());
    int gx = (int)grid.x;
    int gy = (int)grid.y;

    // 中心
    createFlameAt(gx, gy, map, parent, scene);

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

            if (tile == 1) break; // 墙阻挡
            createFlameAt(nx, ny, map, parent, scene);

            if (tile == 2)
            {
                map->setTile(nx, ny, 0); // 砖块炸掉
                break;
            }
        }
    }

    this->removeFromParent();
}
