#pragma once
#include "cocos2d.h"

class HowToPlayScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(HowToPlayScene);

private:
    // --- 交互回调 ---
    void onButtonClick(cocos2d::Ref* pSender);
};