#include "GameBackground.h"
#include "StartScene.h" 
#include "GameScene.h" 
#include "AudioEngine.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d::experimental;

// --- 生命周期 ---

Scene* GameBackground::createScene()
{
    auto scene = Scene::create();
    auto layer = GameBackground::create();
    scene->addChild(layer);
    return scene;
}

bool GameBackground::init()
{
    if (!Layer::init()) return false;

    // 1. 数据初始化
    _currentLevel = 1;
    _timeLeft = 3 * 60; // 180秒
    _isSoundOn = true;
    _isPaused = false;

    // 2. 背景绘制
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto bg = Sprite::create("UI/gamebackground.png");
    if (bg) {
        bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(bg, 0);
    }
    else {
        CCLOG("Error: Background image missing!");
    }

    // 3. 构建界面模块
    initGrid();
    initSideBar();
    initButtons();

    // 4. 启动倒计时 (1秒间隔)
    this->schedule(CC_SCHEDULE_SELECTOR(GameBackground::updateTimer), 1.0f);

    return true;
}

// --- UI布局初始化 ---

void GameBackground::initGrid()
{
    // 网格布局参数：起始点(668.5, 1393) 向下排列
    float startX = 668.5f;
    float startY = 1393.0f;
    float gap = 105.7f;

    for (int row = 0; row < 13; ++row) {
        for (int col = 0; col < 13; ++col) {
            auto block = Sprite::create("UI/cell-button.png");
            if (block) {
                float x = startX + col * gap;
                float y = startY - row * gap;
                block->setPosition(Vec2(x, y));
                this->addChild(block, 1);
            }
        }
    }
}

void GameBackground::initSideBar()
{
    // 1. 创建左侧玩家头像 (硬编码坐标)
    struct PlayerPos { float x; float y; };
    PlayerPos positions[] = {
        {153.4f, 1242.3f}, {153.4f, 1017.9f}, {153.4f, 793.1f}, {153.4f, 567.9f}
    };

    for (int i = 0; i < 4; ++i) {
        auto p = Sprite::create(StringUtils::format("player/player%d.png", i + 1));
        p->setPosition(Vec2(positions[i].x, positions[i].y));
        this->addChild(p, 2);
    }

    // 2. 初始化积分与道具 Label
    _scoreLabels.clear();
    _itemCountLabels.clear();

    for (int i = 0; i < 4; ++i)
    {
        // 积分 (黄色)
        auto scoreLabel = Label::createWithSystemFont("0", "Arial", 50);
        scoreLabel->setAnchorPoint(Vec2(0, 0.5f));
        scoreLabel->setPosition(Vec2(450, positions[i].y + 41.8f));
        scoreLabel->setColor(Color3B::YELLOW);
        this->addChild(scoreLabel, 2);
        _scoreLabels.push_back(scoreLabel);

        // 道具数 (绿色)
        auto itemLabel = Label::createWithSystemFont("0", "Arial", 50);
        itemLabel->setAnchorPoint(Vec2(0, 0.5f));
        itemLabel->setPosition(Vec2(450, positions[i].y - 41.8f));
        itemLabel->setColor(Color3B::GREEN);
        this->addChild(itemLabel, 2);
        _itemCountLabels.push_back(itemLabel);
    }

    // 3. 顶部信息栏 (Level & Timer)
    _levelLabel = Label::createWithSystemFont(std::to_string(_currentLevel), "Arial", 55);
    _levelLabel->setPosition(Vec2(129, 291));
    _levelLabel->setColor(Color3B::WHITE);
    this->addChild(_levelLabel, 2);

    _timerLabel = Label::createWithSystemFont("03:00", "Arial", 55);
    _timerLabel->setPosition(Vec2(406, 291));
    _timerLabel->setColor(Color3B::WHITE);
    this->addChild(_timerLabel, 2);
}

