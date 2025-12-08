#include "Item.h"

USING_NS_CC;

Item* Item::createItem(Type type)
{
    Item* item = new (std::nothrow) Item();
    if (item && item->initWithFile("item_placeholder.png"))
    {
        item->autorelease();
        item->_type = type;
        return item;
    }

    CC_SAFE_DELETE(item);
    return nullptr;
}
