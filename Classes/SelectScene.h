#pragma once
#ifndef __SELECT_SCENE_H__
#define __SELECT_SCENE_H__

#include "cocos2d.h"

// 使用 Cocos2d 命名空间
USING_NS_CC;

class SelectScene : public cocos2d::Scene
{
public:
    // 静态创建场景的方法
    static cocos2d::Scene* createScene();

    // 虚函数 init()，用于初始化场景
    virtual bool init();

    // Cocos2d-x 的常用宏，用于创建实例
    CREATE_FUNC(SelectScene);

private:
    // 成员变量
    cocos2d::Sprite* _arrowSprite;            //选择箭头 Sprite
    std::vector<cocos2d::Vec2> _characterPositions; // 存储四个角色上方箭头的目标位置
    int _currentSelectedIndex;                // 当前选中的角色索引 (0 到 3)

    // 键盘事件回调函数
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);

    // 自定义方法，用于执行平滑移动和更新索引
    void moveArrowTo(int newIndex);
};

#endif // __SELECT_SCENE_H__
