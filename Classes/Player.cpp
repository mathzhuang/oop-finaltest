#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"

USING_NS_CC;

Player* Player::createPlayer()
{
    Player* p = new (std::nothrow) Player();
    if (p && p->initWithFile("player_black(1).png"))
    {  
        //人物缩放(后续根据map调整）
        p->setScale(2.0);
        p->autorelease();
        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}

// 地图碰撞的移动函数：dir 已规范（例如 Vec2(1,0) 表示向右）
void Player::move(const Vec2& dir, MapLayer* mapLayer)
{
    if (!mapLayer) return;

    // 速度（像素 / 秒）
    float speed = 120.0f;
    float dt = Director::getInstance()->getDeltaTime();

    Vec2 newPos = this->getPosition() + dir * speed * dt;

    // 世界坐标 -> 网格坐标（使用 MapLayer 的函数）
    Vec2 grid = mapLayer->worldToGrid(newPos);
    int gx = (int)grid.x;
    int gy = (int)grid.y;

    // 如果不可走就不移动
    if (!mapLayer->isWalkable(gx, gy))
        return;

    this->setPosition(newPos);
}

// 兼容旧接口（单步移动）
void Player::moveUp() { this->setPositionY(this->getPositionY() + 5); }
void Player::moveDown() { this->setPositionY(this->getPositionY() - 5); }
void Player::moveLeft() { this->setPositionX(this->getPositionX() - 5); }
void Player::moveRight() { this->setPositionX(this->getPositionX() + 5); }

// 放炸弹：使用 Bomb 类，更易扩展（range, owner 等）
void Player::placeBomb()
{
    auto bomb = Sprite::create("bomb.png");
    bomb->setScale(3.0);
    bomb->setPosition(this->getPosition());
    this->getParent()->addChild(bomb);

    // 把炸弹放到网格中心
    // 先计算玩家所在格子，然后把炸弹位置对齐到格子中心
    auto parent = this->getParent();
    MapLayer* map = nullptr;
    // 在场景结构里，Player 的 parent 通常是 GameScene，GameScene 中有 MapLayer
    // 简单方式：在父节点查找 MapLayer（假设只有一个）
    for (auto child : parent->getChildren())
    {
        map = dynamic_cast<MapLayer*>(child);
        if (map) break;
    }

    if (map)
    {
        Vec2 grid = map->worldToGrid(this->getPosition());
        Vec2 world = map->gridToWorld((int)grid.x, (int)grid.y);
        b->setPosition(world);
    }
    else
    {
        b->setPosition(this->getPosition());
    }

    parent->addChild(b);
    b->startCountdown();
}
