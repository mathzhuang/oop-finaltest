#include "SelectScene.h"
#include "GameScene.h"
#include "SelectDifficultyScene.h"
#include <vector>

#define LOG_CONFIRM(...) cocos2d::log(__VA_ARGS__)
#define WIN_SIZE Director::getInstance()->getWinSize()

USING_NS_CC;

// --- 生命周期 ---

SelectScene::SelectScene()
    : _mode(GameMode::SINGLE)
    , _selectedChar1(0)
    , _selectedChar2(0)
    , _selectingPlayer1(true)
    , _currentSelectedIndex(0)
{
}

Scene* SelectScene::createScene(GameMode mode)
{
    auto scene = SelectScene::create();
    scene->_mode = mode;
    return scene;
}

bool SelectScene::init()
{
    if (!Scene::init()) return false;

    // 1. 初始化角色坐标 (硬编码适配UI)
    float charY = 1076.9f;
    _characterPositions = {
        Vec2(239.7f, charY),
        Vec2(741.6f, charY),
        Vec2(1263.8f, charY),
        Vec2(1801.3f, charY)
    };

    // 2. 背景设置 (全屏拉伸)
    auto background = Sprite::create("UI/selectbackground.png");
    if (background)
    {
        background->setPosition(WIN_SIZE / 2);
        background->setScaleX(WIN_SIZE.width / background->getContentSize().width);
        background->setScaleY(WIN_SIZE.height / background->getContentSize().height);
        this->addChild(background, 0);
    }

    // 3. 选择光标 (带上下浮动动画)
    _arrowSprite = Sprite::create("UI/arrow.png");
    if (_arrowSprite)
    {
        _arrowSprite->setPosition(_characterPositions[_currentSelectedIndex]);
        this->addChild(_arrowSprite, 10);

        auto move = MoveBy::create(0.5f, Vec2(0, 20));
        _arrowSprite->runAction(RepeatForever::create(Sequence::create(move, move->reverse(), nullptr)));
    }

    // 4. 注册键盘监听
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(SelectScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// --- 输入事件处理 ---

void SelectScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    int newIndex = _currentSelectedIndex;
    int totalChars = (int)_characterPositions.size();

    // 1. 左右导航
    if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW)
    {
        newIndex = (_currentSelectedIndex - 1 + totalChars) % totalChars;
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
    {
        newIndex = (_currentSelectedIndex + 1) % totalChars;
    }
    // 2. 确认选择
    else if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        int selectedID = _currentSelectedIndex + 1; // 角色ID从1开始

        if (_mode == GameMode::SINGLE)
        {
            // 单人模式 -> 跳转难度选择
            CCLOG("Single Player Selected: %d", selectedID);
            _selectedChar1 = selectedID;
            auto diffScene = SelectDifficultyScene::createScene(_mode, _selectedChar1);
            Director::getInstance()->replaceScene(TransitionFade::create(1.0f, diffScene));
        }
        else if (_mode == GameMode::FOG)
        {
            // 迷雾模式 -> 直接开始游戏
            CCLOG("Fog Mode Selected: %d", selectedID);
            _selectedChar1 = selectedID;
            auto gameScene = GameScene::createWithMode(GameMode::FOG, _selectedChar1);
            Director::getInstance()->replaceScene(TransitionFade::create(1.0f, gameScene));
        }
        else if (_mode == GameMode::LOCAL_2P)
        {
            // 双人模式分步选择
            if (_selectingPlayer1)
            {
                // 阶段一：P1 选择完成
                _selectedChar1 = selectedID;
                CCLOG("P1 Selected: %d. Waiting for P2...", _selectedChar1);

                _selectingPlayer1 = false; // 进入P2选择阶段

                // 自动跳到下一个位置，避免光标重叠
                int nextPos = (_selectedChar1 % 4);
                if (nextPos == _selectedChar1 - 1) nextPos = (nextPos + 1) % 4;
                moveArrowTo(nextPos);

                return; // 早期返回，等待下一次输入
            }
            else
            {
                // 阶段二：P2 选择完成 -> 开始游戏
                _selectedChar2 = selectedID;
                CCLOG("P2 Selected: %d. Starting Game...", _selectedChar2);

                // 持久化保存选择
                UserDefault::getInstance()->setIntegerForKey("SelectedCharacter1", _selectedChar1);
                UserDefault::getInstance()->setIntegerForKey("SelectedCharacter2", _selectedChar2);

                auto gameScene = GameScene::createWithMode(GameMode::LOCAL_2P, _selectedChar1, _selectedChar2);
                Director::getInstance()->replaceScene(TransitionFade::create(1.0f, gameScene));

                _selectingPlayer1 = true; // 重置状态
                return;
            }
        }
    }

    // 更新光标位置
    if (newIndex != _currentSelectedIndex)
    {
        moveArrowTo(newIndex);
    }
}

// --- 视觉反馈 ---

void SelectScene::moveArrowTo(int newIndex)
{
    _currentSelectedIndex = newIndex;
    Vec2 targetPos = _characterPositions[_currentSelectedIndex];

    // 平滑移动光标
    // 注意：stopAllActions 会停止浮动动画，如需保留浮动需修改此处逻辑
    _arrowSprite->stopAllActions();

    auto moveTo = MoveTo::create(0.25f, targetPos);
    auto ease = EaseSineOut::create(moveTo);
    _arrowSprite->runAction(ease);
}