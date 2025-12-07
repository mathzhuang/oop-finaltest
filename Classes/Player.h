#pragma once
#include "cocos2d.h"

class Player : public cocos2d::Sprite
{
public:
    static Player* createPlayer();

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
};
