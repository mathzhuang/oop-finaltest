#pragma once
#include "cocos2d.h"
#include <vector>

class HighScoresScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(HighScoresScene);

    // --- 数据接口 ---

    // 静态保存分数 (自动读取旧数据、排序、保留前5名并回写)
    static void saveScore(int score);

private:
    // --- 交互回调 ---
    void onReturn(cocos2d::Ref* sender);

    // --- 内部实现 ---

    // 读取本地存储的分数列表
    static std::vector<int> getStoredScores();
};