#include "SelectScene_2Player.h"
#include "GameScene.h"  // 必须包含，用于跳转
#include "GameMode.h"   // 用于传递 GameMode::LOCAL_2P

USING_NS_CC;

#define WIN_SIZE Director::getInstance()->getWinSize()

Scene* SelectScene_2Player::createScene()
{
    return SelectScene_2Player::create();
}

bool SelectScene_2Player::init()
{
    if (!Scene::init()) return false;

    // 1. 定义角色位置 (保持和你原版一致)
    float charY = 1076.9f;
    _characterPositions = {
        Vec2(239.7f, charY),
        Vec2(741.6f, charY),
        Vec2(1263.8f, charY),
        Vec2(1801.3f, charY)
    };

    // 2. 背景
    auto background = Sprite::create("UI/selectBackground_twoPlayers.png");
    if (background)
    {
        background->setPosition(WIN_SIZE / 2);
        Vec2 scale(WIN_SIZE.width / background->getContentSize().width,
            WIN_SIZE.height / background->getContentSize().height);
        background->setScale(scale.x, scale.y);
        this->addChild(background, 0);
    }

    // 3. 初始化箭头 P1 (红色)
    _arrowP1 = Sprite::create("UI/redarrow.png");
    if (_arrowP1) {
        //_arrowP1->setColor(Color3B::RED);
        _arrowP1->setPosition(_characterPositions[_p1Index] + Vec2(0, 50));
        this->addChild(_arrowP1, 10);

        // 浮动动画
        auto move = MoveBy::create(0.5f, Vec2(0, 20));
        _arrowP1->runAction(RepeatForever::create(Sequence::create(move, move->reverse(), nullptr)));
    }

    // 4. 初始化箭头 P2 (蓝色)
    _arrowP2 = Sprite::create("UI/bluearrow.png");
    if (_arrowP2) {
        //_arrowP2->setColor(Color3B::BLUE);
        _arrowP2->setPosition(_characterPositions[_p2Index] + Vec2(0, 50));
        this->addChild(_arrowP2, 10);

        // 浮动动画 (稍微错开时间)
        auto move = MoveBy::create(0.5f, Vec2(0, 20));
        _arrowP2->runAction(RepeatForever::create(Sequence::create(move, move->reverse(), nullptr)));
    }

    // 5. 键盘监听
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(SelectScene_2Player::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void SelectScene_2Player::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    // ================= P1 控制 (WASD) =================
    if (!_p1Confirmed) // 未锁定时可以移动
    {
        int prev = _p1Index;
        if (keyCode == EventKeyboard::KeyCode::KEY_A)
            _p1Index = (_p1Index - 1 + 4) % 4;
        else if (keyCode == EventKeyboard::KeyCode::KEY_D)
            _p1Index = (_p1Index + 1) % 4;

        if (prev != _p1Index) updateArrowPos(_arrowP1, _p1Index);

        // P1 确认 (Space)
        if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
        {
            // 检查冲突：如果 P2 已经确认且位置相同，则 P1 不能选
            if (_p2Confirmed && _p1Index == _p2Index) {
                CCLOG("Cannot select: P2 already took this spot!");
                // 可以加个音效或抖动
            }
            else {
                _p1Confirmed = true;
                _arrowP1->setColor(Color3B::GREEN); // 变绿
                checkStartGame();
            }
        }
    }
    // P1 取消 (Space)
    else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        _p1Confirmed = false;
        _arrowP1->setColor(Color3B::RED); // 变回红
    }

    // ================= P2 控制 (Arrows) =================
    if (!_p2Confirmed) // 未锁定时可以移动
    {
        int prev = _p2Index;
        if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW)
            _p2Index = (_p2Index - 1 + 4) % 4;
        else if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
            _p2Index = (_p2Index + 1) % 4;

        if (prev != _p2Index) updateArrowPos(_arrowP2, _p2Index);

        // P2 确认 (Enter)
        if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER)
        {
            // 检查冲突
            if (_p1Confirmed && _p1Index == _p2Index) {
                CCLOG("Cannot select: P1 already took this spot!");
            }
            else {
                _p2Confirmed = true;
                _arrowP2->setColor(Color3B::GREEN); // 变绿
                checkStartGame();
            }
        }
    }
    // P2 取消 (Enter)
    else if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER)
    {
        _p2Confirmed = false;
        _arrowP2->setColor(Color3B::BLUE); // 变回蓝
    }
}

void SelectScene_2Player::updateArrowPos(Sprite* arrow, int index)
{
    if (arrow) {
        // 只修改 X 轴，保留 Y 轴动作
        arrow->setPositionX(_characterPositions[index].x);
    }
}

void SelectScene_2Player::checkStartGame()
{
    // 只有两人都确认，且不冲突时才开始
    if (_p1Confirmed && _p2Confirmed)
    {
        if (_p1Index == _p2Index) {
            // 理论上上面的按键逻辑已经防住了，这里双重保险
            return;
        }

        int p1ID = _p1Index + 1;
        int p2ID = _p2Index + 1;

        CCLOG("Starting 2P Game: P1=%d, P2=%d", p1ID, p2ID);

        // 跳转到游戏场景
        auto scene = GameScene::createWithMode(GameMode::LOCAL_2P, p1ID, p2ID);
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }
}