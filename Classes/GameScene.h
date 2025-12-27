#pragma once
#include "cocos2d.h"
#include "GameMode.h"
#include "AIState.h"
#include "AIController.h"
#include "FogManager.h"

#include <vector>
#include <functional> // 必须包含这个
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

// 1. 升级 BombDanger 结构体，添加判定逻辑
struct BombDanger
{
    cocos2d::Vec2 bombGrid;
    int range;
    float timeLeft;

    // 统一判定逻辑：给 AI 避灾用，也给原来的 isGridDanger 使用
    bool willExplodeGrid(const cocos2d::Vec2& targetGrid) const {
        // 同行且在威力范围内
        if (targetGrid.x == bombGrid.x && std::abs(targetGrid.y - bombGrid.y) <= range)
            return true;
        // 同列且在威力范围内
        if (targetGrid.y == bombGrid.y && std::abs(targetGrid.x - bombGrid.x) <= range)
            return true;
        return false;
    }
};
class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;
    //static GameScene* createWithMode(GameMode mode, int characterId1 = -1, int characterId2 = -1);
    static GameScene* createWithMode(GameMode mode, int p1Face, int p2Face = 0);

    const std::vector<Player*>& getPlayers() const { return _players; }

    ItemManager* getItemManager() { return _itemManager; }
    

    // 👈 必须在 public
    MapLayer* getMapLayer() { return _mapLayer; }
   // 2. 添加 getter 接口供 AIController 使用
    const std::vector<BombDanger>& getActiveBombs() const { return _bombDangers; }

    // 3. 统一危险判定接口 (建议将 isGridDanger 改为这个，或者二合一)
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

    std::vector<BombDanger> _bombDangers;
    
    void registerBomb(const cocos2d::Vec2& grid, int range);
    void updateBombDangers(float dt);

    std::vector<cocos2d::Vec2> findSmartPath(const cocos2d::Vec2& start, const cocos2d::Vec2& target, bool avoidDanger);
private:
    // =========================
    // 核心组件
    // =========================
    GameBackground* _gameBG = nullptr;
    MapLayer* _mapLayer = nullptr;
    ItemManager* _itemManager = nullptr;
    GameMode _gameMode = GameMode::SINGLE;
    FogManager* _fogManager = nullptr;

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

    void setupPlayers(GameMode mode, int p1Face, int p2Face);

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
  

    // 修改声明，确保与报错信息要求的格式对齐
    std::vector<cocos2d::Vec2> findPathBFS(
        const cocos2d::Vec2& start,
        std::function<bool(const cocos2d::Vec2&)> isTarget,
        bool avoidDanger
    );

    
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
    void GameScene::onExit();



};
