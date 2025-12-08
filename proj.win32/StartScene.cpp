#include "StartScene.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "GameScene.h"
//#include "SettingScene.h"

USING_NS_CC;

// 创建场景
Scene* StartScene::createScene()
{
    auto scene = Scene::create();
    auto layer = StartScene::create();
    scene->addChild(layer);
    return scene;
}

// 初始化
bool StartScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // 设置设计分辨率
    auto glview = Director::getInstance()->getOpenGLView();
    glview->setDesignResolutionSize(DESIGN_WIDTH, DESIGN_HEIGHT,
        ResolutionPolicy::SHOW_ALL);

    // 初始化UI
    initUI();

    return true;
}

// 初始化UI
void StartScene::initUI()
{
    createBackground();
    createImageButtons();
}

// 创建背景
void StartScene::createBackground()
{
    //// 创建纯色背景
    //auto background = LayerColor::create(Color4B(45, 45, 60, 255),
    //    DESIGN_WIDTH, DESIGN_HEIGHT);
    //background->setPosition(Vec2(0, 0));
    //this->addChild(background, 0);

    // 或者使用图片背景（如果有）
    
    auto bgSprite = Sprite::create("Resources/UI/startBackground.png");
    if (bgSprite)
    {
        bgSprite->setPosition(Vec2(DESIGN_WIDTH/2, DESIGN_HEIGHT/2));
        bgSprite->setContentSize(Size(DESIGN_WIDTH, DESIGN_HEIGHT));
        this->addChild(bgSprite, 0);
    }
    

    // 添加游戏标题（可选）
    //auto title = Label::createWithTTF("我的游戏", "fonts/arial.ttf", 120);
    //if (title)
    //{
    //    title->setPosition(Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT * 0.8f));
    //    title->setTextColor(Color4B(255, 215, 0, 255)); // 金色
    //    title->enableShadow(Color4B::BLACK, Size(8, -8), 10);
    //    title->enableOutline(Color4B(139, 0, 0, 255), 5); // 深红色描边
    //    this->addChild(title, 1);
    //}
}

// 创建图片按钮
void StartScene::createImageButtons()
{
    // 按钮1: 开始游戏按钮
    _startButton = createImageButton(
        "Resources/UI/start.png",          // 正常状态图片
        "Resources/UI/start-after.png",         // 按下状态图片
        Vec2(DESIGN_WIDTH *0.23, DESIGN_HEIGHT * 0.7), // 位置
        "start_button"                  // 按钮名称
    );
    addButtonEffect(_startButton);
    _startButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                this->onStartGame(sender);
            }
        });

    // 按钮2: 玩法按钮
    _settingButton = createImageButton(
        "Resources/UI/htp-button.png",
        "Resources/UI/htp-button-after.png",
        Vec2(DESIGN_WIDTH *0.23, DESIGN_HEIGHT /2),
        "howtoplay_button"
    );
    addButtonEffect(_settingButton);
    _settingButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                this->onSetting(sender);
            }
        });

    // 按钮3: 历史分数按钮
    _exitButton = createImageButton(
        "Resources/UI/score.png",
        "Resources/UI/score-after.png",
        Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT * 0.25f),
        "score_button"
    );
    addButtonEffect(_exitButton);
    _exitButton->addTouchEventListener([this](Ref* sender, ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                this->onExit(sender);
            }
        });

    // 添加按钮下方的文字标签（可选）
    //addButtonLabels();
}

