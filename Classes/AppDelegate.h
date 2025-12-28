#pragma once
#include "cocos2d.h"

class AppDelegate : private cocos2d::Application
{
public:
    // --- 生命周期 ---
    AppDelegate();
    virtual ~AppDelegate();

    // --- 核心配置 ---

    // 配置 OpenGL 上下文属性
    virtual void initGLContextAttrs();

    // 应用启动初始化 (初始化导演、加载首场景)
    // return: true=成功继续; false=失败退出
    virtual bool applicationDidFinishLaunching();

    // --- 状态管理 ---

    // 应用进入后台 (暂停动画、音频)
    virtual void applicationDidEnterBackground();

    // 应用切回前台 (恢复动画、音频)
    virtual void applicationWillEnterForeground();
};