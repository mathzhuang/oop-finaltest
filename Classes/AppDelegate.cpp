
#include "AppDelegate.h"
#include "StartScene.h"

// 音频引擎配置宏检查
// #define USE_AUDIO_ENGINE 1
// #define USE_SIMPLE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

// 设计分辨率 (2K)
static cocos2d::Size designResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
}

// --- OpenGL 配置 ---

void AppDelegate::initGLContextAttrs()
{
    // 设置 OpenGL 上下文属性: r,g,b,a, depth, stencil, multisamples
    GLContextAttrs glContextAttrs = { 8, 8, 8, 8, 24, 8, 0 };
    GLView::setGLContextAttrs(glContextAttrs);
}

// --- 应用启动核心逻辑 ---

bool AppDelegate::applicationDidFinishLaunching() {
    // 1. 初始化导演类
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();

    // 2. 创建 OpenGL 视图
    if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        // 桌面端：设置窗口标题和初始大小
        glview = GLViewImpl::createWithRect("BomberGame", cocos2d::Rect(0, 0, 2048, 1536));
#else
        // 移动端：全屏
        glview = GLViewImpl::create("BomberGame");
#endif
        director->setOpenGLView(glview);
    }

    // 3. 性能数据显示 (帧率/DrawCall)
    director->setDisplayStats(true);

    // 4. 设置目标帧率 (60 FPS)
    director->setAnimationInterval(1.0f / 60);

    // 5. 屏幕适配策略
    // SHOW_ALL: 保持宽高比，显示全部内容，可能有黑边
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::SHOW_ALL);

    // 6. 运行首场景
    auto scene = StartScene::createScene();
    director->runWithScene(scene);

    return true;
}

// --- 前后台切换处理 ---

// 应用切入后台 (如来电、Home键)
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// 应用切回前台
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}