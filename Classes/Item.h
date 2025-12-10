#pragma once

#include "cocos2d.h"

class Item : public cocos2d::Sprite
{
public:
    // 定义物品类型枚举
    enum Type
    {
        TYPE_A,
        TYPE_B,
        TYPE_C
        // 可根据需要添加更多类型
    };

    // 工厂方法声明
    static Item* createItem(Type type);

    // 类型成员变量
    Type _type;
};
