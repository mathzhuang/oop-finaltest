#include "Item.h"
#include "Player.h"

USING_NS_CC;

// --- 工厂方法 ---

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

Item* Item::createRandom(bool isFogMode)
{
    if (isFogMode)
    {
        // 迷雾模式：强制产出 Light (增加视野)
        CCLOG("Fog Mode: Spawning Light item.");
        return Item::createItem(ItemType::Light);
    }
    else
    {
        // 普通模式：随机产出 Light 之外的道具
        int lightIdx = static_cast<int>(ItemType::Light);
        int maxIdx = lightIdx - 1;

        if (maxIdx < 0) return nullptr;

        int r = cocos2d::RandomHelper::random_int(0, maxIdx);
        ItemType type = static_cast<ItemType>(r);

        CCLOG("Normal Mode: Random item type: %d", r);
        return Item::createItem(type);
    }
}

// --- 初始化 ---

bool Item::initWithType(ItemType t)
{
    _type = t;
    std::string filename;

    // 资源映射
    switch (t)
    {
    case ItemType::PowerBomb: filename = "powerbomb(1).png"; break;
    case ItemType::Heal:      filename = "heal(1).png";      break;
    case ItemType::Shield:    filename = "shield(1).png";    break;
    case ItemType::Block:     filename = "block(1).png";     break;
    case ItemType::SpeedUp:   filename = "speedup(1).png";   break;
    case ItemType::Light:     filename = "light(1).png";     break;
    default: return false;
    }

    if (!Sprite::initWithFile(filename)) return false;

    // 基础配置
    this->setScale(2.0f);
    this->setTag(500); // 供GameScene识别拾取

    return true;
}

// --- 动画逻辑 ---

void Item::playSpawnAnimation()
{
    // 弹跳出生效果：从0放大到2.0
    this->setScale(0.0f);
    auto act = EaseBackOut::create(ScaleTo::create(0.3f, 2.0f));
    this->runAction(act);
}

void Item::playPickAnimation(const std::function<void()>& onFinish)
{
    // 拾取效果：放大并淡出
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

void Item::playPickAnimationEffect(Player* player)
{
    // 根据类型播放不同的反馈特效
    switch (_type)
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
    case ItemType::Light:
        showFloatingText(player->getPosition(), "VISION UP!", Color4B::YELLOW);
        break;
    default:
        break;
    }
}

// --- 视觉特效辅助 ---

void Item::showFloatingText(const Vec2& pos, const std::string& text, const Color4B& color)
{
    auto label = Label::createWithTTF(text, "fonts/arial.ttf", 24);
    label->setTextColor(color);
    label->setPosition(pos);
    // 添加到父节点(GameLayer)而非自身，防止Item移除后文字消失
    if (this->getParent()) {
        this->getParent()->addChild(label, 20);
    }

    // 向上飘动并淡出
    auto moveUp = MoveBy::create(1.0f, Vec2(0, 50));
    auto fadeOut = FadeOut::create(1.0f);
    auto seq = Sequence::create(
        Spawn::create(moveUp, fadeOut, nullptr),
        RemoveSelf::create(),
        nullptr
    );

    label->runAction(seq);
}

void Item::showShieldEffect(Player* player, float duration)
{
    auto shieldSprite = Sprite::create("shield(2).png");
    shieldSprite->setPosition(player->getContentSize() / 2);
    shieldSprite->setOpacity(150);
    player->addChild(shieldSprite, 15);

    // 持续一段时间后消失
    auto seq = Sequence::create(
        DelayTime::create(duration),
        RemoveSelf::create(),
        nullptr
    );
    shieldSprite->runAction(seq);
}

void Item::showSpeedEffect(Player* player, float duration)
{
    auto effect = Sprite::create("speedup(2).png");
    effect->setPosition(player->getContentSize() / 2);
    player->addChild(effect, 15);

    // 闪烁特效
    auto blink = Blink::create(duration, 5);
    auto seq = Sequence::create(
        blink,
        RemoveSelf::create(),
        nullptr
    );
    effect->runAction(seq);
}