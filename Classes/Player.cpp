#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Item.h" 
#include"GameScene.h"


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
    Player* p = new (std::nothrow) Player();
    if (p && p->init())
    {
        p->autorelease();

        // ===============================
        // ⭐ 强制初始化所有“生死相关状态”
        // ===============================
        p->isDead = false;
        p->isAI = false;          // 人类玩家默认 false
        p->hp = 3;
        p->maxHp = 5;
        p->invincible = false;
        p->hasShield = false;

        // 炸弹相关
        p->currentBombCount = 0;
        p->canPlaceBomb = true;

        // 移动相关
        p->isMoving = false;
        p->moveSpeed = p->defaultMoveSpeed;

        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}



// ==================================================
// 玩家移动：走位 + 碰撞检测 + 贴墙滑动
// ==================================================
void Player::move(const Vec2& dir, MapLayer* mapLayer)
{
    if (!mapLayer || isDead) return;
    if (stunned) return;   // ⭐ 被 Block，禁止移动
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
//新接口
bool Player::tryMoveTo(const Vec2& nextGrid, MapLayer* map)
{
    if (isDead) { CCLOG("AI can't move: dead"); return false; }
    if (stunned) { CCLOG("AI can't move: stunned"); return false; }
    if (isMoving) { CCLOG("AI can't move: isMoving=true"); return false; }
    if (!map) { CCLOG("AI can't move: map=NULL"); return false; }

    CCLOG("AI moving from (%f,%f) to (%f,%f)", currentGrid.x, currentGrid.y, nextGrid.x, nextGrid.y);
    if (!map->isWalkable(nextGrid.x, nextGrid.y))
        return false;

    currentGrid = map->worldToGrid(this->getPosition());
    targetGrid = nextGrid;
    isMoving = true;

    Vec2 worldPos = map->gridToWorld(targetGrid.x, targetGrid.y);

    auto move = MoveTo::create(0.15f, worldPos);
    auto done = CallFunc::create([this]() {
        isMoving = false;
        });

    this->runAction(Sequence::create(move, done, nullptr));
    return true;
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

    switch (item->getType())
    {
    case Item::ItemType::PowerBomb: increaseBombRange(); break;
    case Item::ItemType::Heal: heal(); break;
    case Item::ItemType::Shield: activateShield(5.0f); break;
    case Item::ItemType::Block: blockOpponent(); break;
    case Item::ItemType::SpeedUp: speedUp(3.0f, 1.5f); break;
    }

    // ✅ 视觉效果仍然由 Item 播放
    item->playPickAnimationEffect(this);
}

// -------------------- 道具效果接口 --------------------
void Player::increaseBombRange()
{
    bombRange+=2;
    CCLOG("PowerBomb! New range = %d", bombRange);
}

void Player::heal()
{
    if (hp < maxHp) hp++;
    CCLOG("Heal! HP = %d", hp);
}

void Player::activateShield(float duration)
{
    hasShield = true;
    invincible = true;
    CCLOG("Shield ON!");

    this->runAction(Sequence::create(
        DelayTime::create(duration),
        CallFunc::create([this]() {
            hasShield = false;
            invincible = false;
            CCLOG("Shield OFF");
            }),
        nullptr
    ));
}

void Player::speedUp(float duration, float factor)
{
    moveSpeed = defaultMoveSpeed * factor;
    this->runAction(Sequence::create(
        DelayTime::create(duration),
        CallFunc::create([this]() {
            moveSpeed = defaultMoveSpeed;
            CCLOG("Speed OFF");
            }),
        nullptr
    ));
}

void Player::blockOpponent()
{
    if (!_scene) return;

    Player* target = nullptr;
    float minDist = FLT_MAX;

    // 找最近的“不是自己”的玩家
    for (auto p : _scene->getPlayers())
    {
        if (!p || p == this || p->isDead) continue;

        float d = this->getPosition().distance(p->getPosition());
        if (d < minDist)
        {
            minDist = d;
            target = p;
        }
    }

    if (!target) return;

    // ⭐ 定身
    target->stunned = true;
    CCLOG("Block! Target stunned");

    // ⭐ 3 秒后解除
    target->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([target]() {
            target->stunned = false;
            CCLOG("Block end");
            }),
        nullptr
    ));
}

Player* Player::findNearestEnemy(float minValidDist)
{
    if (!_scene) return nullptr;

    Player* target = nullptr;
    float minDist = FLT_MAX;

    for (auto p : _scene->getPlayers())
    {
        if (!p) continue;
        if (p == this) continue;
        if (p->isDead) continue;

        // ⭐ 只锁敌人（你现在最安全）
        if (p->isAI == this->isAI) continue;

        float d = this->getPosition().distance(p->getPosition());

        // ⭐ 过滤重叠/同格
        if (d < minValidDist) continue;

        if (d < minDist)
        {
            minDist = d;
            target = p;
        }
    }

    return target;
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
    // z=6，比道具高，低于玩家
    scene->addChild(bomb, 6);

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
bool Player::init()
{
    if (!Sprite::init())
        return false;

    // ===== 生命状态 =====
    isDead = false;
    invincible = false;
    hasShield = false;

    hp = maxHp;

    // ===== AI / 控制 =====
    isAI = false;
    isMoving = false;

    // ===== 炸弹 =====
    canPlaceBomb = true;
    currentBombCount = 0;

    return true;
}

void Player::die()
{
    if (isDead) return;
    isDead = true;

    this->stopAllActions();
    this->setColor(Color3B::RED);

    // 死亡动画（保留）
    this->runAction(Sequence::create(
        DelayTime::create(0.8f),
        CallFunc::create([this]() {
            this->setVisible(false); // 或 removeFromParent()
        }),
        nullptr
    ));
}