// 创建单个图片按钮
ui::Button* StartScene::createImageButton(const std::string& normalImage,
    const std::string& pressedImage,
    const cocos2d::Vec2& position,
    const std::string& buttonName)
{
    // 创建按钮
    auto button = ui::Button::create();
    button->setName(buttonName);

    // 设置按钮图片
    if (!normalImage.empty() && !pressedImage.empty())
    {
        // 检查文件是否存在
        bool normalExists = FileUtils::getInstance()->isFileExist(normalImage);
        bool pressedExists = FileUtils::getInstance()->isFileExist(pressedImage);

        if (normalExists && pressedExists)
        {
            button->loadTextures(normalImage, pressedImage, "");
        }
        else
        {
            // 如果图片不存在，创建替代的按钮
            CCLOG("Image not found: %s or %s", normalImage.c_str(), pressedImage.c_str());
            button->setTitleText(buttonName);
            button->setTitleFontSize(48);
            button->setTitleColor(Color3B::WHITE);
            button->setScale9Enabled(true);
            button->setContentSize(Size(400, 150));
            button->setColor(Color3B(100, 149, 237)); // 矢车菊蓝
        }
    }

    // 设置位置
    button->setPosition(position);

    // 添加到场景
    this->addChild(button, 1);

    return button;
}

// 添加按钮点击效果
void StartScene::addButtonEffect(ui::Button* button)
{
    if (!button) return;

    // 添加触摸事件实现点击效果
    button->addTouchEventListener([button](Ref* sender, ui::Widget::TouchEventType type)
        {
            switch (type)
            {
            case ui::Widget::TouchEventType::BEGAN:
                // 按下时缩小效果
                button->runAction(ScaleTo::create(0.1f, 0.95f));
                break;

            case ui::Widget::TouchEventType::ENDED:
                // 释放时恢复并轻微反弹
                button->runAction(Sequence::create(
                    ScaleTo::create(0.1f, 1.05f),
                    ScaleTo::create(0.1f, 1.0f),
                    nullptr
                ));
                break;

            case ui::Widget::TouchEventType::CANCELED:
                // 取消时恢复
                button->runAction(ScaleTo::create(0.1f, 1.0f));
                break;

            default:
                break;
            }
        });
}

// 添加按钮文字标签
void StartScene::addButtonLabels()
{
    // 开始按钮标签
    auto startLabel = Label::createWithTTF("开始游戏", "fonts/arial.ttf", 48);
    startLabel->setPosition(Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT * 0.55f - 100));
    startLabel->setTextColor(Color4B(255, 255, 255, 255));
    startLabel->enableShadow(Color4B::BLACK, Size(2, -2), 3);
    this->addChild(startLabel, 2);

    // 设置按钮标签
    auto settingLabel = Label::createWithTTF("游戏设置", "fonts/arial.ttf", 48);
    settingLabel->setPosition(Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT * 0.4f - 100));
    settingLabel->setTextColor(Color4B(255, 255, 255, 255));
    settingLabel->enableShadow(Color4B::BLACK, Size(2, -2), 3);
    this->addChild(settingLabel, 2);

    // 退出按钮标签
    auto exitLabel = Label::createWithTTF("退出游戏", "fonts/arial.ttf", 48);
    exitLabel->setPosition(Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT * 0.25f - 100));
    exitLabel->setTextColor(Color4B(255, 255, 255, 255));
    exitLabel->enableShadow(Color4B::BLACK, Size(2, -2), 3);
    this->addChild(exitLabel, 2);
}

// 开始游戏按钮回调
void StartScene::onStartGame(cocos2d::Ref* sender)
{
    CCLOG("开始游戏按钮被点击");

    // 播放点击音效（可选）
    // CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click.mp3");

    // 添加按钮点击反馈
    if (_startButton)
    {
        _startButton->setEnabled(false); // 禁用按钮防止重复点击
    }

    // 创建淡出效果
    auto fadeOut = FadeOut::create(0.5f);
    this->runAction(Sequence::create(
        fadeOut,
        CallFunc::create([this]() {
            // 切换到游戏场景
            auto scene = GameScene::createScene();
            if (scene)
            {
                auto transition = TransitionFade::create(0.5f, scene);
                Director::getInstance()->replaceScene(transition);
            }
            else
            {
                CCLOGERROR("无法创建游戏场景");
                if (_startButton) _startButton->setEnabled(true);
                this->setOpacity(255);
            }
            }),
        nullptr
    ));
}

