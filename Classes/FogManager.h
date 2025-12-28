#pragma once
#include "cocos2d.h"
#include "SafeDrawNode.h"

class Player;

class FogManager : public cocos2d::Node
{
public:
    // --- 生命周期 ---
    CREATE_FUNC(FogManager);
    virtual bool init() override;
    virtual ~FogManager();

    // --- 对外接口 ---

    // 初始化配置 (计算区域、创建画布)
    void initFog(const cocos2d::Size& screenSize);

    // 刷新迷雾 (每一帧调用)
    void updateFog(Player* player);

private:
    // --- 内部实现 ---

    // 生成程序化柔光笔刷
    cocos2d::Sprite* generateSoftLightSprite(float radius);

private:
    // --- 成员变量 ---

    cocos2d::RenderTexture* _renderTexture = nullptr; // 迷雾画布
    cocos2d::Sprite* _lightBrush = nullptr;    // 擦除笔刷 (反向遮罩)
    cocos2d::Size           _fogSize;                 // 迷雾有效区域
};