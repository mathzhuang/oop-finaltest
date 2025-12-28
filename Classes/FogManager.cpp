#include "FogManager.h"
#include "Player.h"

USING_NS_CC;

bool FogManager::init()
{
    if (!Node::init()) return false;
    return true;
}

//void FogManager::initFog(const Size& screenSize, float radius)
//{
//    _fogRadius = radius;
//    _fogLayer = LayerColor::create(Color4B(0, 0, 0, 240), screenSize.width, screenSize.height);
//
//    _stencil = SafeDrawNode::create();
//
//    _clipNode = ClippingNode::create(_stencil);
//    _clipNode->setInverted(true);
//    _clipNode->setAlphaThreshold(0.05f);
//
//    // 显式保证 stencil 的 parent 被设置（防止 ClippingNode 实现不自动 addChild）
//    if (_stencil && _stencil->getParent() == nullptr) {
//        _clipNode->addChild(_stencil);
//    }
//
//    _clipNode->addChild(_fogLayer);
//    this->addChild(_clipNode, 100);
//}
void FogManager::initFog(const Size& screenSize)
{
    // 1. 定义左侧侧边栏宽度
    float leftSideWidth = 609.0f;

    // 2. 计算迷雾区的实际大小 (屏幕宽 - 侧边栏, 屏幕高)
    _fogSize = Size(screenSize.width - leftSideWidth, screenSize.height);

    // 3. 设置 FogManager 自身的位置
    // 这样 FogManager 内部坐标 (0,0) 就对应屏幕的 (609, 0)
    // 从而物理上避开了左侧 UI
    this->setPosition(Vec2(leftSideWidth, 0));

    // 4. 创建画布
    _renderTexture = RenderTexture::create(_fogSize.width, _fogSize.height, Texture2D::PixelFormat::RGBA8888);
    _renderTexture->setPosition(_fogSize / 2); // 居中放到 Node 上
    this->addChild(_renderTexture);

    // 5. 生成虚化笔刷 (半径设大一点，利用缩放控制实际大小)
    _lightBrush = generateSoftLightSprite(200.0f);
    _lightBrush->retain(); // 保持引用，避免被自动回收

    // 6.设置混合模式
    // GL_ZERO: 丢弃源像素(笔刷)的颜色
    // GL_ONE_MINUS_SRC_ALPHA: 目标像素(黑色迷雾) = 目标像素 * (1 - 笔刷透明度)
    // 效果：笔刷越不透明的地方，黑色迷雾就被擦除得越干净；笔刷半透明边缘，迷雾就半透明。
    BlendFunc blend = { GL_ZERO, GL_ONE_MINUS_SRC_ALPHA };
    _lightBrush->setBlendFunc(blend);
}

//void FogManager::updateFog(Player* player)
//{
//    if (!_stencil || !_clipNode || !player || player->isDead) return;
//
//    _stencil->safeClear();
//
//    Vec2 worldPos = player->getPosition();
//    Vec2 localPos = _stencil->getParent()->convertToNodeSpace(worldPos);
//
//    // --- 💡 关键修改点 ---
//    // 不要使用成员变量 _fogRadius，而是调用 player 的动态接口
//    float dynamicRadius = player->getVisionRadius();
//    // --------------------
//
//    int steps = 20;
//    float radiusStep = dynamicRadius / steps; // 使用动态半径计算步长
//
//    for (int i = 0; i < steps; ++i)
//    {
//        // 同样，这里的计算全部基于 dynamicRadius
//        float r = dynamicRadius - i * radiusStep;
//        float alpha = 0.9f * (1.0f - i / (float)steps);
//        _stencil->safeDrawSolidCircle(localPos, r, 50, Color4F(0, 0, 0, alpha));
//    }
//}
//
//FogManager::~FogManager()
//{
//    if (_stencil) _stencil->removeFromParent();
//    if (_clipNode) _clipNode->removeFromParent();
//}

void FogManager::updateFog(Player* player)
{
    if (!_renderTexture || !_lightBrush || !player) return;

    // 1. 获取玩家在迷雾层中的相对坐标
    // convertToNodeSpace 会自动减去 FogManager 的坐标 (609, 0)
    Vec2 localPos = this->convertToNodeSpace(player->getPosition());

    // 2. 设置笔刷位置
    _lightBrush->setPosition(localPos);

    // 3. 设置笔刷大小 (根据玩家当前的平滑视野半径)
    // 笔刷原始半径是 200，除以 200 归一化，再乘以玩家实际视野
    // 额外乘 1.2f 是为了让虚化边缘稍微大一点，看起来更自然
    float scale = (player->getVisionRadius() / 200.0f) * 1.2f;
    _lightBrush->setScale(scale);

    // 4. 开始绘制一帧
    _renderTexture->beginWithClear(0, 0, 0, 0.96f); // 0.96f 是迷雾浓度(0~1)，越黑越高

    // 绘制笔刷 (执行擦除操作)
    _lightBrush->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);

    _renderTexture->end();
}


Sprite* FogManager::generateSoftLightSprite(float radius)
{
    // 创建一个临时的 DrawNode
    auto draw = DrawNode::create();

    // 画很多层透明度递减的圆，模拟径向渐变
    int steps = 40;
    for (int i = 0; i < steps; i++)
    {
        float r = radius * (1.0f - (float)i / steps);
        float alpha = (float)i / steps; // 中心不透明，边缘透明
        // 指数衰减，让中心更亮，边缘衰减更柔和
        alpha = pow(alpha, 2.0f);

        draw->drawSolidCircle(Vec2(radius, radius), r, 0, 60, Color4F(1, 1, 1, 0.05f));
    }

    // 将 DrawNode 渲染成纹理
    auto rt = RenderTexture::create(radius * 2, radius * 2);
    rt->beginWithClear(0, 0, 0, 0);
    draw->setPosition(Vec2::ZERO);
    draw->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);
    rt->end();

    // 从纹理创建 Sprite
    auto tex = rt->getSprite()->getTexture();
    auto sprite = Sprite::createWithTexture(tex);

    // 翻转 Y 轴 (RenderTexture 生成的纹理通常是倒的)
    sprite->setFlippedY(true);

    return sprite;
}


FogManager::~FogManager()
{
    CC_SAFE_RELEASE(_lightBrush);
}
