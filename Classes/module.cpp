#include "classes\module.h"
// 需要跳转的目标场景头文件
#include "GameScene.h"
#include"StartScene.h"
#include"GameBackground.h"
#include"SelectScene.h"

USING_NS_CC;

Scene* moduleScene::createScene()
{
    return moduleScene::create();
}

bool moduleScene::init()
{
    // 1. 父类初始化失败则返回 false
    if (!Scene::init())
    {
        return false;
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

    // one player botton
    auto btn1 = MenuItemImage::create(
        "UI/oneplayer.png",   // 正常状态图片
        "UI/oneplayer-after.png", // 按下状态图片
        CC_CALLBACK_1(moduleScene::onButton1Click, this));
    // 设置位置
    if (btn1) {
        btn1->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.8 + origin.y));
    }

    // two players button
    auto btn2 = MenuItemImage::create(
        "UI/twoplayers.png",
        "UI/twoplayers-after.png",
        CC_CALLBACK_1(moduleScene::onButton2Click, this));
    if (btn2)btn2->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.6 + origin.y));

    // score
    auto btn3 = MenuItemImage::create(
        "UI/online.png",
        "UI/online-after.png",
        CC_CALLBACK_1(moduleScene::onButton3Click, this));
    if (btn3)btn3->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.4 + origin.y));

    // return
    auto btn4 = MenuItemImage::create(
        "UI/return.png",
        "UI/return-after.png",
        CC_CALLBACK_1(moduleScene::onButton4Click, this));
    if (btn4)btn4->setPosition(Vec2(visibleSize.width * 0.75 + origin.x, visibleSize.height * 0.2 + origin.y));

    // 4. 创建菜单容器并添加按钮
    // MenuItem 必须放入 Menu 中才能响应点击
    auto menu = Menu::create(btn1, btn2, btn3, btn4, NULL);

    // ！！！重要：如果不设置这一行，Cocos默认会把Menu放在(0,0)，但里面的按钮位置会变成相对坐标
    // 设置为 Vec2::ZERO 后，按钮设置的 setPosition 才是基于屏幕的绝对坐标
    menu->setPosition(Vec2::ZERO);

    this->addChild(menu, 1); // 层级为1，在背景之上

    return true;
}

// 5. 回调函数实现 (跳转逻辑)
void moduleScene::onButton1Click(Ref* pSender)
{
    CCLOG("One player Button clicked - Jumping to Game Scene.");
    // 单人模式
    auto scene = SelectScene::createScene(GameMode::SINGLE);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void moduleScene::onButton2Click(Ref* pSender)
{
    CCLOG("Two players Button clicked - Jumping to Select Scene.");
    // 双人模式，先进入角色选择场景
    auto scene = SelectScene::createScene(GameMode::LOCAL_2P);
    // 在角色选择后，将选择的信息传递给 GameScene
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void moduleScene::onButton3Click(Ref* pSender)
{
    CCLOG("Online Button clicked - Jumping to Game Scene (Online Mode).");
    // 在线模式（未来扩展）
    auto scene = GameScene::createWithMode(GameMode::ONLINE);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void moduleScene::onButton4Click(Ref* pSender)
{
    CCLOG("Return button clicked - Jumping to Start Scene.");
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}
