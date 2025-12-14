#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Item.h" 


USING_NS_CC;
// 设置角色显示
void Player::setCharacter(int characterId)
{
    _characterId = characterId;

    // 加载对应角色精灵纹理
    std::string filename = StringUtils::format("player/player%d.png", characterId);
    if (!this->initWithFile(filename))
    {
        CCLOG("Error: Failed to load character sprite %s", filename.c_str());
    }
}
// ==================================================
// 创建玩家
// ==================================================
Player* Player::createPlayer()
{
    Player* ret = new Player();
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


// ==================================================
// 玩家移动：走位 + 碰撞检测 + 贴墙滑动
// ==================================================
void Player::move(const Vec2& dir, MapLayer* mapLayer)
{
    if (!mapLayer || isDead) return;

    float dt = Director::getInstance()->getDeltaTime();
    Vec2 newPos = this->getPosition() + dir * moveSpeed * dt;

    // 主碰撞判断
    if (canMoveTo(newPos, mapLayer))
    {
        this->setPosition(newPos);
        return;
    }

    // -------- 贴墙滑动机制 --------
    if (dir.x != 0)     // 横向移动 → 尝试上下微调
    {
        Vec2 up = newPos + Vec2(0, 8);
        if (canMoveTo(up, mapLayer)) { this->setPosition(up); return; }

        Vec2 down = newPos + Vec2(0, -8);
        if (canMoveTo(down, mapLayer)) { this->setPosition(down); return; }
    }

    if (dir.y != 0)     // 纵向移动 → 尝试左右微调
    {
        Vec2 left = newPos + Vec2(-8, 0);
        if (canMoveTo(left, mapLayer)) { this->setPosition(left); return; }

        Vec2 right = newPos + Vec2(8, 0);
        if (canMoveTo(right, mapLayer)) { this->setPosition(right); return; }
    }
}

// ==================================================
// 判断是否能走到某个位置
// ==================================================
bool Player::canMoveTo(const Vec2& newPos, MapLayer* mapLayer)
{
    Vec2 grid = mapLayer->worldToGrid(newPos);
    return mapLayer->isWalkable(grid.x, grid.y);
}
// ==================================================
// 捡道具判断
// ==================================================
void Player::pickItem(Item* item)
{
    if (!item) return;

    Item::ItemType type = item->getType();

    switch (type)
    {
    case Item::ItemType::PowerBomb:
        bombRange++;
        CCLOG("PowerBomb! New range = %d", bombRange);
        break;

    case Item::ItemType::Heal:
        if (hp < maxHp) hp++;
        CCLOG("Heal! HP = %d", hp);
        break;

    case Item::ItemType::Shield:
        hasShield = true;
        invincible = true;
        CCLOG("Shield ON!");
        // 5 秒后自动失效
        this->runAction(Sequence::create(
            DelayTime::create(5.0f),
            CallFunc::create([this]() {
                hasShield = false;
                invincible = false;
                CCLOG("Shield OFF");
                }),
            nullptr
        ));
        break;

    case Item::ItemType::Block:
        // 自己不用处理，对敌人生效（由 GameScene 或 PlayerManager 处理）
        CCLOG("Picked Block item — send event to block opponent");
        break;

    case Item::ItemType::SpeedUp:
        moveSpeed = defaultMoveSpeed * 1.5f;
        speedBoostTimer = 3.0f;     // 持续 3 秒

        this->runAction(Sequence::create(
            DelayTime::create(3.0f),
            CallFunc::create([this]() {
                moveSpeed = defaultMoveSpeed;
                CCLOG("SpeedOff");
                }),
            nullptr
        ));

        CCLOG("Speed UP! moveSpeed = %f", moveSpeed);
        break;

   // case Item::ItemType::RandomBox:
   // {
        // 随机触发任意效果
     //   int r = cocos2d::RandomHelper::random_int(0, 4);
       // CCLOG("RandomBox roll: %d", r);
        //Item::ItemType real = static_cast<Item::ItemType>(r);
  //      pickItem(new Item(real));  // 递归调用一次
    //    break;
   // }
    }

    // 移除道具
    item->removeFromParent();
}



// ==================================================
// 放置炸弹：冷却 + 数量限制 + 检查重复炸弹
// ==================================================
void Player::placeBomb(Node* scene, MapLayer* mapLayer)
{
    if (!scene || !mapLayer || isDead) return;
    if (!canPlaceBomb) return;
    if (currentBombCount >= maxBombCount) return;

    // 玩家所在格子
    Vec2 grid = mapLayer->worldToGrid(this->getPosition());

    // 检查该格是否已有炸弹
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

    // 炸弹爆炸时 —> 清除数量
    bomb->startCountdown([this]() {
        currentBombCount--;
        });

    resetBombCooldown();
}

// ==================================================
// 炸弹冷却计时
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
// 玩家受伤（扣血 + 无敌闪烁）
// ==================================================
void Player::takeDamage()
{
    if (invincible || isDead) return;

    hp--;
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
// 死亡动画 + 切换新场景
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