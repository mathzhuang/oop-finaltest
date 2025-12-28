#include "GameOverLayer.h"
#include "GameScene.h"
#include "StartScene.h" 
#include "HighScoresScene.h"
#include "AudioEngine.h"

USING_NS_CC;
using namespace cocos2d::experimental;

// --- 工厂方法 ---

GameOverLayer* GameOverLayer::create(bool isWin, GameMode mode, int p1Face, int p2Face, int score)
{
    GameOverLayer* pRet = new(std::nothrow) GameOverLayer();
    if (pRet && pRet->init(isWin, mode, p1Face, p2Face, score))
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

// --- 初始化核心 ---

bool GameOverLayer::init(bool isWin, GameMode mode, int p1Face, int p2Face, int score)
{
    if (!Layer::init()) return false;

    // 1. 播放结算音效 (依赖全局开关)
    if (GameScene::s_isAudioOn) {
        std::string sound = isWin ? "Sound/wow.mp3" : "Sound/fail.mp3";
        AudioEngine::play2d(sound, false, 1.0f);
    }

    // 2. 数据处理
    // 缓存参数以便“重新开始”
    _mode = mode;
    _p1Face = p1Face;
    _p2Face = p2Face;

    // 记录分数 (仅正分计入排行榜)
    if (score > 0) {
        HighScoresScene::saveScore(score);
        CCLOG("Score %d saved to HighScores", score);
    }

    // 3. 构建 UI
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 遮罩层：吞噬触摸事件，防止点击穿透到游戏层
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 半透明黑色背景
    this->addChild(LayerColor::create(Color4B(0, 0, 0, 150)));

    // 胜负背景图
    std::string bgImage = isWin ? "UI/gamewin.png" : "UI/gamelose.png";
    auto bg = Sprite::create(bgImage);
    if (bg) {
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        this->addChild(bg);
    }

    // 分数显示
    auto scoreLabel = Label::createWithSystemFont("Your Score: " + std::to_string(score), "Arial", 50);
    scoreLabel->setPosition(visibleSize.width / 2, 400);
    scoreLabel->setColor(Color3B::YELLOW);
    scoreLabel->enableOutline(Color4B::BLACK, 2);
    this->addChild(scoreLabel, 10);

    // 4. 功能按钮
    auto btnRestart = MenuItemImage::create("UI/restart.png", "UI/restart_after.png",
        CC_CALLBACK_1(GameOverLayer::onRestart, this));

    auto btnReturn = MenuItemImage::create("UI/gamereturn.png", "UI/gamereturn_after.png",
        CC_CALLBACK_1(GameOverLayer::onReturn, this));

    if (btnRestart) btnRestart->setPosition(Vec2(776, 546.95f));
    if (btnReturn)  btnReturn->setPosition(Vec2(1272, 546.95f));

    auto menu = Menu::create(btnRestart, btnReturn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);

    return true;
}

// --- 交互回调 ---

void GameOverLayer::onRestart(Ref* sender)
{
    CCLOG("Restarting Game: Mode=%d", (int)_mode);

    // 使用缓存的参数重建游戏场景
    auto scene = GameScene::createWithMode(_mode, _p1Face, _p2Face);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void GameOverLayer::onReturn(Ref* sender)
{
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}