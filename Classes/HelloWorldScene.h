#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    //初始化
    virtual bool init();
    
    // a selector callback 选择回调函数
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    //宏定义+类名
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__