void GameBackground::initButtons()
{
    // 1. 音效按钮 (同步全局静态状态)
    _isSoundOn = GameScene::s_isAudioOn;
    std::string normalImg = _isSoundOn ? "UI/soundon.png" : "UI/soundoff.png";
    std::string pressImg = _isSoundOn ? "UI/soundon-after.png" : "UI/soundoff-after.png";

    _soundBtn = Button::create(normalImg, pressImg);
    _soundBtn->setPosition(Vec2(296.05f, 168));
    _soundBtn->addTouchEventListener(CC_CALLBACK_2(GameBackground::onSoundEvent, this));
    this->addChild(_soundBtn, 2);

    // 2. 返回按钮
    _returnBtn = Button::create("UI/gamereturn.png", "UI/gamereturn-after.png");
    _returnBtn->setPosition(Vec2(153.6f, 61));
    _returnBtn->addTouchEventListener(CC_CALLBACK_2(GameBackground::onReturnEvent, this));
    this->addChild(_returnBtn, 2);

    // 3. 暂停按钮
    _pauseBtn = Button::create("UI/pause.png", "UI/pause-after.png");
    _pauseBtn->setPosition(Vec2(438, 61));
    _pauseBtn->addTouchEventListener(CC_CALLBACK_2(GameBackground::onPauseEvent, this));
    this->addChild(_pauseBtn, 2);
}

// --- 核心业务逻辑 ---

void GameBackground::updateTimer(float dt)
{
    if (_isPaused) return;

    if (_timeLeft > 0) {
        _timeLeft--;

        // 格式化 mm:ss
        int min = _timeLeft / 60;
        int sec = _timeLeft % 60;
        char timeStr[10];
        sprintf(timeStr, "%02d:%02d", min, sec);

        _timerLabel->setString(timeStr);
    }
    else {
        // 超时逻辑
        this->unschedule(CC_SCHEDULE_SELECTOR(GameBackground::updateTimer));
    }
}

void GameBackground::updatePlayerStat(int playerIndex, int score, int itemCount)
{
    if (playerIndex < 0 || playerIndex >= _scoreLabels.size()) return;

    if (_scoreLabels[playerIndex]) {
        _scoreLabels[playerIndex]->setString(StringUtils::format("%d", score));
    }
    if (_itemCountLabels[playerIndex]) {
        _itemCountLabels[playerIndex]->setString(StringUtils::format("%d", itemCount));
    }
}

void GameBackground::onLevelWin()
{
    _currentLevel++;
    if (_levelLabel) {
        _levelLabel->setString(std::to_string(_currentLevel));
    }
}

// --- 事件回调 ---

void GameBackground::onSoundEvent(Ref* sender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED) {
        _isSoundOn = !_isSoundOn;
        GameScene::s_isAudioOn = _isSoundOn;

        if (_isSoundOn) {
            // 开启：切换贴图并恢复音频
            _soundBtn->loadTextures("UI/soundon.png", "UI/soundon-after.png");
            AudioEngine::resumeAll();

            // 补救逻辑：若背景音未初始化则播放
            if (GameScene::s_gameAudioID == AudioEngine::INVALID_AUDIO_ID) {
                GameScene::s_gameAudioID = AudioEngine::play2d("Sound/GameBackgroundSound.mp3", true, 0.5f);
            }
        }
        else {
            // 关闭：切换贴图并暂停音频
            _soundBtn->loadTextures("UI/soundoff.png", "UI/soundoff-after.png");
            AudioEngine::pauseAll();
        }
    }
}

void GameBackground::onReturnEvent(Ref* sender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED) {
        auto scene = StartScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }
}

void GameBackground::onPauseEvent(Ref* sender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED) {
        _isPaused = !_isPaused;

        // 切换暂停/恢复图标，逻辑由 updateTimer 中的标志位控制
        if (_isPaused) {
            _pauseBtn->loadTextures("UI/resume.png", "UI/resume-after.png");
        }
        else {
            _pauseBtn->loadTextures("UI/pause.png", "UI/pause-after.png");
        }
    }
}