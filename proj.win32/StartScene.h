#pragma once
#ifndef __START_SCENE_H__
#define __START_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class StartScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;

    CREATE_FUNC(StartScene);

private:
    // 三个按钮
    cocos2d::ui::Button* _startButton;
    cocos2d::ui::Button* _settingButton;
    cocos2d::ui::Button* _exitButton;

    // 初始化UI
    void initUI();

    // 创建背景
    void createBackground();

    // 创建图片按钮
    void createImageButtons();

    // 创建单个图片按钮
    cocos2d::ui::Button* createImageButton(const std::string& normalImage,
        const std::string& pressedImage,
        const cocos2d::Vec2& position,
        const std::string& buttonName);

    void addButtonLabels();

    // 按钮回调函数
    void onStartGame(cocos2d::Ref* sender);
    void onSetting(cocos2d::Ref* sender);
    void onExit(cocos2d::Ref* sender);

    // 添加按钮点击效果
    void addButtonEffect(cocos2d::ui::Button* button);

    // 设计分辨率常量
    static constexpr float DESIGN_WIDTH = 2048.0f;
    static constexpr float DESIGN_HEIGHT = 1537.0f;
};

#endif // __START_SCENE_H__