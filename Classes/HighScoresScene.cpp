#include "HighScoresScene.h"
#include "StartScene.h"
#include <algorithm> // 用于 std::sort
#include <string>

USING_NS_CC;

Scene* HighScoresScene::createScene()
{
    return HighScoresScene::create();
}

bool HighScoresScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 背景 (你需要一张 highscores.png，如果没有，先用 startBackground 代替)
    // 建议你准备一张图片放入 Resources/UI 文件夹
    auto bg = Sprite::create("UI/highScoresScene.png"); // 暂时复用选择界面的背景
    if (bg) {
        bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        // 适配屏幕
        bg->setScaleX(visibleSize.width / bg->getContentSize().width);
        bg->setScaleY(visibleSize.height / bg->getContentSize().height);
        this->addChild(bg, 0);
    }

    //// 2. 标题
    //auto title = Label::createWithSystemFont("HIGH SCORES", "Arial", 100);
    //title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.85f));
    //title->setColor(Color3B::YELLOW);
    //title->enableOutline(Color4B::BLACK, 2);
    //this->addChild(title, 1);

    // 3. 读取并显示分数
    std::vector<int> scores = getStoredScores();

    for (int i = 0; i < scores.size(); ++i)
    {
        // 格式： 1. 5000
        std::string scoreStr = StringUtils::format("%d.   %d", i + 1, scores[i]);

        auto label = Label::createWithSystemFont(scoreStr, "Arial", 100);
        label->setColor(Color3B::WHITE);
        label->setPosition(Vec2(visibleSize.width / 2, 1000 - (i * 100))); // 每行间隔100
        this->addChild(label, 1);
    }

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

void HighScoresScene::onReturn(Ref* sender)
{
    auto scene = StartScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

// ---------------------------------------------------------
// 核心逻辑：读取分数
// ---------------------------------------------------------
std::vector<int> HighScoresScene::getStoredScores()
{
    std::vector<int> scores;
    auto userDefault = UserDefault::getInstance();

    // 假设我们要存前 5 名
    for (int i = 0; i < 5; ++i)
    {
        std::string key = StringUtils::format("HighScore%d", i);
        int val = userDefault->getIntegerForKey(key.c_str(), -1); // -1 代表没存过
        if (val >= 0) {
            scores.push_back(val);
        }
    }
    return scores;
}

// ---------------------------------------------------------
// 核心逻辑：保存分数
// ---------------------------------------------------------
void HighScoresScene::saveScore(int newScore)
{
    if (newScore <= 0) return; // 0分不存

    // 1. 获取旧分数
    std::vector<int> scores = getStoredScores();

    // 2. 加入新分数
    scores.push_back(newScore);

    // 3. 排序 (从大到小)
    std::sort(scores.begin(), scores.end(), [](int a, int b) {
        return a > b;
        });

    // 4. 截取前 5 名
    if (scores.size() > 5) {
        scores.resize(5);
    }

    // 5. 写回 UserDefault
    auto userDefault = UserDefault::getInstance();
    for (int i = 0; i < scores.size(); ++i)
    {
        std::string key = StringUtils::format("HighScore%d", i);
        userDefault->setIntegerForKey(key.c_str(), scores[i]);
    }
    userDefault->flush(); // 强制写入文件
}