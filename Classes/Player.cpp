#include "Player.h"
#include "MapLayer.h"
#include "Bomb.h"
#include "Item.h" 
#include "GameScene.h"
#include "AudioEngine.h"
#include "GameBackground.h"

USING_NS_CC;
using namespace cocos2d::experimental;

// --- 工厂与生命周期 ---

Player* Player::createPlayer()
{
    Player* p = new (std::nothrow) Player();
    if (p && p->init())
    {
        p->autorelease();

        // 初始化游戏性属性
        p->isDead = false;
        p->isAI = false;
        p->hp = 3;
        p->maxHp = 5;
        p->invincible = false;
        p->hasShield = false;

        // 炸弹属性
        p->currentBombCount = 0;
        p->canPlaceBomb = true;

        // 移动属性
        p->isMoving = false;
        p->moveSpeed = p->defaultMoveSpeed;

        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}

bool Player::init()
{
    if (!Sprite::init()) return false;

    // 基础状态初始化
    isDead = false;
    invincible = false;
    hasShield = false;
    hp = maxHp;

    isAI = false;
    isMoving = false;

    canPlaceBomb = true;
    currentBombCount = 0;

    return true;
}

void Player::onExit()
{
    // 强制停止音效
    if (_walkAudioID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(_walkAudioID);
        _walkAudioID = AudioEngine::INVALID_AUDIO_ID;
    }
    Sprite::onExit();
}

void Player::setCharacter(int characterId)
{
    _characterId = characterId;
    std::string filename = StringUtils::format("player/player%d.png", characterId);
    if (!this->initWithFile(filename)) {
        CCLOG("Error: Failed to load character sprite %s", filename.c_str());
    }
}

// --- 移动与动画逻辑 ---

void Player::move(const Vec2& dir, MapLayer* mapLayer)
{
    if (!mapLayer || isDead || stunned) {
        stopWalkAnimation();
        return;
    }

    // 1. 音效管理
    if (dir.length() > 0 && GameScene::s_isAudioOn) {
        if (_walkAudioID == AudioEngine::INVALID_AUDIO_ID) {
            _walkAudioID = AudioEngine::play2d("Sound/stepsSound.mp3", true, 5.0f);
        }
    }
    else {
        stopWalkAnimation(); // 内含停止音效逻辑
    }

    // 2. 动画管理
    if (dir.length() > 0) {
        updateWalkAnimation(dir);
    }
    else {
        stopWalkAnimation();
    }

    // 3. 位置更新与碰撞检测
    float dt = Director::getInstance()->getDeltaTime();
    Vec2 newPos = this->getPosition() + dir * moveSpeed * dt;

    // A. 直接通行
    if (canMoveTo(newPos, mapLayer)) {
        this->setPosition(newPos);
        return;
    }

    // B. 贴墙滑动 (辅助修正)
    if (dir.x != 0) { // 横向受阻，尝试上下微调
        Vec2 up = newPos + Vec2(0, 8);
        if (canMoveTo(up, mapLayer)) { this->setPosition(up); return; }

        Vec2 down = newPos + Vec2(0, -8);
        if (canMoveTo(down, mapLayer)) { this->setPosition(down); return; }
    }
    if (dir.y != 0) { // 纵向受阻，尝试左右微调
        Vec2 left = newPos + Vec2(-8, 0);
        if (canMoveTo(left, mapLayer)) { this->setPosition(left); return; }

        Vec2 right = newPos + Vec2(8, 0);
        if (canMoveTo(right, mapLayer)) { this->setPosition(right); return; }
    }
}

bool Player::tryMoveTo(const Vec2& nextGrid, MapLayer* map)
{
    if (isDead || stunned || isMoving || !map) return false;

    // 检查目标是否可行走
    if (!map->isWalkable(nextGrid.x, nextGrid.y)) return false;

    currentGrid = map->worldToGrid(this->getPosition());
    targetGrid = nextGrid;
    isMoving = true;

    Vec2 worldPos = map->gridToWorld(targetGrid.x, targetGrid.y);
    Vec2 moveDir = worldPos - this->getPosition();
    moveDir.normalize();

    // 播放动画并执行移动动作
    updateWalkAnimation(moveDir);

    auto move = MoveTo::create(0.15f, worldPos);
    auto done = CallFunc::create([this]() {
        isMoving = false;
        stopWalkAnimation();
        });

    this->runAction(Sequence::create(move, done, nullptr));
    return true;
}

bool Player::canMoveTo(const Vec2& newPos, MapLayer* mapLayer)
{
    Vec2 grid = mapLayer->worldToGrid(newPos);
    return mapLayer->isWalkable(grid.x, grid.y);
}

void Player::updateWalkAnimation(const Vec2& dir)
{
    // 确定朝向
    Direction newDir = Direction::None;
    if (std::abs(dir.x) > std::abs(dir.y)) {
        newDir = (dir.x > 0) ? Direction::Right : Direction::Left;
    }
    else {
        newDir = (dir.y > 0) ? Direction::Up : Direction::Down;
    }

    // 避免重复设置相同动画
    if (newDir == _currentDirection && this->getActionByTag(ACTION_TAG_WALK)) {
        return;
    }

    // 切换动画
    this->stopActionByTag(ACTION_TAG_WALK);
    _currentDirection = newDir;

    Action* walkAction = createWalkAction(newDir);
    if (walkAction) {
        walkAction->setTag(ACTION_TAG_WALK);
        this->runAction(walkAction);
    }
}

void Player::stopWalkAnimation()
{
    if (_currentDirection == Direction::None) return;

    this->stopActionByTag(ACTION_TAG_WALK);
    _currentDirection = Direction::None;

    if (_walkAudioID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(_walkAudioID);
        _walkAudioID = AudioEngine::INVALID_AUDIO_ID;
    }
}

cocos2d::Action* Player::createWalkAction(Direction dir)
{
    std::string dirStr = "";
    switch (dir) {
    case Direction::Up:    dirStr = "up"; break;
    case Direction::Down:  dirStr = "down"; break;
    case Direction::Left:  dirStr = "left"; break;
    case Direction::Right: dirStr = "right"; break;
    default: return nullptr;
    }

    // 加载动画帧 (假设3帧)
    int frameCount = 3;
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= frameCount; ++i) {
        std::string filename = StringUtils::format("player/player%d_%s_%d.png", _characterId, dirStr.c_str(), i);
        auto sprite = Sprite::create(filename);
        if (sprite) {
            frames.pushBack(sprite->getSpriteFrame());
        }
        else {
            SpriteFrame* sf = SpriteFrameCache::getInstance()->getSpriteFrameByName(filename);
            if (sf) frames.pushBack(sf);
        }
    }

    if (frames.empty()) return nullptr;

    Animation* animation = Animation::createWithSpriteFrames(frames, 0.1f);
    return RepeatForever::create(Animate::create(animation));
}

