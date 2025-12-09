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
void Player::placeBomb(cocos2d::Node* scene, MapLayer* mapLayer)
{
    if (!scene) return;

    Vec2 grid(-1, -1);
    if (mapLayer)
        grid = mapLayer->worldToGrid(this->getPosition());

    // 检查该格子是否已有炸弹
    bool hasBomb = false;
    for (auto child : scene->getChildren())
    {
        Bomb* b = dynamic_cast<Bomb*>(child);
        if (!b) continue;
        if (!mapLayer) continue;

        Vec2 bGrid = mapLayer->worldToGrid(b->getPosition());
        if (bGrid == grid)
        {
            hasBomb = true;
            break;
        }
    }
    if (hasBomb) return; // 已有炸弹，不放

    // 创建炸弹
    Bomb* bomb = Bomb::createBomb();
    if (!bomb) return;

    bomb->setScale(2.0f);

    // 对齐到网格
    if (mapLayer)
    {
        Vec2 world = mapLayer->gridToWorld((int)grid.x, (int)grid.y);
        bomb->setPosition(world);
    }
    else
    {
        bomb->setPosition(this->getPosition());
    }

    // 添加到场景
    scene->addChild(bomb);

    // 开始倒计时
    bomb->startCountdown();
}


void Player::takeDamage()
{
    if (invincible) return;   // 无敌则不扣血

    hp -= 1;

    CCLOG("Player HP = %d", hp);

    // 进入无敌状态
    invincible = true;
    this->setOpacity(150); // 变透明表示无敌

    // 一秒后解除无敌
    this->runAction(Sequence::create(
        DelayTime::create(1.0f),
        CallFunc::create([this]() {
            invincible = false;
            this->setOpacity(255);
            }),
        nullptr
    ));

    // 死亡判定
    if (hp <= 0)
    {
        die();
    }
}

void Player::die()
{
    if (isDead) return;
    isDead = true;

    // ① 停止移动
    this->stopAllActions();

    // ② 可替换为死亡动画
    this->setColor(Color3B::RED);

    // ③ 延迟退出 or 重开游戏
    this->runAction(Sequence::create(
        DelayTime::create(0.8f),
        CallFunc::create([]() {
            // TODO: 这里换成你的 GameOverScene 或重新加载 GameScene
            Director::getInstance()->replaceScene(
                TransitionFade::create(1.0f, Scene::create())
            );
            }),
        nullptr
    ));
}
