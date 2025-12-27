#include "SelectDifficultyScene.h"
#include "GameScene.h"

USING_NS_CC;

Scene* SelectDifficultyScene::createScene(GameMode mode, int p1Face)
{
    auto scene = SelectDifficultyScene::create();
    if (scene)
    {
        scene->setData(mode, p1Face);
    }
    return scene;
}

Scene* SelectDifficultyScene::createScene()
{
    return SelectDifficultyScene::create();
}

void SelectDifficultyScene::setData(GameMode mode, int p1Face)
{
    _mode = mode;
    _p1Face = p1Face;
}

bool SelectDifficultyScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 背景 selectdifficulty.png
    auto bg = Sprite::create("UI/selectdifficulty.png");
    if (bg) {
        bg->setPosition(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
        this->addChild(bg, 0);
    }

    // 2. Easy Mode 按钮
    auto easyItem = MenuItemImage::create(
        "UI/easymode.png",
        "UI/easymode-after.png",
        CC_CALLBACK_1(SelectDifficultyScene::onEasyMode, this)
    );
    // 假设位置在屏幕中间偏上，你可以根据实际图片调整
    easyItem->setPosition(Vec2(visibleSize.width * 0.75 , visibleSize.height * 0.35 ));

    // 3. Hard Mode 按钮
    auto hardItem = MenuItemImage::create(
        "UI/hardmode.png",
        "UI/hardmode-after.png",
        CC_CALLBACK_1(SelectDifficultyScene::onHardMode, this)
    );
    // 假设位置在屏幕中间偏下
    hardItem->setPosition(Vec2(visibleSize.width * 0.75 , visibleSize.height * 0.65 ));

    // 4. 菜单
    auto menu = Menu::create(easyItem, hardItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    return true;
}

void SelectDifficultyScene::onEasyMode(Ref* sender)
{
    CCLOG("Selected Easy Mode");
    startGame(GameDifficulty::EASY);
}

void SelectDifficultyScene::onHardMode(Ref* sender)
{
    CCLOG("Selected Hard Mode");
    startGame(GameDifficulty::HARD);
}

void SelectDifficultyScene::startGame(GameDifficulty diff)
{
    // ? 跳转到 GameScene，并将难度参数传递过去
    // 我们需要修改 GameScene 的 createWithMode 接口来接收 diff
    auto scene = GameScene::createWithMode(_mode, _p1Face, 0, diff); // 0 是 p2Face(单人不需)
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}