// --- 战斗与状态逻辑 ---

void Player::placeBomb(Node* scene, MapLayer* mapLayer)
{
    if (!scene || !mapLayer || isDead || !canPlaceBomb) return;
    if (currentBombCount >= maxBombCount) return;

    Vec2 grid = mapLayer->worldToGrid(this->getPosition());

    // 检查格子上是否已有炸弹
    for (auto child : scene->getChildren()) {
        Bomb* b = dynamic_cast<Bomb*>(child);
        if (b && mapLayer->worldToGrid(b->getPosition()) == grid) return;
    }

    // 1. 创建炸弹
    Bomb* bomb = Bomb::createBomb(this->bombRange);
    if (!bomb) return;

    bomb->setOwner(this);
    bomb->setScale(2.0f);
    bomb->setPosition(mapLayer->gridToWorld(grid.x, grid.y));
    scene->addChild(bomb, 6);

    if (GameScene::s_isAudioOn) {
        AudioEngine::play2d("Sound/layProps.mp3", false, 1.0f);
    }

    // 2. 消耗加强次数
    if (_enhancedBombsLeft > 0) {
        _enhancedBombsLeft--;
        if (_enhancedBombsLeft == 0) {
            this->bombRange = _defaultBombRange;
        }
    }

    // 3. 更新状态与回调
    currentBombCount++;
    canPlaceBomb = false;

    bomb->startCountdown([this]() {
        currentBombCount--;
        });

    if (_scene) _scene->registerBomb(grid, bomb->range);
    resetBombCooldown();
}

