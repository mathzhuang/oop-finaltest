#pragma once
#ifndef __SELECT_SCENE_2PLAYER_H__
#define __SELECT_SCENE_2PLAYER_H__

#include "cocos2d.h"
#include <vector>

class SelectScene_2Player : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(SelectScene_2Player);

private:
    // --- UI 资源 ---
    std::vector<cocos2d::Vec2> _characterPositions; // 角色坐标
    cocos2d::Sprite* _arrowP1 = nullptr;            // P1 箭头 (红)
    cocos2d::Sprite* _arrowP2 = nullptr;            // P2 箭头 (蓝)

    // --- 逻辑状态 ---
    int _p1Index = 0;           // P1 当前位置
    bool _p1Confirmed = false;  // P1 是否锁定

    int _p2Index = 1;           // P2 当前位置 (默认和P1错开)
    bool _p2Confirmed = false;  // P2 是否锁定

    // --- 内部方法 ---
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    void updateArrowPos(cocos2d::Sprite* arrow, int index);
    void checkStartGame();      // 检查是否两人都准备好了
};

#endif // __SELECT_SCENE_2PLAYER_H__