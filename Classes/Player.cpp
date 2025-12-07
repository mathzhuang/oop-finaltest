#include "Player.h"
USING_NS_CC;

Player* Player::createPlayer()
{
    Player* p = new (std::nothrow) Player();
    if (p && p->initWithFile("player_black(1).png"))
    {
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

// ------------------ ÐÂÔö£º·ÅÕ¨µ¯ ------------------
void Player::placeBomb()
{
    auto bomb = Sprite::create("bomb.png");
    bomb->setPosition(this->getPosition());
    this->getParent()->addChild(bomb);

    // 2 Ãëºó±¬Õ¨
    auto delay = DelayTime::create(2.0f);

    auto explode = CallFunc::create([=]() {
        auto boom = Sprite::create("explosion.png");
        boom->setPosition(bomb->getPosition());
        this->getParent()->addChild(boom);

        // ±¬Õ¨0.5ÃëºóÈÃ±¬Õ¨Í¼ÏûÊ§
        boom->runAction(Sequence::create(
            DelayTime::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        });

    bomb->runAction(Sequence::create(delay, explode, RemoveSelf::create(), nullptr));
}
