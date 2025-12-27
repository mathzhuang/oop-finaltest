#include "FogManager.h"
#include "Player.h"

USING_NS_CC;

bool FogManager::init()
{
    if (!Node::init()) return false;
    return true;
}

void FogManager::initFog(const Size& screenSize, float radius)
{
    _fogRadius = radius;
    _fogLayer = LayerColor::create(Color4B(0, 0, 0, 240), screenSize.width, screenSize.height);

    _stencil = SafeDrawNode::create();

    _clipNode = ClippingNode::create(_stencil);
    _clipNode->setInverted(true);
    _clipNode->setAlphaThreshold(0.05f);

    // 显式保证 stencil 的 parent 被设置（防止 ClippingNode 实现不自动 addChild）
    if (_stencil && _stencil->getParent() == nullptr) {
        _clipNode->addChild(_stencil);
    }

    _clipNode->addChild(_fogLayer);
    this->addChild(_clipNode, 100);
}

void FogManager::updateFog(Player* player)
{
    if (!_stencil || !_clipNode || !player || player->isDead) return;

    _stencil->safeClear();

    Vec2 worldPos = player->getPosition();
    Vec2 localPos = _stencil->getParent()->convertToNodeSpace(worldPos);

    // --- 💡 关键修改点 ---
    // 不要使用成员变量 _fogRadius，而是调用 player 的动态接口
    float dynamicRadius = player->getVisionRadius();
    // --------------------

    int steps = 20;
    float radiusStep = dynamicRadius / steps; // 使用动态半径计算步长

    for (int i = 0; i < steps; ++i)
    {
        // 同样，这里的计算全部基于 dynamicRadius
        float r = dynamicRadius - i * radiusStep;
        float alpha = 0.9f * (1.0f - i / (float)steps);
        _stencil->safeDrawSolidCircle(localPos, r, 50, Color4F(0, 0, 0, alpha));
    }
}

FogManager::~FogManager()
{
    if (_stencil) _stencil->removeFromParent();
    if (_clipNode) _clipNode->removeFromParent();
}

