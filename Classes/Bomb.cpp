#include "Bomb.h"
USING_NS_CC;

Bomb* Bomb::createBomb(int range)
{
    Bomb* b = new (std::nothrow) Bomb();
    if (b && b->initWithFile("bomb(1).png"))   // 请把 bomb.png 放到 Resources
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
    // 在 parent 上显示一个爆炸特效（临时：单点爆炸）
    auto parent = this->getParent();
    if (!parent) return;

    auto boom = Sprite::create("explosion.png");
    boom->setPosition(this->getPosition());
    parent->addChild(boom);

    // 持续 0.4 秒后移除
    boom->runAction(Sequence::create(
        DelayTime::create(0.4f),
        RemoveSelf::create(),
        nullptr
    ));

    // 将来：这里需要做爆炸范围扩散、碰到砖块摧毁 mapLayer.setTile(...)
    // 现在先移除炸弹自身
    this->removeFromParent();
}