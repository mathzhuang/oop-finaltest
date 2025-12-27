#include "StartScene.h"
// 需要跳转的目标场景头文件
#include "GameScene.h"
#include"module.h"
#include"HowtoplayScene.h"
#include "AudioEngine.h"


USING_NS_CC;
using namespace cocos2d::experimental;

Scene* StartScene::createScene()
{
    return StartScene::create();
}

bool StartScene::init()
{
    // 1. 父类初始化失败则返回 false
    if (!Scene::init())
    {
        return false;
    }

    // --- 音频逻辑 ---
    // 如果菜单音乐没有在播放，并且全局音效是开启的（或者默认为开启）
    if (GameScene::s_menuAudioID == AudioEngine::INVALID_AUDIO_ID) {
        // 播放 StartSound，循环
        GameScene::s_menuAudioID = AudioEngine::play2d("Sound/StartSound.mp3", true, 0.6f);
    }

    // 确保从游戏场景返回时，如果音效是关的，这里也要暂停
    if (!GameScene::s_isAudioOn) {
        AudioEngine::pause(GameScene::s_menuAudioID);
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 2. 添加背景 (2048*1537)
    auto sprite = Sprite::create("UI/startBackground.png");

    // 安全检查：如果图片没找到，打印错误
    if (sprite == nullptr) {
        CCLOG("Error: Background image not found! Check file name and path.");
    }
    else {
        sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(sprite, 0);
    }

    // 3. 创建三个按钮

    // 参数说明：create(正常状态图片, 按下状态图片, 回调函数)
    // 如果没有按下状态的图，第二个参数可以填与第一个相同，或者 create("img.png", CC_CALLBACK...)

    // start按钮
    auto btn1 = MenuItemImage::create(
        "UI/start.png",   // 正常状态图片
        "UI/start-after.png", // 按下状态图片
        CC_CALLBACK_1(StartScene::onButton1Click, this));
    // 设置位置
    if (btn1) {
        btn1->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.7 + origin.y));
    }

    // how-to-play button(游戏规则)
    auto btn2 = MenuItemImage::create(
        "UI/htp-button.png",
        "UI/htp-button-after.png",
        CC_CALLBACK_1(StartScene::onButton2Click, this));
    if(btn2)btn2->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.5 + origin.y));

    // score
    auto btn3 = MenuItemImage::create(
        "UI/score.png",
        "UI/score-after.png",
        CC_CALLBACK_1(StartScene::onButton3Click, this));
    if(btn3)btn3->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.3 + origin.y));

    // 4. 创建菜单容器并添加按钮
    // MenuItem 必须放入 Menu 中才能响应点击
    auto menu = Menu::create(btn1, btn2, btn3, NULL);

    // ！！！重要：如果不设置这一行，Cocos默认会把Menu放在(0,0)，但里面的按钮位置会变成相对坐标
    // 设置为 Vec2::ZERO 后，按钮设置的 setPosition 才是基于屏幕的绝对坐标
    menu->setPosition(Vec2::ZERO);

    this->addChild(menu, 1); // 层级为1，在背景之上

    return true;
}

// 5. 回调函数实现 (跳转逻辑)
void StartScene::onButton1Click(Ref* pSender)
{
    auto scene = moduleScene::createScene();
    Director::getInstance()->replaceScene(
        TransitionFade::create(0.5, scene)
    );
}

void StartScene::onButton2Click(Ref* pSender)
{
    CCLOG("Button 2 clicked - Jumping to Scene B");
    auto scene = HowToPlayScene::createScene();
    Director::getInstance()->replaceScene(
        TransitionFade::create(0.5, scene)
    );
}

void StartScene::onButton3Click(Ref* pSender)
{
    CCLOG("Button 3 clicked - Jumping to Scene C");
    // auto scene = AboutScene::createScene();
    // Director::getInstance()->replaceScene(scene);
}