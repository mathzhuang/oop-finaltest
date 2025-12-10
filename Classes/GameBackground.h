#pragma once
#ifndef __GAME_BACKGROUND_H__
#define __GAME_BACKGROUND_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class GameBackground : public cocos2d::Layer
{
public:
    // 创建场景的静态方法
    static cocos2d::Scene* createScene();

    virtual bool init();

    // 实现 create() 方法
    CREATE_FUNC(GameBackground);

    // 外部调用：当玩家获胜时调用此函数，Level + 1
    void onLevelWin();

    // 外部调用：获取当前暂停状态（供Player类检查）
    bool isGamePaused() const { return _isPaused; }

private:
    // --- UI 组件 ---
    cocos2d::Label* _levelLabel;
    cocos2d::Label* _timerLabel;
    cocos2d::ui::Button* _soundBtn;
    cocos2d::ui::Button* _returnBtn;
    cocos2d::ui::Button* _pauseBtn;

    // --- 数据变量 ---
    int _currentLevel;
    int _timeLeft;      // 剩余秒数
    bool _isSoundOn;    // 声音开关状态
    bool _isPaused;     // 暂停状态

    // --- 内部逻辑方法 ---
    void initGrid();        // 初始化13x13网格
    void initSideBar();     // 初始化侧边栏（图片、Level、Timer）
    void initButtons();     // 初始化底部按钮

    // --- 回调函数 ---
    void updateTimer(float dt); // 倒计时调度器
    void onSoundEvent(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
    void onReturnEvent(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
    void onPauseEvent(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
};

#endif // __GAME_BACKGROUND_H__