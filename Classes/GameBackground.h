#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class GameBackground : public cocos2d::Layer
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameBackground);

    // --- 状态控制 ---

    // 处理关卡胜利 (Level自增)
    void onLevelWin();

    // 获取当前暂停状态
    bool isGamePaused() const { return _isPaused; }

    // --- UI 更新 ---

    // 刷新指定玩家的数据显示 (playerIndex: 0~3)
    void updatePlayerStat(int playerIndex, int score, int itemCount);

private:
    // --- 内部初始化 ---
    void initGrid();    // 绘制13x13网格
    void initSideBar(); // 布局侧边栏 (头像、分数、计时器)
    void initButtons(); // 布局底部功能按钮

    // --- 事件回调 ---
    void updateTimer(float dt); // 倒计时调度器
    void onSoundEvent(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
    void onReturnEvent(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
    void onPauseEvent(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

private:
    // --- UI 组件 ---
    cocos2d::Label* _levelLabel = nullptr;
    cocos2d::Label* _timerLabel = nullptr;
    cocos2d::ui::Button* _soundBtn = nullptr;
    cocos2d::ui::Button* _returnBtn = nullptr;
    cocos2d::ui::Button* _pauseBtn = nullptr;

    // 动态存储玩家信息Label
    std::vector<cocos2d::Label*> _scoreLabels;
    std::vector<cocos2d::Label*> _itemCountLabels;

    // --- 游戏数据 ---
    int  _currentLevel;
    int  _timeLeft;   // 剩余秒数
    bool _isSoundOn;  // 音效开关
    bool _isPaused;   // 暂停标志
};