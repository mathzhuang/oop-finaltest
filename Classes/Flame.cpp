#include "Flame.h"
USING_NS_CC;

Flame* Flame::createFlame()
{
    Flame* f = new Flame();
    if (f && f->initWithFile("explosion(1).png"))
    {
        f->setScale(2.5f);
        f->autorelease();
        return f;
    }
    delete f;
    return nullptr;
}
