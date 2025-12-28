#pragma once
#include "cocos2d.h"

class Player;

class Item : public cocos2d::Sprite
{
public:
    // --- 类型定义 ---
    enum class ItemType {
        PowerBomb = 0,
        Heal,
        Shield,
        Block,
        SpeedUp,
        Light,
        MAX_TYPES // 辅助计数
    };

    // --- 工厂方法 ---
    static Item* createItem(ItemType type);

    // 随机生成道具 (isFogMode为真时强制生成Light)
    static Item* createRandom(bool isFogMode = false);

    // --- 属性接口 ---
    ItemType getType() const { return _type; }

    // --- 动画逻辑 ---

    // 播放出生动画 (弹跳出现)
    void playSpawnAnimation();

    // 播放被拾取动画 (放大淡出，完成后回调)
    void playPickAnimation(const std::function<void()>& onFinish);

    // 播放拾取后的效果反馈 (如飘字、护盾光圈)
    void playPickAnimationEffect(Player* player);

private:
    // --- 内部实现 ---
    bool initWithType(ItemType type);

    // --- 特效辅助函数 ---
    void showFloatingText(const cocos2d::Vec2& pos, const std::string& text, const cocos2d::Color4B& color);
    void showShieldEffect(Player* player, float duration);
    void showSpeedEffect(Player* player, float duration);
    void showBombEffect(Player* player);
    void showBlockEffect(Player* target);

private:
    // --- 成员变量 ---
    ItemType _type;
    cocos2d::Sprite* _sprite = nullptr;
    cocos2d::Sprite* _effectSprite = nullptr;
};