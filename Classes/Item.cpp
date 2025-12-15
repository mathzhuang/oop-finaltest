#include "Item.h"

USING_NS_CC;

Item* Item::createItem(ItemType type)
{
    Item* ret = new (std::nothrow) Item();
    if (ret && ret->initWithType(type))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Item::initWithType(ItemType t)
{
    _type = t;

    std::string filename;

    // 根据类型选择贴图
    switch (t)
    {
    case ItemType::PowerBomb:
        filename = "powerbomb(1).png";
        break;

    case ItemType::Heal:
        filename = "heal(1).png";
        break;

    case ItemType::Shield:
        filename = "shield(1).png";
        break;

    case ItemType::Block:
        filename = "block(1).png";
        break;

    case ItemType::SpeedUp:
        filename = "speed(1).png";
        break;
    }

    // 初始化 Sprite
    if (!Sprite::initWithFile(filename))
        return false;

    // 统一缩放
    this->setScale(2.0f);

    // Item tag（GameScene 用来判断拾取）
    this->setTag(500);

    return true;
}

void Item::playSpawnAnimation()
{
    // 出生弹跳动画
    this->setScale(0.0f);
    auto act = EaseBackOut::create(
        ScaleTo::create(0.3f, 2.0f)
    );
    this->runAction(act);
}

void Item::playPickAnimation(const std::function<void()>& onFinish)
{
    auto fade = FadeOut::create(0.2f);
    auto scale = ScaleTo::create(0.2f, 2.5f);

    auto spawn = Spawn::create(fade, scale, nullptr);

    auto seq = Sequence::create(
        spawn,
        CallFunc::create(onFinish),
        nullptr
    );

    this->runAction(seq);
}
