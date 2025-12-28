#include "module.h"
#include "StartScene.h"
#include "SelectScene.h"
#include "SelectScene_2Player.h"

USING_NS_CC;

// --- 生命周期 ---

Scene* moduleScene::createScene()
{
    return moduleScene::create();
}

bool moduleScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 初始化背景
    auto bg = Sprite::create("UI/startBackground.png");
    if (bg) {
        bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(bg, 0);
    }
    else {
        CCLOG("Error: Background image not found!");
    }

    // 2. 初始化菜单按钮
    // 统一 X 坐标
    float btnX = visibleSize.width * 0.75 + origin.x;

    // 单人模式 (One Player)
    auto btnSingle = MenuItemImage::create(
        "UI/oneplayer.png", "UI/oneplayer-after.png",
        CC_CALLBACK_1(moduleScene::onButton1Click, this));
    btnSingle->setPosition(Vec2(btnX, visibleSize.height * 0.8 + origin.y));

    // 双人模式 (Two Players)
    auto btnMulti = MenuItemImage::create(
        "UI/twoplayers.png", "UI/twoplayers-after.png",
        CC_CALLBACK_1(moduleScene::onButton2Click, this));
    btnMulti->setPosition(Vec2(btnX, visibleSize.height * 0.6 + origin.y));

    // 在线/迷雾模式 (Online -> Fog)
    auto btnOnline = MenuItemImage::create(
        "UI/online.png", "UI/online-after.png",
        CC_CALLBACK_1(moduleScene::onButton3Click, this));
    btnOnline->setPosition(Vec2(btnX, visibleSize.height * 0.4 + origin.y));

    // 返回 (Return)
    auto btnReturn = MenuItemImage::create(
        "UI/return.png", "UI/return-after.png",
        CC_CALLBACK_1(moduleScene::onButton4Click, this));
    btnReturn->setPosition(Vec2(btnX, visibleSize.height * 0.2 + origin.y));

    // 3. 构建菜单容器
    auto menu = Menu::create(btnSingle, btnMulti, btnOnline, btnReturn, nullptr);
    menu->setPosition(Vec2::ZERO); // 必须归零，否则内部按钮坐标会偏移
    this->addChild(menu, 1);

    return true;
}

// --- 交互回调 ---

void moduleScene::onButton1Click(Ref* pSender)
{
    // 跳转：单人模式 -> 选人界面
    auto scene = SelectScene::createScene(GameMode::SINGLE);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void moduleScene::onButton2Click(Ref* pSender)
{
    // 跳转：双人模式 -> 专属双人选人界面
    auto scene = SelectScene_2Player::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void moduleScene::onButton3Click(Ref* pSender)
{
    // 跳转：迷雾模式 (原Online按钮) -> 选人界面
    auto scene = SelectScene::createScene(GameMode::FOG);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void moduleScene::onButton4Click(Ref* pSender)
{
    // 跳转：返回主菜单
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}