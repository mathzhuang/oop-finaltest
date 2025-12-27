#pragma once
#include "cocos2d.h"
class Player;


class Item : public cocos2d::Sprite
{
public:
    enum class ItemType {
        PowerBomb = 0,
        Heal,
        Shield,
        Block,
        SpeedUp,
        Light,
        MAX_TYPES // 自动计算数量
    };
    static Item* createItem(ItemType type);

    ItemType getType() const { return _type; }

    // 出现动画
    void playSpawnAnimation();

    // 被拾取动画
    void playPickAnimation(const std::function<void()>& onFinish);

    static Item* createRandom(bool isFogMode = false);

    void playPickAnimationEffect(Player* player);
private:
    ItemType _type;

    bool initWithType(ItemType type);
    cocos2d::Sprite* _sprite = nullptr;
    cocos2d::Sprite* _effectSprite = nullptr;
    // 各类效果实现
    void showFloatingText(const cocos2d::Vec2& pos, const std::string& text, const cocos2d::Color4B& color);
    void showShieldEffect(Player* player, float duration);
    void showSpeedEffect(Player* player, float duration);
    void showBombEffect(Player* player);
    void showBlockEffect(Player* target);

};
