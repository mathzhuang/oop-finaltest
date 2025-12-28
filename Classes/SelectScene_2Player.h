#pragma once
#include "cocos2d.h"
#include <vector>

class SelectScene_2Player : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(SelectScene_2Player);

private:
    // --- 内部逻辑 ---

    // 键盘监听 (P1: WASD+Space, P2: Arrows+Enter)
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

    // 更新箭头位置 (仅修改X轴)
    void updateArrowPos(cocos2d::Sprite* arrow, int index);

    // 检查开始条件 (两人均锁定且不冲突)
    void checkStartGame();

private:
    // --- UI 资源 ---
    std::vector<cocos2d::Vec2> _characterPositions; // 预设坐标
    cocos2d::Sprite* _arrowP1 = nullptr;            // P1 红色箭头
    cocos2d::Sprite* _arrowP2 = nullptr;            // P2 蓝色箭头

    // --- 游戏状态 ---
    int  _p1Index = 0;          // P1 当前选角索引
    bool _p1Confirmed = false;  // P1 锁定状态

    int  _p2Index = 1;          // P2 当前选角索引 (默认错开)
    bool _p2Confirmed = false;  // P2 锁定状态
};