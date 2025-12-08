#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"

USING_NS_CC;

Player* Player::createPlayer()
{
    Player* p = new (std::nothrow) Player();
    if (p && p->initWithFile("player_black(1).png"))
    {
        p->setScale(2.0f);
        p->autorelease();
        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}

// 地图碰撞移动
void Player::move(const Vec2& dir, MapLayer* mapLayer)
{
    if (!mapLayer) return;

    float speed = 120.0f;
    float dt = Director::getInstance()->getDeltaTime();
    Vec2 newPos = this->getPosition() + dir * speed * dt;

    Vec2 grid = mapLayer->worldToGrid(newPos);
    int gx = (int)grid.x;
    int gy = (int)grid.y;

    if (!mapLayer->isWalkable(gx, gy))
        return;

    this->setPosition(newPos);
}

// 放炸弹
void Player::placeBomb()
{
    auto parent = this->getParent();
    if (!parent) return;

    // 创建真正的炸弹对象
    Bomb* bomb = Bomb::createBomb();
    if (!bomb) return;

    bomb->setScale(2.0f);  // 建议别太大

    // 先放在玩家脚下
    bomb->setPosition(this->getPosition());

    // 找 map
    MapLayer* map = nullptr;
    for (auto child : parent->getChildren())
    {
        map = dynamic_cast<MapLayer*>(child);
        if (map) break;
    }

    // 对齐网格
    if (map)
    {
        Vec2 grid = map->worldToGrid(this->getPosition());
        Vec2 world = map->gridToWorld((int)grid.x, (int)grid.y);
        bomb->setPosition(world);
    }

    // 只 addChild 一次！！
    parent->addChild(bomb);

    // 启动倒计时
    bomb->startCountdown();
}