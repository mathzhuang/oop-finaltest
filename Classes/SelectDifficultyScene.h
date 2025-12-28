#pragma once
#include "cocos2d.h"
#include "GameMode.h"

class SelectDifficultyScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();

    // 创建场景并注入前序数据 (模式、P1角色)
    static cocos2d::Scene* createScene(GameMode mode, int p1Face);

    virtual bool init() override;
    CREATE_FUNC(SelectDifficultyScene);

    // --- 数据接口 ---
    void setData(GameMode mode, int p1Face);

private:
    // --- 交互回调 ---
    void onEasyMode(cocos2d::Ref* sender);
    void onHardMode(cocos2d::Ref* sender);

    // --- 内部逻辑 ---
    // 携带最终确定的难度参数跳转至 GameScene
    void startGame(GameDifficulty diff);

private:
    // --- 暂存数据 ---
    GameMode _mode;
    int      _p1Face;
};