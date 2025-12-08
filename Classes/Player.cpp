#include "Player.h"
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

void Player::moveUp() { this->setPositionY(this->getPositionY() + 5); }
void Player::moveDown() { this->setPositionY(this->getPositionY() - 5); }
void Player::moveLeft() { this->setPositionX(this->getPositionX() - 5); }
void Player::moveRight() { this->setPositionX(this->getPositionX() + 5); }

// ------------------ 新增：放炸弹 ------------------
void Player::placeBomb()
{
    auto bomb = Sprite::create("bomb.png");
    bomb->setScale(3.0);
    bomb->setPosition(this->getPosition());
    this->getParent()->addChild(bomb);

    // 2 秒后爆炸
    auto delay = DelayTime::create(2.0f);

    auto explode = CallFunc::create([=]() {
        auto boom = Sprite::create("explosion.png");
        boom->setPosition(bomb->getPosition());
        this->getParent()->addChild(boom);

        // 爆炸0.5秒后让爆炸图消失
        boom->runAction(Sequence::create(
            DelayTime::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        });

    bomb->runAction(Sequence::create(delay, explode, RemoveSelf::create(), nullptr));
}
