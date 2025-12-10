#include "HowToPlayScene.h"
// 需要跳转的目标场景头文件
#include "StartScene.h"


USING_NS_CC;

Scene* HowToPlayScene::createScene()
{
    return HowToPlayScene::create();
}

bool HowToPlayScene::init()
{
    // 1. 父类初始化失败则返回 false
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 2. 添加背景 (2048*1537)
    auto sprite = Sprite::create("UI/howtoplay.png");

    // 安全检查：如果图片没找到，打印错误
    if (sprite == nullptr) {
        CCLOG("Error: Background image not found! Check file name and path.");
    }
    else {
        sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(sprite, 0);
    }

    // 3. 创建按钮

    // 参数说明：create(正常状态图片, 按下状态图片, 回调函数)
    // 如果没有按下状态的图，第二个参数可以填与第一个相同，或者 create("img.png", CC_CALLBACK...)

    // return按钮
    auto btn1 = MenuItemImage::create(
        "UI/gamereturn.png",   // 正常状态图片
        "UI/gamereturn-after.png", // 按下状态图片
        CC_CALLBACK_1(HowToPlayScene::onButtonClick, this));
    // 设置位置
    if (btn1) {
        btn1->setPosition(Vec2(1710.80f, 118.35f));
    }


    // 4. 创建菜单容器并添加按钮
    // MenuItem 必须放入 Menu 中才能响应点击
    auto menu = Menu::create(btn1, NULL);

    menu->setPosition(Vec2::ZERO);

    this->addChild(menu, 1); // 层级为1，在背景之上

    return true;
}

// 5. 回调函数实现 (跳转逻辑)
void HowToPlayScene::onButtonClick(Ref* pSender)
{
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(
        TransitionFade::create(0.5, scene)
    );
}