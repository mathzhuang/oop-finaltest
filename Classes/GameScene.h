#pragma once
#include "cocos2d.h"
#include "GameMode.h"

#include <vector>

// ===== 前向声明 =====
class Player;
class MapLayer;
class ItemManager;
class GameBackground;

struct AIInput
{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool bomb = false;

    float thinkTimer = 0.0f;
};

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;
    static GameScene* createWithMode(GameMode mode, int characterId1 = -1, int characterId2 = -1);

    // 👈 必须在 public


private:
    // =========================
    // 核心组件
    // =========================
    GameBackground* _gameBG = nullptr;
    MapLayer* _mapLayer = nullptr;
    ItemManager* _itemManager = nullptr;
    GameMode _gameMode = GameMode::SINGLE;

    // =========================
    // 玩家容器（支持单人 / 双人 / AI）
    // =========================
    std::vector<Player*> _players;

    float speed = 120.0f;

    // =========================
    // 键盘状态
    // =========================
    bool keyW = false, keyS = false, keyA = false, keyD = false, keyBomb1 = false;
    bool keyUp = false, keyDown = false, keyLeft = false, keyRight = false, keyBomb2 = false;

    // 玩家角色ID
    int _player1CharacterId = 1;
    int _player2CharacterId = 2;

   

    // =========================
    // 初始化
    // =========================
    void initKeyboard();
    void initPlayers();
    void createLocalPlayer(const cocos2d::Vec2& gridPos, int characterId,const std::string& name);


    // =========================
    // 输入 & 逻辑
    // =========================
    void handleInput(float dt);
    void handlePlayerMove(
        Player* player,
        bool up, bool down, bool left, bool right,
        bool& bombKey,
        float dt
    );

    void checkFlameHit(Player* player);
    void checkItemPickup(Player* player);

    // ===== AI =====
    void updateAI(float dt);
    void thinkForAI(int aiIndex, Player* ai, float dt);

    void createAIPlayer(const cocos2d::Vec2& gridPos,
        int characterId,
        const std::string& name);

    bool isGridDanger(const cocos2d::Vec2& grid);

    struct AIState
    {
        float thinkCooldown = 0.0f;
        cocos2d::Vec2 nextDir = cocos2d::Vec2::ZERO;
        bool wantBomb = false;
    };

    std::vector<AIState> _aiStates;


    // =========================
    // 结束判定
    // =========================
    bool _gameOver = false;
    bool _canCheckGameOver = false;

    void checkGameOver();
    void onGameOver(Player* winner);



};
