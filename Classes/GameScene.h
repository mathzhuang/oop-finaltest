#pragma once
#include "cocos2d.h"
#include "GameMode.h"
#include "AIState.h"
#include "AIController.h"

#include <vector>

#include <queue>
#include <map>


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

    const std::vector<Player*>& getPlayers() const { return _players; }

    ItemManager* getItemManager() { return _itemManager; }
    

    // 👈 必须在 public
    MapLayer* getMapLayer() { return _mapLayer; }
    bool isGridDangerPublic(const cocos2d::Vec2& grid) { return isGridDanger(grid); }


    Player* findNearestPlayer(Player* self);
    std::vector<cocos2d::Vec2> findPathToPlayer(const cocos2d::Vec2& start, Player* target);
    std::vector<cocos2d::Vec2> findPathToItem(const cocos2d::Vec2& start);
    std::vector<cocos2d::Vec2> findPathToSoftWall(const cocos2d::Vec2& start);
    std::vector<cocos2d::Vec2> findSafePathBFS(const cocos2d::Vec2& start);
    bool hasSafeEscape(const cocos2d::Vec2& grid, Player* ai);

    // 判断玩家是否被角落困住（两边堵住或危险格）
    bool isPlayerCornered(Player* player);

    // 判断炸弹放下后是否会威胁到指定玩家（同一行/列 + 炸弹范围）
    bool willBombTrapPlayer(const cocos2d::Vec2& bombGrid, Player* target, int bombRange);


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
   

    void createAIPlayer(const cocos2d::Vec2& gridPos,
        int characterId,
        const std::string& name);

    bool isGridDanger(const cocos2d::Vec2& grid);
  

    // 通用 BFS
    std::vector<cocos2d::Vec2> findPathBFS(
        const cocos2d::Vec2& start,
        std::function<bool(const cocos2d::Vec2&)> isTarget,
        bool avoidDanger);

    // 功能封装
 


    std::vector<AIState> _aiStates;
    AIController* _aiController = nullptr;


    // =========================
    // 结束判定
    // =========================
    bool _gameOver = false;
    bool _canCheckGameOver = false;

    void checkGameOver();
    void onGameOver(Player* winner);



};
