#include "cocos2d.h"
#include "Player.h"
#include "MapLayer.h"
#include"GameBackground.h"

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(GameScene);

    void placeBomb();

private:
    GameBackground* _gameBG = nullptr; //持有背景层的引用
    MapLayer* _mapLayer = nullptr;
    Player* _player = nullptr;

    // 键盘状态
    bool keyW = false;
    bool keyS = false;
    bool keyA = false;
    bool keyD = false;

    float speed = 120.0f; // 玩家速度

    void handleInput(float dt);
};

