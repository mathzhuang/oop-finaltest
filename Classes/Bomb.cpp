#include "Bomb.h"
#include "GameScene.h" 
#include "MapLayer.h"
#include "Flame.h"
#include "Item.h"
#include "cocos2d.h"
#include "ItemManager.h"
#include "AudioEngine.h"
#include "Player.h"

USING_NS_CC;
using namespace cocos2d::experimental;

// --- 工厂与生命周期 ---

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

// --- 对外接口 ---

void Bomb::startCountdown(const std::function<void()>& onExplode)
{
    // 原始逻辑：仅使用 Action 进行延时回调
    this->runAction(Sequence::create(
        DelayTime::create(2.0f),
        CallFunc::create([=]() {
            this->explode();
            if (onExplode) onExplode();
            }),
        nullptr
    ));
}

bool Bomb::willExplodeGrid(const Vec2& targetGrid) const
{
    // 利用绝对值判断十字范围
    return (targetGrid.x == gridPos.x && std::abs(targetGrid.y - gridPos.y) <= range) ||
        (targetGrid.y == gridPos.y && std::abs(targetGrid.x - gridPos.x) <= range);
}

// --- 内部逻辑 ---

void Bomb::explode()
{
    auto parent = this->getParent();
    if (!parent) return;

    // 获取 MapLayer
    MapLayer* map = nullptr;
    for (auto child : parent->getChildren()) {
        map = dynamic_cast<MapLayer*>(child);
        if (map) break;
    }
    if (!map) return;

    // 播放爆炸音效
    if (GameScene::s_isAudioOn) {
        AudioEngine::play2d("Sound/bomb.mp3", false, 1.0f);
    }

    Vec2 grid = map->worldToGrid(this->getPosition());
    int gx = (int)grid.x;
    int gy = (int)grid.y;

    // 1. 中心火焰
    createFlameAt(gx, gy, map, 15);

    const Vec2 dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    // 2. 向四周扩散
    for (int d = 0; d < 4; d++)
    {
        for (int i = 1; i <= range; i++)
        {
            int nx = gx + dirs[d].x * i;
            int ny = gy + dirs[d].y * i;

            int tile = map->getTile(nx, ny);

            // 遇到铁墙：火焰完全阻断
            if (tile == MapLayer::TILE_IRON_WALL) break;

            // 生成火焰并标记逻辑位
            createFlameAt(nx, ny, map, 15);

            // 遇到软墙：销毁墙体、触发掉落并阻断火焰
            if (tile == MapLayer::TILE_SOFT_WALL)
            {
                map->setTile(nx, ny, 0);
                map->removeWallAt(nx, ny); // 内部应包含 setTile(nx, ny, 0)

                if (_owner) {
                    _owner->addScore(5);
                }

                auto scene = dynamic_cast<GameScene*>(parent);
                if (scene && scene->getItemManager()) {
                    scene->getItemManager()->dropItem(nx, ny, 35);
                }
                break; // 阻断后续火焰
            }
        }
    }

    this->removeFromParent();
    CCLOG("Bomb exploded at grid (%d,%d)", gx, gy);
}

void Bomb::createFlameAt(int gx, int gy, MapLayer* map, int zOrder)
{
    if (!map) return;

    auto flame = Flame::createFlame();
    if (!flame) return;

    flame->gridPos = Vec2(gx, gy);
    flame->setOwner(_owner); // 传递主人

    // 设置渲染位置
    Vec2 worldPos = map->gridToWorld(gx, gy);
    Vec2 localPos = map->convertToNodeSpace(worldPos);
    flame->setPosition(localPos);
    flame->setTag(MapLayer::TILE_FLAME);

    // 同步标记地图逻辑数据
    map->setTile(gx, gy, MapLayer::TILE_FLAME);

    map->addChild(flame, zOrder);

    // 延时动作：火焰消失的同时清理地图逻辑数据
    flame->runAction(Sequence::create(
        DelayTime::create(0.25f),
        CallFunc::create([map, gx, gy]() {
            // 只有当格子当前还是火焰时才清理（防止覆盖新放的炸弹/物体）
            if (map->getTile(gx, gy) == MapLayer::TILE_FLAME) {
                map->setTile(gx, gy, MapLayer::TILE_EMPTY);
            }
            }),
        RemoveSelf::create(),
        nullptr
    ));
}