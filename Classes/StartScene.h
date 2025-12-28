#pragma once
#include "cocos2d.h"

class StartScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(StartScene);

private:
    // --- 交互回调 ---
    void onButton1Click(cocos2d::Ref* pSender); // 开始游戏 (进入模式选择)
    void onButton2Click(cocos2d::Ref* pSender); // 游戏说明
    void onButton3Click(cocos2d::Ref* pSender); // 查看排行榜
};