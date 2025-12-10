#pragma once
#include "cocos2d.h"
#include "Item.h"

class ItemManager : public cocos2d::Node
{
public:
    CREATE_FUNC(ItemManager);

    void spawnItem(const cocos2d::Vec2& worldPos, Item::ItemType type);

    void update(float dt) override;

    void setPlayer(cocos2d::Node* player) { _player = player; }

private:
    cocos2d::Node* _player = nullptr;
    std::vector<Item*> _items;
};
