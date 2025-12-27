#include "GameBackground.h"
// 开始场景头文件
#include "StartScene.h" 
#include "GameScene.h" // 访问静态变量
#include "AudioEngine.h"
#include"SimpleAudioEngine.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace cocos2d::experimental;

cocos2d::Scene* GameBackground::createScene()
{
    auto scene = Scene::create();
    auto layer = GameBackground::create();
    scene->addChild(layer);
    return scene;
}

bool GameBackground::init()
{
    if (!Layer::init())
    {
        return false;
    }

    // 初始化数据
    _currentLevel = 1;
    _timeLeft = 3 * 60; // 3分钟 = 180秒
    _isSoundOn = true;
    _isPaused = false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 添加背景 (2048*1536)
    auto bg = Sprite::create("UI/gamebackground.png");
    // 注意：假设你的设计分辨率已适配好，这里直接居中或设为原点
    if (bg == nullptr) {
        CCLOG("Error: Background image not found! Check file name and path.");
    }
    else {
        bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        this->addChild(bg, 0);
    }

    // 2. 初始化各个模块
    initGrid();
    initSideBar();
    initButtons();

    // 3. 开启倒计时 (每1秒调用一次 updateTimer)
    this->schedule(CC_SCHEDULE_SELECTOR(GameBackground::updateTimer), 1.0f);

    return true;
}

void GameBackground::initGrid()
{
    // 网格起始点 (669, 1393)
    // 假设 Y 轴是向下排列 (因为 1393 接近顶部)
    float startX = 668.5f;
    float startY = 1393.0f;
    float gap = 105.7f; // 间隔

    for (int row = 0; row < 13; ++row) {
        for (int col = 0; col < 13; ++col) {
            // 添加网格的方块(92x92)
            auto block = Sprite::create("UI/cell-button.png"); 
            if (block) {
                // 计算坐标
                float x = startX + col * gap;
                float y = startY - row * gap; // 向下延伸

                block->setPosition(Vec2(x, y));
                this->addChild(block, 1);
            }
        }
    }
}

void GameBackground::initSideBar()
{
    // 1. 左栏 4个 64x64 图片

    auto player1 = Sprite::create("player/player1.png");
    //player1->setScale(2.5f);
    player1->setPosition(Vec2(153.4f, 1242.3f));
    this->addChild(player1, 2);
    
    auto player2 = Sprite::create("player/player2.png");
    //player2->setScale(2.5f);
    player2->setPosition(Vec2(153.4f, 1017.9f));
    this->addChild(player2, 2);

    auto player3 = Sprite::create("player/player3.png");
    //player3->setScale(2.5f);
    player3->setPosition(Vec2(153.4f, 793.1f));
    this->addChild(player3, 2);

    auto player4 = Sprite::create("player/player4.png");
    //player4->setScale(2.5f);
    player4->setPosition(Vec2(153.4f, 567.9f));
    this->addChild(player4, 2);

    float yPos[] = { 1242.3f, 1017.9f, 793.1f, 567.9f };
    _scoreLabels.clear();
    _itemCountLabels.clear();

    for (int i = 0; i < 4; ++i)
    {
        // 1. 积分 Label (显示在头像右侧上方)
        auto scoreLabel = Label::createWithSystemFont("0", "Arial", 50);
        scoreLabel->setAnchorPoint(Vec2(0, 0.5f)); // 左对齐
        scoreLabel->setPosition(Vec2(450, yPos[i] + 41.8f)); // 头像右侧
        scoreLabel->setColor(Color3B::YELLOW);
        this->addChild(scoreLabel, 2);
        _scoreLabels.push_back(scoreLabel);

        // 2. 道具数 Label (显示在积分下方)
        auto itemLabel = Label::createWithSystemFont("0", "Arial", 50);
        itemLabel->setAnchorPoint(Vec2(0, 0.5f));
        itemLabel->setPosition(Vec2(450, yPos[i] - 41.8f));
        itemLabel->setColor(Color3B::GREEN);
        this->addChild(itemLabel, 2);
        _itemCountLabels.push_back(itemLabel);
    }


    // 2. Level 数字显示 (129, 291)
    // 使用系统字体，你也可以换成 TTF
    _levelLabel = Label::createWithSystemFont(std::to_string(_currentLevel), "Arial", 55);
    _levelLabel->setPosition(Vec2(129, 291));
    _levelLabel->setColor(Color3B::WHITE);
    this->addChild(_levelLabel, 2);

    // 3. 倒计时显示 (406, 291)
    _timerLabel = Label::createWithSystemFont("03:00", "Arial", 55);
    _timerLabel->setPosition(Vec2(406, 291));
    _timerLabel->setColor(Color3B::WHITE);
    this->addChild(_timerLabel, 2);
}

