#pragma once
#include "cocos2d.h"
#include "SafeDrawNode.h"


class Player;

class FogManager : public cocos2d::Node
{
public:
    CREATE_FUNC(FogManager);
    virtual bool init() override;

    // 初始化迷雾层
    void initFog(const cocos2d::Size& screenSize, float radius = 150.0f);

    // 每帧更新迷雾
    void updateFog(Player* player);

    void setFogRadius(float radius) { _fogRadius = radius; }

    virtual ~FogManager();

private:
    cocos2d::ClippingNode* _clipNode = nullptr;   // 裁剪节点
    cocos2d::LayerColor* _fogLayer = nullptr;     // 黑色迷雾
    SafeDrawNode* _stencil = nullptr;            // 可见区域
    float _fogRadius = 150.0f;
    int _gradientSteps = 10;                     // 渐变边缘
};
