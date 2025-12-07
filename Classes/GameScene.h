#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Player.h"

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void update(float dt) override;

    CREATE_FUNC(GameScene);

private:
    cocos2d::Sprite* player;

    bool keyW = false;
    bool keyS = false;
    bool keyA = false;
    bool keyD = false;

    float speed = 150.0f;

    void placeBomb();
};

#endif
