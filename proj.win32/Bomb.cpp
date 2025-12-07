#include "Bomb.h"
USING_NS_CC;

Bomb* Bomb::createBomb(int range)
{
    Bomb* b = new (std::nothrow) Bomb();
    if (b && b->initWithFile("bomb.png"))   // 请把 bomb.png 放到 Resources
    {
        b->autorelease();
        b->_range = range;
        return b;
    }
    CC_SAFE_DELETE(b);
    return nullptr;
}

void Bomb::startCountdown()
{
    // 2 秒后爆炸
    this->runAction(
        Sequence::create(
            DelayTime::create(2.0f),
            CallFunc::create([=]() { this->explode(); }),
            nullptr
        )
    );
}

void Bomb::explode()
{
    // 爆炸的视觉效果（一个简单的 sprite）
    auto explosion = Sprite::create("explosion.png"); // 换成你的爆炸图片
    explosion->setPosition(this->getPosition());
    this->getParent()->addChild(explosion);

    // 爆炸 0.4 秒后消失
    explosion->runAction(
        Sequence::create(
            FadeOut::create(0.4f),
            RemoveSelf::create(),
            nullptr
        )
    );

    // 删除炸弹本体
    this->removeFromParent();
}