void GameBackground::initButtons()
{  
    // --- Sound Button (296, 168) ---
    _isSoundOn = GameScene::s_isAudioOn;

    std::string normalImg = _isSoundOn ? "UI/soundon.png" : "UI/soundoff.png";
    std::string pressImg = _isSoundOn ? "UI/soundon-after.png" : "UI/soundoff-after.png";
    // Normal: soundon.png, Pressed: soundon-after.png
    _soundBtn = Button::create(normalImg, pressImg);
    _soundBtn->setPosition(Vec2(296.05f, 168));
    _soundBtn->addTouchEventListener(CC_CALLBACK_2(GameBackground::onSoundEvent, this));
    this->addChild(_soundBtn, 2);

    // --- Return Button (153.6, 61) ---
    _returnBtn = Button::create("UI/gamereturn.png", "UI/gamereturn-after.png");
    _returnBtn->setPosition(Vec2(153.6f, 61));
    _returnBtn->addTouchEventListener(CC_CALLBACK_2(GameBackground::onReturnEvent, this));
    this->addChild(_returnBtn, 2);

    // --- Pause Button (438, 61) ---
    _pauseBtn = Button::create("UI/pause.png", "UI/pause-after.png");
    _pauseBtn->setPosition(Vec2(438, 61));
    _pauseBtn->addTouchEventListener(CC_CALLBACK_2(GameBackground::onPauseEvent, this));
    this->addChild(_pauseBtn, 2);
}

// --- 逻辑处理 ---

void GameBackground::updateTimer(float dt)
{
    if (_isPaused) return;
    if (_timeLeft > 0) {
        _timeLeft--;

        // 格式化时间 mm:ss
        int min = _timeLeft / 60;
        int sec = _timeLeft % 60;
        char timeStr[10];
        sprintf(timeStr, "%02d:%02d", min, sec);

        _timerLabel->setString(timeStr);
    }
    else {
        // 时间到，处理游戏结束逻辑
        this->unschedule(CC_SCHEDULE_SELECTOR(GameBackground::updateTimer));
    }
}

void GameBackground::onLevelWin()
{
    // 外部调用此函数增加等级
    _currentLevel++;
    if (_levelLabel) {
        _levelLabel->setString(std::to_string(_currentLevel));
    }
    // 可以在这里重置时间或添加其他获胜逻辑
}

void GameBackground::onSoundEvent(Ref* sender, Widget::TouchEventType type)
{
    // ui::Button 内部会自动处理按下显示 "Selected Image" 的逻辑
    // 我们只需要在手指抬起(Ended)时，切换按钮的这套图片素材即可

    if (type == Widget::TouchEventType::ENDED) {
        _isSoundOn = !_isSoundOn; // 切换状态

        GameScene::s_isAudioOn = _isSoundOn;

        if (_isSoundOn) {
            // 切换回开启状态
            // 此时：鼠标按下显示 soundon-after，抬起(常态)显示 soundon
            _soundBtn->loadTextures("UI/soundon.png", "UI/soundon-after.png");
            // 开启音乐
            AudioEngine::resumeAll();
            // 特殊处理：如果背景音乐之前没播（比如一开始就是静音进入游戏的），现在补播
            if (GameScene::s_gameAudioID == AudioEngine::INVALID_AUDIO_ID) {
                GameScene::s_gameAudioID = AudioEngine::play2d("Sound/GameBackgroundSound.mp3", true, 0.5f);
            }
            // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
        }
        else {
            // 切换到关闭状态
            // 此时：鼠标按下显示 soundoff-after，抬起(常态)显示 soundoff
            _soundBtn->loadTextures("UI/soundoff.png", "UI/soundoff-after.png");
            // 这里可以添加关闭音乐的代码
            // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
            AudioEngine::pauseAll();
        }
    }
}

void GameBackground::onReturnEvent(Ref* sender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED) {
        // 跳转回 StartScene
        // 确保你已经创建了 StartScene 类
        auto scene = StartScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }
}

void GameBackground::onPauseEvent(Ref* sender, Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED) {
        _isPaused = !_isPaused; // 切换暂停状态

        if (_isPaused) {
            // 暂停倒计时
            //this->pause(); // 暂停当前层的 update 和 action
            _pauseBtn->loadTextures("UI/resume.png", "UI/resume-after.png");
            // 如果你有 Player 节点，你需要通知它们停止动作
            // 例如：Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("GAME_PAUSE");
        }
        else {
            // 恢复
            //this->resume();
            _pauseBtn->loadTextures("UI/pause.png", "UI/pause-after.png");
            //_pauseBtn = Button::create("UI/pause.png", "UI/pause-after.png");
            // Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("GAME_RESUME");
        }
    }
}

// 实现更新接口
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