// 设置按钮回调
void StartScene::onSetting(cocos2d::Ref* sender)
{
    CCLOG("设置按钮被点击");

    // 播放点击音效
    // CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click.mp3");

    // 添加按钮点击反馈
    if (_settingButton)
    {
        _settingButton->setEnabled(false);
    }

    // 创建淡出效果
    auto fadeOut = FadeOut::create(0.5f);
    this->runAction(Sequence::create(
        fadeOut,
        CallFunc::create([this]() {
            // 切换到设置场景
            auto scene = SettingScene::createScene();
            if (scene)
            {
                auto transition = TransitionFade::create(0.5f, scene);
                Director::getInstance()->replaceScene(transition);
            }
            else
            {
                CCLOGERROR("无法创建设置场景");
                if (_settingButton) _settingButton->setEnabled(true);
                this->setOpacity(255);
            }
            }),
        nullptr
    ));
}

// 退出按钮回调
void StartScene::onExit(cocos2d::Ref* sender)
{
    CCLOG("退出按钮被点击");

    // 播放点击音效
    // CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click.mp3");

    // 添加按钮点击反馈
    if (_exitButton)
    {
        _exitButton->setEnabled(false);
    }

    // 创建确认对话框
    auto messageBox = LayerColor::create(Color4B(0, 0, 0, 200),
        DESIGN_WIDTH, DESIGN_HEIGHT);
    messageBox->setPosition(Vec2(0, 0));
    this->addChild(messageBox, 10);

    // 对话框背景
    auto dialogBg = Sprite::create();
    dialogBg->setColor(Color3B(60, 60, 80));
    dialogBg->setTextureRect(Rect(0, 0, 800, 400));
    dialogBg->setPosition(Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT / 2));
    messageBox->addChild(dialogBg, 1);

    // 对话框文字
    auto messageLabel = Label::createWithTTF("确定要退出游戏吗？", "fonts/arial.ttf", 60);
    messageLabel->setPosition(Vec2(DESIGN_WIDTH / 2, DESIGN_HEIGHT / 2 + 80));
    messageLabel->setTextColor(Color4B::WHITE);
    messageBox->addChild(messageLabel, 2);

    // 确认按钮
    auto confirmButton = ui::Button::create("ui/confirm_normal.png",
        "ui/confirm_pressed.png");
    if (!confirmButton)
    {
        confirmButton = ui::Button::create();
        confirmButton->setTitleText("确认");
        confirmButton->setTitleFontSize(48);
        confirmButton->setTitleColor(Color3B::WHITE);
        confirmButton->setScale9Enabled(true);
        confirmButton->setContentSize(Size(200, 100));
        confirmButton->setColor(Color3B(220, 20, 60)); // 深红色
    }
    confirmButton->setPosition(Vec2(DESIGN_WIDTH / 2 - 150, DESIGN_HEIGHT / 2 - 80));
    confirmButton->addTouchEventListener([messageBox, this](Ref* sender,
        ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                CCLOG("确认退出游戏");
                Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
                exit(0);
#endif
            }
        });
    messageBox->addChild(confirmButton, 2);

    // 取消按钮
    auto cancelButton = ui::Button::create("ui/cancel_normal.png",
        "ui/cancel_pressed.png");
    if (!cancelButton)
    {
        cancelButton = ui::Button::create();
        cancelButton->setTitleText("取消");
        cancelButton->setTitleFontSize(48);
        cancelButton->setTitleColor(Color3B::WHITE);
        cancelButton->setScale9Enabled(true);
        cancelButton->setContentSize(Size(200, 100));
        cancelButton->setColor(Color3B(100, 149, 237)); // 矢车菊蓝
    }
    cancelButton->setPosition(Vec2(DESIGN_WIDTH / 2 + 150, DESIGN_HEIGHT / 2 - 80));
    cancelButton->addTouchEventListener([messageBox, this](Ref* sender,
        ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                CCLOG("取消退出");
                messageBox->removeFromParent();
                if (_exitButton) _exitButton->setEnabled(true);
            }
        });
    messageBox->addChild(cancelButton, 2);
}