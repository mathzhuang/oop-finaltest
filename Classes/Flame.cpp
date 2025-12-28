#include "Flame.h"

USING_NS_CC;

// --- 工厂方法 ---

Flame* Flame::createFlame()
{
    Flame* f = new Flame();
    if (f && f->initWithFile("explosion(1).png"))
    {
        f->setScale(2.5f); // 保持原有的视觉缩放
        f->autorelease();
        return f;
    }
    delete f;
    return nullptr;
}