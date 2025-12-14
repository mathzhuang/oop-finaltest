#include "SelectScene.h"
#include "GameScene.h"
#include <vector>

#define LOG_CONFIRM(...) cocos2d::log(__VA_ARGS__)
#define WIN_SIZE Director::getInstance()->getWinSize()

// 默认模式：单人
SelectScene::SelectScene()
    : _mode(GameMode::SINGLE), _selectedChar1(0), _selectedChar2(0), _selectingPlayer1(true)
{
}

// --- 场景创建方法 ---
cocos2d::Scene* SelectScene::createScene(GameMode mode)
{
    auto scene = SelectScene::create();
    scene->_mode = mode;
    return scene;
}

// --- 初始化方法 ---
bool SelectScene::init()
{
    if (!Scene::init()) return false;

    // 1. 定义角色位置
    float charY = 1076.9f;
    _characterPositions = {
        Vec2(239.7f, charY),
        Vec2(741.6f, charY),
        Vec2(1263.8f, charY),
        Vec2(1801.3f, charY)
    };
    _currentSelectedIndex = 0;

    // 2. 添加背景
    auto background = Sprite::create("UI/selectbackground.png");
    if (background)
    {
        background->setPosition(WIN_SIZE / 2);
        background->setScaleX(WIN_SIZE.width / background->getContentSize().width);
        background->setScaleY(WIN_SIZE.height / background->getContentSize().height);
        this->addChild(background, 0);
    }

    // 3. 添加箭头
    _arrowSprite = Sprite::create("UI/arrow.png");
    if (_arrowSprite)
    {
        _arrowSprite->setPosition(_characterPositions[_currentSelectedIndex]);
        this->addChild(_arrowSprite, 10);
    }

    // 4. 注册键盘事件
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(SelectScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// --- 键盘事件处理 ---
void SelectScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    int newIndex = _currentSelectedIndex;

    // 左右选择
    if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW)
        newIndex = (_currentSelectedIndex - 1 + (int)_characterPositions.size()) % (int)_characterPositions.size();
    else if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
        newIndex = (_currentSelectedIndex + 1) % (int)_characterPositions.size();

    // 确认选择
    else if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        if (_mode == GameMode::SINGLE)
        {
            _selectedChar1 = _currentSelectedIndex + 1;
            CCLOG("单人模式玩家选择: %d", _selectedChar1);

            auto gameScene = GameScene::createWithMode(GameMode::SINGLE, _selectedChar1);
            Director::getInstance()->replaceScene(TransitionFade::create(1.0f, gameScene));
        }
        else if (_mode == GameMode::LOCAL_2P)
        {
            if (_selectingPlayer1)
            {
                _selectedChar1 = _currentSelectedIndex + 1;
                CCLOG("玩家1选择完成: %d", _selectedChar1);

                // 切换到玩家2选择
                _selectingPlayer1 = false;

                // 玩家2默认选一个不同于玩家1的角色
                _currentSelectedIndex = (_selectedChar1 % 4);
                if (_currentSelectedIndex == _selectedChar1 - 1)
                    _currentSelectedIndex = (_currentSelectedIndex + 1) % 4;

                moveArrowTo(_currentSelectedIndex);
                return;
            }
            else
            {
                _selectedChar2 = _currentSelectedIndex + 1;
                CCLOG("玩家2选择完成: %d", _selectedChar2);

                // 保存选择（可选）
                UserDefault::getInstance()->setIntegerForKey("SelectedCharacter1", _selectedChar1);
                UserDefault::getInstance()->setIntegerForKey("SelectedCharacter2", _selectedChar2);

                // ⚠️ 关键：传递正确的玩家 ID 和模式
                auto gameScene = GameScene::createWithMode(GameMode::LOCAL_2P, _selectedChar1, _selectedChar2);
                CCLOG("跳转 GameScene: player1=%d, player2=%d", _selectedChar1, _selectedChar2);
                Director::getInstance()->replaceScene(TransitionFade::create(1.0f, gameScene));

                _selectingPlayer1 = true; // 下次重新选择
                return;
            }
        }
    }


    // 移动箭头
    if (newIndex != _currentSelectedIndex)
        moveArrowTo(newIndex);
}

// --- 平滑移动箭头 ---
void SelectScene::moveArrowTo(int newIndex)
{
    _currentSelectedIndex = newIndex;
    Vec2 targetPos = _characterPositions[_currentSelectedIndex];

    auto moveTo = MoveTo::create(0.25f, targetPos);
    auto ease = EaseSineOut::create(moveTo);
    _arrowSprite->stopAllActions();
    _arrowSprite->runAction(ease);
}
