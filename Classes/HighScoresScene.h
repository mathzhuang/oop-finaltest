#pragma once
#ifndef __HIGHSCORES_SCENE_H__
#define __HIGHSCORES_SCENE_H__

#include "cocos2d.h"
#include <vector>

class HighScoresScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(HighScoresScene);

    // 保存分数（供 GameOverLayer 调用）
    // 会自动读取旧分数，插入新分数，排序，保留前5名
    static void saveScore(int score);

private:
    void onReturn(cocos2d::Ref* sender);

    // 获取当前保存的分数列表
    static std::vector<int> getStoredScores();
};

#endif // __HIGHSCORES_SCENE_H__