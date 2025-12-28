#pragma once
#include "cocos2d.h"

class moduleScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(moduleScene);

private:
    // --- 交互回调 ---
    void onButton1Click(cocos2d::Ref* pSender); // 单人模式
    void onButton2Click(cocos2d::Ref* pSender); // 双人模式
    void onButton3Click(cocos2d::Ref* pSender); // 迷雾模式
    void onButton4Click(cocos2d::Ref* pSender); // 返回主页
};