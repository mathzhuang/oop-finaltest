#pragma once
#include "cocos2d.h"
#include "GameMode.h" // 确保能访问你的 GameMode 枚举

class GameOverLayer : public cocos2d::Layer
{
public:
    // 自定义 create 函数，接收输赢状态和重启所需参数
    static GameOverLayer* create(bool isWin, GameMode mode, int p1Face, int p2Face);

    // 初始化
    virtual bool init(bool isWin, GameMode mode, int p1Face, int p2Face);

private:
    // 记录重启参数
    GameMode _mode;
    int _p1Face;
    int _p2Face;

    void onRestart(cocos2d::Ref* sender);
    void onReturn(cocos2d::Ref* sender);
};