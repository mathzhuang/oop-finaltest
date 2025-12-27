#pragma once
#ifndef __SELECT_DIFFICULTY_SCENE_H__
#define __SELECT_DIFFICULTY_SCENE_H__

#include "cocos2d.h"
#include "GameMode.h"

class SelectDifficultyScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    // ⭐ 自定义创建函数，接收上一个场景传来的数据
    static cocos2d::Scene* createScene(GameMode mode, int p1Face);

    virtual bool init();

    // 必须手动实现 create，或者使用 CREATE_FUNC 但我们需要传参，所以主要用上面的 createScene
    CREATE_FUNC(SelectDifficultyScene);

    // 设置数据的函数
    void setData(GameMode mode, int p1Face);

private:
    // 暂存的数据
    GameMode _mode;
    int _p1Face;

    // 按钮回调
    void onEasyMode(cocos2d::Ref* sender);
    void onHardMode(cocos2d::Ref* sender);

    // 开始游戏跳转
    void startGame(GameDifficulty diff);
};

#endif // __SELECT_DIFFICULTY_SCENE_H__