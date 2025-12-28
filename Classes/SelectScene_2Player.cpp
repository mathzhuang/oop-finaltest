#include "SelectScene_2Player.h"
#include "GameScene.h"
#include "GameMode.h"

USING_NS_CC;

// --- 生命周期 ---

Scene* SelectScene_2Player::createScene()
{
    return SelectScene_2Player::create();
}

bool SelectScene_2Player::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 定义角色坐标 (硬编码适配UI)
    float charY = 1076.9f;
    _characterPositions = {
        Vec2(239.7f, charY), Vec2(741.6f, charY),
        Vec2(1263.8f, charY), Vec2(1801.3f, charY)
    };

    // 2. 背景适配
    auto bg = Sprite::create("UI/selectBackground_twoPlayers.png");
    if (bg)
    {
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        bg->setScaleX(visibleSize.width / bg->getContentSize().width);
        bg->setScaleY(visibleSize.height / bg->getContentSize().height);
        this->addChild(bg, 0);
    }

    // 3. 初始化 P1 箭头 (红色)
    _arrowP1 = Sprite::create("UI/redarrow.png");
    if (_arrowP1) {
        _arrowP1->setPosition(_characterPositions[_p1Index] + Vec2(0, 50));
        this->addChild(_arrowP1, 10);

        // 呼吸动画
        auto move = MoveBy::create(0.5f, Vec2(0, 20));
        _arrowP1->runAction(RepeatForever::create(Sequence::create(move, move->reverse(), nullptr)));
    }

    // 4. 初始化 P2 箭头 (蓝色，默认位置错开)
    _arrowP2 = Sprite::create("UI/bluearrow.png");
    if (_arrowP2) {
        _arrowP2->setPosition(_characterPositions[_p2Index] + Vec2(0, 50));
        this->addChild(_arrowP2, 10);

        // 呼吸动画
        auto move = MoveBy::create(0.5f, Vec2(0, 20));
        _arrowP2->runAction(RepeatForever::create(Sequence::create(move, move->reverse(), nullptr)));
    }

    // 5. 注册输入监听
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(SelectScene_2Player::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// --- 输入事件处理 ---

void SelectScene_2Player::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    // ---------------- P1 控制 (WASD + Space) ----------------
    if (!_p1Confirmed)
    {
        // 移动选择
        int prev = _p1Index;
        if (keyCode == EventKeyboard::KeyCode::KEY_A)      _p1Index = (_p1Index - 1 + 4) % 4;
        else if (keyCode == EventKeyboard::KeyCode::KEY_D) _p1Index = (_p1Index + 1) % 4;

        if (prev != _p1Index) updateArrowPos(_arrowP1, _p1Index);

        // 确认锁定 (Space)
        if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
        {
            // 防冲突：不能选择 P2 已经锁定的角色
            if (_p2Confirmed && _p1Index == _p2Index) {
                CCLOG("P1 Conflict: Spot taken by P2!");
            }
            else {
                _p1Confirmed = true;
                _arrowP1->setColor(Color3B::GREEN); // 视觉反馈：变绿
                checkStartGame();
            }
        }
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        // 解锁
        _p1Confirmed = false;
        _arrowP1->setColor(Color3B::WHITE); // 恢复原色(红箭头的原图色)
    }

    // ---------------- P2 控制 (方向键 + Enter) ----------------
    if (!_p2Confirmed)
    {
        // 移动选择
        int prev = _p2Index;
        if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW)       _p2Index = (_p2Index - 1 + 4) % 4;
        else if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) _p2Index = (_p2Index + 1) % 4;

        if (prev != _p2Index) updateArrowPos(_arrowP2, _p2Index);

        // 确认锁定 (Enter)
        if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER)
        {
            // 防冲突
            if (_p1Confirmed && _p1Index == _p2Index) {
                CCLOG("P2 Conflict: Spot taken by P1!");
            }
            else {
                _p2Confirmed = true;
                _arrowP2->setColor(Color3B::GREEN);
                checkStartGame();
            }
        }
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER)
    {
        // 解锁
        _p2Confirmed = false;
        _arrowP2->setColor(Color3B::WHITE);
    }
}

// --- 视觉反馈 ---

void SelectScene_2Player::updateArrowPos(Sprite* arrow, int index)
{
    if (arrow) {
        // 仅更新X轴，保留Y轴的呼吸动画位置
        arrow->setPositionX(_characterPositions[index].x);
    }
}

// --- 核心逻辑 ---

void SelectScene_2Player::checkStartGame()
{
    // 双人均已锁定，且位置不冲突
    if (_p1Confirmed && _p2Confirmed && _p1Index != _p2Index)
    {
        int p1ID = _p1Index + 1;
        int p2ID = _p2Index + 1;

        CCLOG("Both Ready! P1:%d, P2:%d. Starting...", p1ID, p2ID);

        auto scene = GameScene::createWithMode(GameMode::LOCAL_2P, p1ID, p2ID);
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }
}