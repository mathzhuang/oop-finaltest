#pragma once
#include "cocos2d.h"

class Bomb : public cocos2d::Sprite
{
public:
    static Bomb* createBomb(int range = 1);  // range = ±¬Õ¨·¶Î§£¨Î´À´¿ÉÉý¼¶£©

    void startCountdown();   // ¿ªÊ¼µ¹¼ÆÊ±
    void explode();          // ±¬Õ¨

private:
    int _range = 1;          // ±¬Õ¨·¶Î§
};
