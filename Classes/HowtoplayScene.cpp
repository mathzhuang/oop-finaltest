#include "HowToPlayScene.h"
#include "StartScene.h"

USING_NS_CC;

// --- 生命周期 ---

Scene* HowToPlayScene::createScene()
{
    return HowToPlayScene::create();
}

bool HowToPlayScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 初始化背景
    auto bg = Sprite::create("UI/howtoplay.png");
    if (bg) {
        bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(bg, 0);
    }
    else {
        CCLOG("Error: Background image not found!");
    }

    // 2. 构建返回按钮
    auto returnBtn = MenuItemImage::create(
        "UI/gamereturn.png",
        "UI/gamereturn-after.png",
        CC_CALLBACK_1(HowToPlayScene::onButtonClick, this));

    if (returnBtn) {
        returnBtn->setPosition(Vec2(1710.80f, 118.35f));
    }

    // 3. 添加菜单容器
    auto menu = Menu::create(returnBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    return true;
}

// --- 交互回调 ---

void HowToPlayScene::onButtonClick(Ref* pSender)
{
    // 切换回开始场景
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}