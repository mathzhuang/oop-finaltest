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
    void initFog(const cocos2d::Size& screenSize);

    // 每帧更新迷雾
    void updateFog(Player* player);

    

    virtual ~FogManager();

private:
    //cocos2d::ClippingNode* _clipNode = nullptr;   // 裁剪节点
    //cocos2d::LayerColor* _fogLayer = nullptr;     // 黑色迷雾
    //SafeDrawNode* _stencil = nullptr;            // 可见区域
    //float _fogRadius = 150.0f;
    //int _gradientSteps = 10;                     // 渐变边缘

    // 渲染纹理（画布）
    cocos2d::RenderTexture* _renderTexture = nullptr;

    // 光圈笔刷（用于擦除黑色）
    cocos2d::Sprite* _lightBrush = nullptr;

    // 迷雾区域的大小
    cocos2d::Size _fogSize;

    // 内部函数：程序化生成一个虚化的圆形笔刷
    cocos2d::Sprite* generateSoftLightSprite(float radius);
};
