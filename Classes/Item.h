#ifndef __ITEM_H__
#define __ITEM_H__

#include "cocos2d.h"

class Item : public cocos2d::Sprite
{
public:
    enum class Type
    {
        POWER,   // 增加炸弹数量
        RANGE,   // 增加爆炸范围
        SPEED    // 增加移速
    };

    static Item* createItem(Type type);

    Type getType() const { return _type; }

private:
    Type _type;
};

#endif
