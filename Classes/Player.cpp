#include "Player.h"
USING_NS_CC;

Player* Player::createPlayer()
{
    Player* p = new (std::nothrow) Player();
    if (p && p->initWithFile("player_black(1).png"))   // 你后面可以换成你喜欢的Q版角色
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