void Player::resetBombCooldown()
{
    this->runAction(Sequence::create(
        DelayTime::create(bombCooldown),
        CallFunc::create([this]() { canPlaceBomb = true; }),
        nullptr
    ));
}

void Player::takeDamage()
{
    if (invincible || isDead) return;

    hp--;
    invincible = true;
    this->setOpacity(150);

    // 1秒后解除无敌
    this->runAction(Sequence::create(
        DelayTime::create(1.0f),
        CallFunc::create([this]() {
            invincible = false;
            this->setOpacity(255);
            }),
        nullptr
    ));

    if (hp <= 0) die();
}

void Player::die()
{
    if (isDead) return;
    isDead = true;

    this->stopAllActions();
    this->setColor(Color3B::RED);

    // 延迟消失
    this->runAction(Sequence::create(
        DelayTime::create(0.8f),
        CallFunc::create([this]() { this->setVisible(false); }),
        nullptr
    ));
}

void Player::addScore(int value)
{
    _score += value;
    if (_scene) {
        _scene->updateUIForPlayer(this);
    }
}

// --- 道具与效果 ---

void Player::pickItem(Item* item)
{
    if (!item) return;

    addScore(50);
    changeItemCount(1);

    if (GameScene::s_isAudioOn) {
        AudioEngine::play2d("Sound/get.mp3", false, 1.0f);
    }

    // 触发道具效果
    switch (item->getType())
    {
    case Item::ItemType::PowerBomb: increaseBombRange(); break;
    case Item::ItemType::Heal:      heal(); break;
    case Item::ItemType::Shield:    activateShield(5.0f); break;
    case Item::ItemType::Block:     blockOpponent(); break;
    case Item::ItemType::SpeedUp:   speedUp(3.0f, 1.5f); break;
    case Item::ItemType::Light:     activateLightEffect(10.0f); break;
    }

    item->playPickAnimationEffect(this);
}

void Player::changeItemCount(int delta)
{
    _itemHoldCount += delta;
    if (_itemHoldCount < 0) _itemHoldCount = 0;
    if (_scene) _scene->updateUIForPlayer(this);
}

void Player::increaseBombRange()
{
    this->bombRange = _defaultBombRange + 2;
    this->_enhancedBombsLeft = 3;
}

void Player::heal()
{
    if (hp < maxHp) hp++;
}

void Player::activateShield(float duration)
{
    hasShield = true;
    invincible = true;

    this->runAction(Sequence::create(
        DelayTime::create(duration),
        CallFunc::create([this]() {
            hasShield = false;
            invincible = false;
            changeItemCount(-1);
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
            changeItemCount(-1);
            }),
        nullptr
    ));
}

void Player::blockOpponent()
{
    if (!_scene) return;

    // 寻找最近的敌人
    Player* target = nullptr;
    float minDist = FLT_MAX;

    for (auto p : _scene->getPlayers()) {
        if (!p || p == this || p->isDead) continue;
        float d = this->getPosition().distance(p->getPosition());
        if (d < minDist) {
            minDist = d;
            target = p;
        }
    }

    if (target) {
        target->stunned = true;
        // 3秒后解除定身
        target->runAction(Sequence::create(
            DelayTime::create(3.0f),
            CallFunc::create([target]() { target->stunned = false; }),
            nullptr
        ));
    }
}

void Player::activateLightEffect(float duration)
{
    _lightTimer += duration;
    if (_lightTimer > 20.0f) _lightTimer = 20.0f;
    _visionRadius = 350.0f;
}

void Player::updateVision(float dt)
{
    if (_lightTimer > 0) {
        _lightTimer -= dt;
        if (_lightTimer <= 0) {
            _visionRadius = 150.0f; // 恢复默认视野
        }
    }
}

// --- AI 辅助 ---

Player* Player::findNearestEnemy(float minValidDist)
{
    if (!_scene) return nullptr;

    Player* target = nullptr;
    float minDist = FLT_MAX;

    for (auto p : _scene->getPlayers())
    {
        if (!p || p == this || p->isDead) continue;
        // 只锁定对立阵营 (AI vs Human)
        if (p->isAI == this->isAI) continue;

        float d = this->getPosition().distance(p->getPosition());
        if (d < minValidDist) continue;

        if (d < minDist) {
            minDist = d;
            target = p;
        }
    }
    return target;
}