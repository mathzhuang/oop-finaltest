#pragma once
#ifndef __HOWTOPLAY_SCENE_H__
#define __HOWTOPLAY_SCENE_H__

#include "cocos2d.h"

class HowToPlayScene : public cocos2d::Scene
{
public:
    // 创建场景的静态方法
    static cocos2d::Scene* createScene();

    // 初始化方法
    virtual bool init();

    // 宏：自动实现 create() 方法
    CREATE_FUNC(HowToPlayScene);

private:
    // 声明按钮回调函数
    void onButtonClick(cocos2d::Ref* pSender);

};

#endif // __START_SCENE_H__