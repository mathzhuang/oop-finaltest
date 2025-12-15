#pragma once
#include "cocos2d.h"

class Item : public cocos2d::Sprite
{
public:
    enum class ItemType
    {
        PowerBomb,  // 强力炸弹（火焰范围+1）
        Heal,       // 恢复药水（hp +1）
        Shield,     // 安全帽（5s保护）
        Block,      // 路障（定住对手3s）
        SpeedUp     // 加速
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
