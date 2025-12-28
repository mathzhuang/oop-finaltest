#pragma once
#include "cocos2d.h"
#include "GameMode.h"
#include "GameScene.h"

USING_NS_CC;

class SelectScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    SelectScene(); // 构造函数
    virtual bool init() override;
    CREATE_FUNC(SelectScene);

    // 创建场景并指定模式 (默认为单人)
    static cocos2d::Scene* createScene(GameMode mode = GameMode::SINGLE);

private:
    // --- 交互回调 ---

    // 键盘事件监听 (左右移动，回车确认)
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);

    // 平滑移动选择光标
    void moveArrowTo(int newIndex);

private:
    // --- 游戏配置 ---
    GameMode _mode;

    // --- 选择状态 ---
    int  _currentSelectedIndex; // 当前光标位置 (0~3)
    int  _selectedChar1;        // P1 选定角色ID
    int  _selectedChar2;        // P2 选定角色ID
    bool _selectingPlayer1;     // 双人模式标志位: true=P1正在选, false=P2正在选

    // --- UI 组件 ---
    cocos2d::Sprite* _arrowSprite = nullptr;
    std::vector<cocos2d::Vec2> _characterPositions; // 预设的角色坐标列表
};