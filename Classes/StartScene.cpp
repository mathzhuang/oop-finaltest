#include "StartScene.h"
#include "GameScene.h"
#include "module.h"
#include "HowtoplayScene.h"
#include "HighScoresScene.h"
#include "AudioEngine.h"

USING_NS_CC;
using namespace cocos2d::experimental;

// --- 生命周期 ---

Scene* StartScene::createScene()
{
    return StartScene::create();
}

bool StartScene::init()
{
    if (!Scene::init()) return false;

    // 1. 音频逻辑 (背景音乐)
    // 检查是否已有音乐在播放，防止场景切换时音乐重置
    if (GameScene::s_menuAudioID == AudioEngine::INVALID_AUDIO_ID) {
        GameScene::s_menuAudioID = AudioEngine::play2d("Sound/StartSound.mp3", true, 0.6f);
    }

    // 同步全局静音开关
    if (!GameScene::s_isAudioOn) {
        AudioEngine::pause(GameScene::s_menuAudioID);
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 2. 初始化背景
    auto bg = Sprite::create("UI/startBackground.png");
    if (bg) {
        bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(bg, 0);
    }
    else {
        CCLOG("Error: Background image not found!");
    }

    // 3. 初始化菜单按钮
    // 统一 X 轴位置
    float btnX = visibleSize.width * 0.75 + origin.x;

    // Start 按钮
    auto btnStart = MenuItemImage::create(
        "UI/start.png", "UI/start-after.png",
        CC_CALLBACK_1(StartScene::onButton1Click, this));
    if (btnStart) btnStart->setPosition(Vec2(btnX, visibleSize.height * 0.7 + origin.y));

    // How-to-play 按钮
    auto btnHelp = MenuItemImage::create(
        "UI/htp-button.png", "UI/htp-button-after.png",
        CC_CALLBACK_1(StartScene::onButton2Click, this));
    if (btnHelp) btnHelp->setPosition(Vec2(btnX, visibleSize.height * 0.5 + origin.y));

    // Score 按钮
    auto btnScore = MenuItemImage::create(
        "UI/score.png", "UI/score-after.png",
        CC_CALLBACK_1(StartScene::onButton3Click, this));
    if (btnScore) btnScore->setPosition(Vec2(btnX, visibleSize.height * 0.3 + origin.y));

    // 4. 构建菜单容器
    auto menu = Menu::create(btnStart, btnHelp, btnScore, nullptr);
    menu->setPosition(Vec2::ZERO); // 归零以使用绝对坐标
    this->addChild(menu, 1);

    return true;
}

// --- 交互回调 ---

void StartScene::onButton1Click(Ref* pSender)
{
    // 跳转 -> 模式选择 (moduleScene)
    auto scene = moduleScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void StartScene::onButton2Click(Ref* pSender)
{
    // 跳转 -> 游戏说明
    auto scene = HowToPlayScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void StartScene::onButton3Click(Ref* pSender)
{
    // 跳转 -> 排行榜
    auto scene = HighScoresScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}