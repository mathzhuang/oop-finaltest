#pragma once
#include "cocos2d.h"
#include "GameMode.h"

class GameOverLayer : public cocos2d::Layer
{
public:
    // --- 工厂方法 ---

    // 创建结算层 (支持传递分数)
    static GameOverLayer* create(bool isWin, GameMode mode, int p1Face, int p2Face);
    static GameOverLayer* create(bool isWin, GameMode mode, int p1Face, int p2Face, int score);

    // --- 初始化 ---
    virtual bool init(bool isWin, GameMode mode, int p1Face, int p2Face, int score);

private:
    // --- 交互回调 ---
    void onRestart(cocos2d::Ref* sender);
    void onReturn(cocos2d::Ref* sender);

private:
    // --- 缓存数据 ---
    // 保存当前局配置，用于点击 Restart 时快速重建场景
    GameMode _mode;
    int      _p1Face;
    int      _p2Face;
};