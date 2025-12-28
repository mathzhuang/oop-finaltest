#pragma once
#include "cocos2d.h"

class SafeDrawNode : public cocos2d::DrawNode
{
public:
    // --- 生命周期 ---
    CREATE_FUNC(SafeDrawNode);
    virtual bool init() override { return DrawNode::init(); }

    // --- 安全操作 ---

    // 安全移除：先取消调度器再移除，防止回调悬空
    void safeRemoveFromParent()
    {
        if (this->getParent() != nullptr) {
            this->unscheduleAllCallbacks();
            this->removeFromParent();
        }
    }

    // 安全清理：确保对象在树中才执行 clear
    void safeClear()
    {
        if (this->getParent() == nullptr) return;
        DrawNode::clear();
    }

    // --- 绘图接口 ---

    // 安全绘制实心圆：限制最大段数，防止顶点缓冲区溢出导致 Crash
    void safeDrawSolidCircle(const cocos2d::Vec2& pos, float radius, int segments, const cocos2d::Color4F& color)
    {
        int maxSegments = 1000;
        int seg = std::min(segments, maxSegments);
        DrawNode::drawSolidCircle(pos, radius, 0, seg, color);
    }
};