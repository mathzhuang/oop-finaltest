#pragma once
#ifndef __SELECT_SCENE_H__
#define __SELECT_SCENE_H__

#include "cocos2d.h"
#include "GameMode.h"   // 你的游戏模式枚举
#include "GameScene.h"  // 游戏主场景

USING_NS_CC;

class SelectScene : public cocos2d::Scene
{
public:
    SelectScene(); // ✅ 声明默认构造函数
    // 支持传入模式参数
    static cocos2d::Scene* createScene(GameMode mode = GameMode::SINGLE);

    virtual bool init() override;

    CREATE_FUNC(SelectScene);

private:
    bool _selectingPlayer1 = true; // 用于双人模式判断当前是玩家1还是玩家2选择
    // ------------------------
    // 成员变量
    // ------------------------
    cocos2d::Sprite* _arrowSprite = nullptr;            // 选择箭头
    std::vector<cocos2d::Vec2> _characterPositions;    // 四个角色箭头位置
    int _currentSelectedIndex = 0;                      // 当前选择索引

    GameMode _mode = GameMode::SINGLE;                 // 当前选择模式
    // 选中的角色索引（双人模式用）
    int _selectedChar1 = 0;
    int _selectedChar2 = 1;
    // ------------------------
    // 方法
    // ------------------------
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void moveArrowTo(int newIndex);
};

#endif // __SELECT_SCENE_H__
