#include "GameOverLayer.h"
#include "GameScene.h"
#include "StartScene.h" 
#include"AudioEngine.h"

USING_NS_CC;
using namespace cocos2d::experimental;

GameOverLayer* GameOverLayer::create(bool isWin, GameMode mode, int p1Face, int p2Face)
{
    GameOverLayer* pRet = new(std::nothrow) GameOverLayer();
    if (pRet && pRet->init(isWin, mode, p1Face, p2Face))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool GameOverLayer::init(bool isWin, GameMode mode, int p1Face, int p2Face)
{
    if (!Layer::init()) return false;

    // --- 播放音效 ---
    // 只有当全局音效开启时
    if (GameScene::s_isAudioOn) {
        if (isWin) {
            AudioEngine::play2d("Sound/wow.mp3", false, 1.0f);
        }
        else {
            AudioEngine::play2d("Sound/fail.mp3", false, 1.0f);
        }
    }

    // 1. 保存参数（为了重启）
    _mode = mode;
    _p1Face = p1Face;
    _p2Face = p2Face;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 2. 吞噬触摸（防止点穿到游戏层）
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 3. 添加半透明黑色遮罩（可选，让背景变暗）
    auto layerColor = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(layerColor);

    // 4. 显示输赢图片
    std::string bgImage = isWin ? "UI/gamewin.png" : "UI/gamelose.png";
    auto bg = Sprite::create(bgImage);
    if (bg)
    {
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        this->addChild(bg);
    }
    else
    {
        CCLOG("Error: %s not found!", bgImage.c_str());
    }

    // 5. 创建按钮
    // 重新开始按钮
    auto btnRestart = MenuItemImage::create(
        "UI/restart.png",       // 正常图
        "UI/restart_after.png", // 按下图
        CC_CALLBACK_1(GameOverLayer::onRestart, this));

    // 返回按钮
    auto btnReturn = MenuItemImage::create(
        "UI/gamereturn.png",
        "UI/gamereturn_after.png",
        CC_CALLBACK_1(GameOverLayer::onReturn, this));

    // 调整按钮位置 (根据你的 UI 图片设计调整坐标)
    // 假设背景图中心在屏幕中心，按钮放在背景图下方或内部
    if (btnRestart) btnRestart->setPosition(Vec2(776,546.95f));
    if (btnReturn)  btnReturn->setPosition(Vec2(1272,546.95f));

    auto menu = Menu::create(btnRestart, btnReturn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);

    return true;
}

// --- 重新开始 (带参数) ---
void GameOverLayer::onRestart(Ref* sender)
{
    CCLOG("Restarting Game: Mode=%d, P1=%d, P2=%d", (int)_mode, _p1Face, _p2Face);

    // ⭐ 关键：使用保存的参数重新创建 GameScene
    auto scene = GameScene::createWithMode(_mode, _p1Face, _p2Face);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// --- 返回主菜单 ---
void GameOverLayer::onReturn(Ref* sender)
{
    // 跳转回 StartScene
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}