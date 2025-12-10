#include "ItemManager.h"
#include "Player.h"

using namespace cocos2d;

void ItemManager::update(float dt)
{
    if (!_player) return;

    Player* player = dynamic_cast<Player*>(_player);
    if (!player) return;

    float pickDist = 30.0f;

    for (int i = (int)_items.size() - 1; i >= 0; --i)
    {
        Item* item = _items[i];
        if (!item) continue;

        if (item->getPosition().distance(player->getPosition()) < pickDist)
        {
            // 先播放拾取动画，然后在回调里移除并通知 player
            item->playPickAnimation([this, player, item]() {
                // 从 _items 中移除
                _items.erase(std::remove(_items.begin(), _items.end(), item), _items.end());
                // 通知 player
                player->pickItem(item);
                // 删除节点
                item->removeFromParent();
                });
        }
    }
}
