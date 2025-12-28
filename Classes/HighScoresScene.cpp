#include "HighScoresScene.h"
#include "StartScene.h"
#include <algorithm> // 用于 std::sort
#include <string>

USING_NS_CC;

// --- 生命周期 ---

Scene* HighScoresScene::createScene()
{
    return HighScoresScene::create();
}

bool HighScoresScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 初始化背景
    // 使用适配屏幕的方式拉伸背景
    auto bg = Sprite::create("UI/highScoresScene.png");
    if (bg) {
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        bg->setScaleX(visibleSize.width / bg->getContentSize().width);
        bg->setScaleY(visibleSize.height / bg->getContentSize().height);
        this->addChild(bg, 0);
    }

    // 2. 读取历史高分
    std::vector<int> scores = getStoredScores();

    // 3. 遍历显示分数列表
    for (int i = 0; i < scores.size(); ++i)
    {
        // 格式化字符串： "1.   5000"
        std::string scoreStr = StringUtils::format("%d.   %d", i + 1, scores[i]);

        auto label = Label::createWithSystemFont(scoreStr, "Arial", 100);
        label->setColor(Color3B::WHITE);
        // 从Y轴 900 开始，每行间隔 100
        label->setPosition(Vec2(visibleSize.width / 2, 900 - (i * 100)));
        this->addChild(label, 1);
    }

    // 空记录处理
    if (scores.empty()) {
        auto label = Label::createWithSystemFont("No Records Yet", "Arial", 100);
        label->setPosition(Vec2(visibleSize.width / 2, 686.8f));
        this->addChild(label, 1);
    }

    // 4. 返回按钮
    auto returnBtn = MenuItemImage::create(
        "UI/gamereturn.png",
        "UI/gamereturn-after.png",
        CC_CALLBACK_1(HighScoresScene::onReturn, this));

    returnBtn->setPosition(Vec2(visibleSize.width * 0.9f, visibleSize.height * 0.1f));

    auto menu = Menu::create(returnBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    return true;
}

// --- 交互回调 ---

void HighScoresScene::onReturn(Ref* sender)
{
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// --- 数据持久化逻辑 ---

std::vector<int> HighScoresScene::getStoredScores()
{
    std::vector<int> scores;
    auto userDefault = UserDefault::getInstance();

    // 读取前5名记录 (Key: HighScore0 ~ HighScore4)
    for (int i = 0; i < 5; ++i)
    {
        std::string key = StringUtils::format("HighScore%d", i);
        int val = userDefault->getIntegerForKey(key.c_str(), -1);

        if (val >= 0) {
            scores.push_back(val);
        }
    }
    return scores;
}

void HighScoresScene::saveScore(int newScore)
{
    if (newScore <= 0) return; // 忽略无效分数

    // 1. 读取旧数据并加入新分数
    std::vector<int> scores = getStoredScores();
    scores.push_back(newScore);

    // 2. 降序排序
    std::sort(scores.begin(), scores.end(), [](int a, int b) {
        return a > b;
        });

    // 3. 截断保留前5名
    if (scores.size() > 5) {
        scores.resize(5);
    }

    // 4. 写入本地存储
    auto userDefault = UserDefault::getInstance();
    for (int i = 0; i < scores.size(); ++i)
    {
        std::string key = StringUtils::format("HighScore%d", i);
        userDefault->setIntegerForKey(key.c_str(), scores[i]);
    }
    userDefault->flush(); // 强制IO写入
}