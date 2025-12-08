#ifndef __START_SCENE_H__
#define __START_SCENE_H__

#include "cocos2d.h"

class StartScene : public cocos2d::Scene
{
public:
    // 创建场景的静态方法
    static cocos2d::Scene* createScene();

    // 初始化方法
    virtual bool init();

    // 宏：自动实现 create() 方法
    CREATE_FUNC(StartScene);

private:
    // 声明三个按钮的回调函数
    void onButton1Click(cocos2d::Ref* pSender);
    void onButton2Click(cocos2d::Ref* pSender);
    void onButton3Click(cocos2d::Ref* pSender);
};

#endif // __START_SCENE_H__