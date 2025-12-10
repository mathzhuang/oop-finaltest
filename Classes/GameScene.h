#include "cocos2d.h"
#include "Player.h"
#include "MapLayer.h"

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(GameScene);

    void placeBomb();

private:
    MapLayer* _mapLayer = nullptr;
    Player* _player = nullptr;

    // ¼üÅÌ×´Ì¬
    bool keyW = false;
    bool keyS = false;
    bool keyA = false;
    bool keyD = false;

    float speed = 120.0f; // Íæ¼ÒËÙ¶È

    void handleInput(float dt);
};

