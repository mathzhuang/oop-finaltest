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
// ==================================================
// 更智能的移动：支持贴墙滑动，不容易卡角
// ==================================================
void Player::move(const Vec2& dir, MapLayer* mapLayer)
{
    if (!mapLayer || isDead) return;

    float dt = Director::getInstance()->getDeltaTime();
    Vec2 newPos = this->getPosition() + dir * moveSpeed * dt;

    // ==== 核心：判断能不能走 ====
    if (canMoveTo(newPos, mapLayer))
    {
        this->setPosition(newPos);
        return;
    }

    // ===【贴墙滑动】===
    // 例如向右走但被卡，尝试上下微调
    if (dir.x != 0)
    {
        Vec2 up = newPos + Vec2(0, 8);
        if (canMoveTo(up, mapLayer)) { this->setPosition(up); return; }

        Vec2 down = newPos + Vec2(0, -8);
        if (canMoveTo(down, mapLayer)) { this->setPosition(down); return; }
    }

    if (dir.y != 0)
    {
        Vec2 left = newPos + Vec2(-8, 0);
        if (canMoveTo(left, mapLayer)) { this->setPosition(left); return; }

        Vec2 right = newPos + Vec2(8, 0);
        if (canMoveTo(right, mapLayer)) { this->setPosition(right); return; }
    }
}

// ==================================================
// 判断位置是否合法
// ==================================================
bool Player::canMoveTo(const Vec2& newPos, MapLayer* mapLayer)
{
    Vec2 grid = mapLayer->worldToGrid(newPos);
    return mapLayer->isWalkable(grid.x, grid.y);
}


// ==================================================
// 放炸弹：加入冷却 & 数量限制
// ==================================================
void Player::placeBomb(Node* scene, MapLayer* mapLayer)
{
    if (!scene || !mapLayer || isDead) return;
    if (!canPlaceBomb) return;
    if (currentBombCount >= maxBombCount) return;

    Vec2 grid = mapLayer->worldToGrid(this->getPosition());

    // 检查该位置是否已有炸弹
    for (auto child : scene->getChildren())
    {
        Bomb* b = dynamic_cast<Bomb*>(child);
        if (!b) continue;

        Vec2 bGrid = mapLayer->worldToGrid(b->getPosition());
        if (bGrid == grid) return;
    }

    // 创建炸弹
    Bomb* bomb = Bomb::createBomb();
    if (!bomb) return;

    bomb->setScale(2.0f);
    bomb->setPosition(mapLayer->gridToWorld(grid.x, grid.y));
    scene->addChild(bomb);

    currentBombCount++;
    canPlaceBomb = false;

    bomb->startCountdown([this]() {
        currentBombCount--;
        });

    resetBombCooldown();
}

// ==================================================
// 炸弹放置冷却
// ==================================================
void Player::resetBombCooldown()
{
    this->runAction(Sequence::create(
        DelayTime::create(bombCooldown),
        CallFunc::create([this]() {
            canPlaceBomb = true;
            }),
        nullptr
    ));
}

// ==================================================
// 受伤 + 无敌闪烁
// ==================================================
void Player::takeDamage()
{
    if (invincible || isDead) return;

    hp -= 1;
    CCLOG("Player HP = %d", hp);

    invincible = true;
    this->setOpacity(150);

    // 解除无敌
    this->runAction(Sequence::create(
        DelayTime::create(1.0f),
        CallFunc::create([this]() {
            invincible = false;
            this->setOpacity(255);
            }),
        nullptr
    ));

    if (hp <= 0)
        die();
}

// ==================================================
// 死亡
// ==================================================
void Player::die()
{
    if (isDead) return;
    isDead = true;

    this->stopAllActions();
    this->setColor(Color3B::RED);

    this->runAction(Sequence::create(
        DelayTime::create(0.8f),
        CallFunc::create([]() {
            Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, Scene::create())
            );
            }),
        nullptr
    ));
}