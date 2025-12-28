#include "SelectDifficultyScene.h"
#include "GameScene.h"

USING_NS_CC;

// --- 生命周期与工厂 ---

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

bool SelectDifficultyScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 初始化背景
    auto bg = Sprite::create("UI/selectdifficulty.png");
    if (bg) {
        bg->setPosition(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
        this->addChild(bg, 0);
    }

    // 2. 初始化难度按钮
    // Easy Mode
    auto easyItem = MenuItemImage::create(
        "UI/easymode.png", "UI/easymode-after.png",
        CC_CALLBACK_1(SelectDifficultyScene::onEasyMode, this)
    );
    easyItem->setPosition(Vec2(visibleSize.width * 0.75, visibleSize.height * 0.35));

    // Hard Mode
    auto hardItem = MenuItemImage::create(
        "UI/hardmode.png", "UI/hardmode-after.png",
        CC_CALLBACK_1(SelectDifficultyScene::onHardMode, this)
    );
    hardItem->setPosition(Vec2(visibleSize.width * 0.75, visibleSize.height * 0.65));

    // 3. 添加菜单容器
    auto menu = Menu::create(easyItem, hardItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    return true;
}

// --- 数据注入 ---

void SelectDifficultyScene::setData(GameMode mode, int p1Face)
{
    _mode = mode;
    _p1Face = p1Face;
}

// --- 交互回调 ---

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

// --- 转场逻辑 ---

void SelectDifficultyScene::startGame(GameDifficulty diff)
{
    // 将积累的参数 (模式、P1角色、难度) 统一传递给核心游戏场景
    // 注：单人模式不需要 P2Face，传 0 即可
    auto scene = GameScene::createWithMode(_mode, _p1Face, 0, diff);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}