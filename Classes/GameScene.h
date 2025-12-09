#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Player.h"
#include "MapLayer.h"
#include <vector>


class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(GameScene);

    void placeBomb();

public:
    // 公开 flames 以便 Bomb 访问
    std::vector<cocos2d::Sprite*> flames;

private:
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

#endif
