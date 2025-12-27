#include "Item.h"
#include "Player.h"
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
        filename = "speedup(1).png";
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
Item* Item::createRandom()
{
    // 使用 MAX_TYPES 自动适配，无论你加多少种道具都不用改这里
    int maxIdx = static_cast<int>(ItemType::MAX_TYPES) - 1; 
    
    // 如果没有道具定义，防止报错
    if (maxIdx < 0) return nullptr;

    int r = cocos2d::RandomHelper::random_int(0, maxIdx);
    ItemType type = static_cast<ItemType>(r);
    
    CCLOG("Randomly picked item type: %d", r); // 方便调试看有没有抽到 4
    return Item::createItem(type);
}
// Item.cpp
void Item::playPickAnimationEffect(Player* player)
{
    switch(_type)
    {
        case ItemType::Heal:
            showFloatingText(player->getPosition(), "+1 HP", Color4B::GREEN);
            break;
        case ItemType::Shield:
            showShieldEffect(player, 5.0f);
            break;
        case ItemType::SpeedUp:
            showSpeedEffect(player, 3.0f);
            break;
        
    }
}
void Item::showFloatingText(const Vec2& pos, const std::string& text, const Color4B& color)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 24);
    label->setTextColor(color);
    label->setPosition(pos);
    this->getParent()->addChild(label, 20);

    auto moveUp = MoveBy::create(1.0f, Vec2(0, 50));
    auto fadeOut = FadeOut::create(1.0f);
    auto seq = Sequence::create(Spawn::create(moveUp, fadeOut, nullptr), RemoveSelf::create(), nullptr);

    label->runAction(seq);
}
void Item::showShieldEffect(Player* player, float duration)
{
    auto shieldSprite = Sprite::create("shield(2).png"); // 你需要一张光圈图片
    shieldSprite->setPosition(player->getContentSize() / 2);
    shieldSprite->setOpacity(150);
    player->addChild(shieldSprite, 15);

    auto fade = FadeOut::create(duration);
    auto seq = Sequence::create(DelayTime::create(duration), RemoveSelf::create(), nullptr);
    shieldSprite->runAction(seq);
}
void Item::showSpeedEffect(Player* player, float duration)
{
    auto effect = Sprite::create("speedup(2).png"); // 轨迹或光圈
    effect->setPosition(player->getContentSize() / 2);
    player->addChild(effect, 15);

    auto blink = Blink::create(duration, 5); // 闪烁效果
    auto seq = Sequence::create(blink, RemoveSelf::create(), nullptr);
    effect->runAction(seq);
}


