#include "SelectScene.h"
#include"GameScene.h"
#include <vector>

#define LOG_CONFIRM(...) cocos2d::log(__VA_ARGS__)
// 宏定义用于方便地获取屏幕尺寸
#define WIN_SIZE Director::getInstance()->getWinSize()

// --- 场景创建方法 ---
cocos2d::Scene* SelectScene::createScene()
{
    return SelectScene::create();
}

// --- 初始化方法 ---
bool SelectScene::init()
{
    if (!Scene::init()) {
        return false;
    }


    // 1. 定义角色位置和初始状态
    float charY =1076.9f;

    // 四个角色水平等距排列，根据您的实际背景图进行微调
    _characterPositions = {
        Vec2(239.7f, charY),    // 角色 1 (左侧)
        Vec2(741.6f, charY),    // 角色 2
        Vec2(1263.8f, charY),    // 角色 3
        Vec2(1801.3f, charY)     // 角色 4 (右侧)
    };

    _currentSelectedIndex = 0; // 默认选择第一个角色

    // ===================================
    // 2. 添加背景和角色贴图
    // ===================================

    // **注意:** 请替换为您的背景图资源名称
    auto background = Sprite::create("UI/selectbackground.png");
    if (background) {
        background->setPosition(WIN_SIZE / 2);
        // 确保背景覆盖整个屏幕，如果您的图片尺寸与屏幕不一致
        background->setScaleX(WIN_SIZE.width / background->getContentSize().width);
        background->setScaleY(WIN_SIZE.height / background->getContentSize().height);
        this->addChild(background, 0);
    }

    // **注意:** 您的描述是“角色贴纸已经在selectbackground中贴好”，
    // 因此这里不再单独添加角色 Sprite。如果角色是单独的 Sprite，
    // 您应该在这里或背景上添加它们。

    // ===================================
    // 3. 创建和初始化箭头 Sprite
    // ===================================

    // **注意:** 请替换为您的箭头 Sprite 资源名称
    _arrowSprite = Sprite::create("UI/arrow.png");
    if (_arrowSprite) {
        // 设置初始位置在第一个角色上方
        _arrowSprite->setPosition(_characterPositions[_currentSelectedIndex]);
        this->addChild(_arrowSprite, 10); // 确保箭头在最上层
    }

    // ===================================
    // 4. 注册键盘事件监听
    // ===================================
    auto listener = EventListenerKeyboard::create();
    // 绑定 onKeyPressed 成员函数作为回调
    listener->onKeyPressed = CC_CALLBACK_2(SelectScene::onKeyPressed, this);
    // 将监听器添加到事件分发器，以便开始监听按键事件
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// --- 键盘事件处理函数 ---
void SelectScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    int newIndex = _currentSelectedIndex;

    // 处理向左选择
    if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW) {
        // 使用取模运算实现循环选择：(当前索引 - 1 + 总数) % 总数
        newIndex = (_currentSelectedIndex - 1 + (int)_characterPositions.size()) % (int)_characterPositions.size();

        // 处理向右选择
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) {
        // 使用取模运算实现循环选择：(当前索引 + 1) % 总数
        newIndex = (_currentSelectedIndex + 1) % (int)_characterPositions.size();
    }

    //  确认选择 (Enter / Space)
    else if (keyCode == EventKeyboard::KeyCode::KEY_ENTER ||
        keyCode == EventKeyboard::KeyCode::KEY_SPACE)
    {
        // ** 确认选择逻辑 **

        LOG_CONFIRM("角色选择已确认！选中角色索引: %d", _currentSelectedIndex);

        // 获取选中的角色编号 (1-based index)
        int characterId = _currentSelectedIndex + 1;

        // TODO: 
        // 1. 在这里执行保存角色选择的操作 (例如，保存到 UserDefault)
        // 2. 切换到下一个游戏场景，并将 characterId 传递过去
        // 例如：
        
        auto GameScene = GameScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(1.0f, GameScene));
        

        return; // 确认操作后，不再执行移动检查
    }
         // 只有当索引发生变化时，才执行移动操作
    if (newIndex != _currentSelectedIndex) {
        moveArrowTo(newIndex);
    }
}

// --- 平滑移动箭头到目标位置 ---
void SelectScene::moveArrowTo(int newIndex)
{
    // 更新当前选中索引
    _currentSelectedIndex = newIndex;

    // 1. 获取目标位置
    Vec2 targetPos = _characterPositions[_currentSelectedIndex];

    // 2. 创建 MoveTo 动作，设置移动时间为 0.25 秒
    auto moveTo = MoveTo::create(0.25f, targetPos);

    // 3. 使用 EaseSineOut 缓动函数来创建平滑移动效果
    // EaseSineOut：开始快，逐渐减速至目标位置
    auto easeAction = EaseSineOut::create(moveTo);

    // 4. 停止 Sprite 当前正在执行的所有动作，防止快速按键冲突
    _arrowSprite->stopAllActions();

    // 5. 运行平滑动作
    _arrowSprite->runAction(easeAction);

    // 可选：播放音效
    // SimpleAudioEngine::getInstance()->playEffect("select_change.mp3");
}