#pragma once
#include "cocos2d.h"

class Item : public cocos2d::Sprite
{
public:
    enum class ItemType
    {
        BombPower,   // 增加爆炸范围
        SpeedUp,     // 增加移速
    };

    static Item* createItem(ItemType type);

    ItemType getType() const { return _type; }

    // 出现动画
    void playSpawnAnimation();

    // 被拾取动画
    void playPickAnimation(const std::function<void()>& onFinish);

private:
    ItemType _type;

    bool initWithType(ItemType type);
};
