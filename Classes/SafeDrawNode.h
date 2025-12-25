#pragma once
#include "cocos2d.h"

class SafeDrawNode : public cocos2d::DrawNode
{
public:
    CREATE_FUNC(SafeDrawNode);
    virtual bool init() override { return DrawNode::init(); }

    void safeDrawSolidCircle(const cocos2d::Vec2& pos, float radius, int segments, const cocos2d::Color4F& color)
    {
        int maxSegments = 1000;               // 限制防止缓冲越界
        int seg = std::min(segments, maxSegments);
        DrawNode::drawSolidCircle(pos, radius, 0, seg, color);
    }

    void safeClear()
    {
        if (this->getParent() == nullptr) return; // 确保对象仍在场景树中
        DrawNode::clear();
    }

    void safeRemoveFromParent() {
        if (this->getParent() != nullptr) {
            this->removeFromParent();
            this->unscheduleAllCallbacks();
        }
    }

};
