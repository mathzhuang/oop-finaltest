#include "Item.h"
using namespace cocos2d;

Item* Item::createItem(ItemType type)
{
    Item* ret = new Item();
    if (ret && ret->initWithType(type))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Item::initWithType(ItemType type)
{
    _type = type;

    // —— 不同道具用不同贴图 ——
    std::string texture;
    switch (type)
    {
    case ItemType::BombPower:
        texture = "item_bombpower.png";
        break;
    case ItemType::SpeedUp:
        texture = "item_speedup.png";
        break;
    default:
        texture = "item_default.png";
        break;
    }

    if (!Sprite::initWithFile(texture))
        return false;

    playSpawnAnimation();

    return true;
}

void Item::playSpawnAnimation()
{
    // 初始缩放为 0，然后放大弹跳
    this->setScale(0.0f);
    auto act = EaseBackOut::create(ScaleTo::create(0.3f, 1.0f));
    this->runAction(act);
}

void Item::playPickAnimation(const std::function<void()>& onFinish)
{
    auto fade = FadeOut::create(0.2f);
    auto scale = ScaleTo::create(0.2f, 1.5f);

    auto spawn = Spawn::create(fade, scale, nullptr);

    auto seq = Sequence::create(
        spawn,
        CallFunc::create(onFinish),
        nullptr
    );

    this->runAction(seq);
}
