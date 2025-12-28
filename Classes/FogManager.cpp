#include "FogManager.h"
#include "Player.h"

USING_NS_CC;

// --- 生命周期 ---

bool FogManager::init()
{
    if (!Node::init()) return false;
    return true;
}

FogManager::~FogManager()
{
    CC_SAFE_RELEASE(_lightBrush);
}

// --- 初始化配置 ---

void FogManager::initFog(const Size& screenSize)
{
    // 避开左侧UI侧边栏
    float leftSideWidth = 609.0f;
    _fogSize = Size(screenSize.width - leftSideWidth, screenSize.height);

    // 偏移坐标，物理避开UI区域
    this->setPosition(Vec2(leftSideWidth, 0));

    // 创建画布并居中
    _renderTexture = RenderTexture::create(_fogSize.width, _fogSize.height, Texture2D::PixelFormat::RGBA8888);
    _renderTexture->setPosition(_fogSize / 2);
    this->addChild(_renderTexture);

    // 生成柔光笔刷，retain防止自动释放
    _lightBrush = generateSoftLightSprite(200.0f);
    _lightBrush->retain();

    // 混合模式：利用笔刷Alpha值反向擦除目标层颜色 (实现透视效果)
    BlendFunc blend = { GL_ZERO, GL_ONE_MINUS_SRC_ALPHA };
    _lightBrush->setBlendFunc(blend);
}

// --- 核心逻辑 ---

void FogManager::updateFog(Player* player)
{
    if (!_renderTexture || !_lightBrush || !player) return;

    // 转为局部坐标
    Vec2 localPos = this->convertToNodeSpace(player->getPosition());
    _lightBrush->setPosition(localPos);

    // 动态计算笔刷缩放：基于视野半径，放大1.2倍优化边缘柔和度
    float scale = (player->getVisionRadius() / 200.0f) * 1.2f;
    _lightBrush->setScale(scale);

    // 重置全屏迷雾：0.96f为黑度
    _renderTexture->beginWithClear(0, 0, 0, 0.96f);

    // 绘制笔刷进行擦除
    _lightBrush->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);

    _renderTexture->end();
}

// --- 辅助工具 ---

Sprite* FogManager::generateSoftLightSprite(float radius)
{
    auto draw = DrawNode::create();

    // 绘制多层同心圆模拟径向渐变
    int steps = 40;
    for (int i = 0; i < steps; i++)
    {
        float r = radius * (1.0f - (float)i / steps);

        // 指数衰减 (pow 2.0)，中心亮边缘柔
        float alpha = (float)i / steps;
        alpha = pow(alpha, 2.0f);

        draw->drawSolidCircle(Vec2(radius, radius), r, 0, 60, Color4F(1, 1, 1, 0.05f));
    }

    // 烘焙为纹理
    auto rt = RenderTexture::create(radius * 2, radius * 2);
    rt->beginWithClear(0, 0, 0, 0);
    draw->setPosition(Vec2::ZERO);
    draw->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);
    rt->end();

    // 创建Sprite并修正Y轴翻转
    auto tex = rt->getSprite()->getTexture();
    auto sprite = Sprite::createWithTexture(tex);
    sprite->setFlippedY(true);

    return sprite;
}