#pragma once
#include "cocos2d.h"
#include "GameMode.h"
#include "AIController.h"
#include "FogManager.h"
#include <vector>
#include <functional>
#include <queue>
#include <map>

// 前向声明
class Player;
class MapLayer;
class ItemManager;
class GameBackground;

// AI 输入状态结构
struct AIInput
{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool bomb = false;
    float thinkTimer = 0.0f;
};

// 炸弹威胁数据结构
struct BombDanger
{
    cocos2d::Vec2 bombGrid;
    int range;
    float timeLeft;

    // 判定目标格子是否在爆炸范围内
    bool willExplodeGrid(const cocos2d::Vec2& targetGrid) const {
        if (targetGrid.x == bombGrid.x && std::abs(targetGrid.y - bombGrid.y) <= range)
            return true;
        if (targetGrid.y == bombGrid.y && std::abs(targetGrid.x - bombGrid.x) <= range)
            return true;
        return false;
    }
};

class GameScene : public cocos2d::Scene
{
public:
    // --- 生命周期 ---
    static cocos2d::Scene* createScene();
    static GameScene* createWithMode(GameMode mode, int p1Face, int p2Face = 0, GameDifficulty diff = GameDifficulty::EASY);

    virtual bool init() override;
    virtual void update(float dt) override;
    virtual void onExit() override;

    // --- 数据访问接口 ---
    MapLayer* getMapLayer() { return _mapLayer; }
    ItemManager* getItemManager() { return _itemManager; }
    const std::vector<Player*>& getPlayers() const { return _players; }

    // 获取当前所有活跃炸弹预警 (AI用)
    const std::vector<BombDanger>& getActiveBombs() const { return _bombDangers; }

    // --- 游戏逻辑接口 ---
    // 注册炸弹信息 (Player调用)
    void registerBomb(const cocos2d::Vec2& grid, int range);
    // 更新 UI (分数/道具)
    void updateUIForPlayer(Player* p);
    // 危险格判定 (公开接口)
    bool isGridDangerPublic(const cocos2d::Vec2& grid) { return isGridDanger(grid); }

    // --- AI 寻路与战术辅助 ---
    Player* findNearestPlayer(Player* self);

    // A* 与 BFS 路径搜索
    std::vector<cocos2d::Vec2> findSmartPath(const cocos2d::Vec2& start, const cocos2d::Vec2& target, bool avoidDanger);
    std::vector<cocos2d::Vec2> findPathToPlayer(const cocos2d::Vec2& start, Player* target);
    std::vector<cocos2d::Vec2> findPathToItem(const cocos2d::Vec2& start);
    std::vector<cocos2d::Vec2> findPathToSoftWall(const cocos2d::Vec2& start);
    std::vector<cocos2d::Vec2> findSafePathBFS(const cocos2d::Vec2& start);

    // 战术判断
    bool hasSafeEscape(const cocos2d::Vec2& grid, Player* ai);
    bool isPlayerCornered(Player* player);
    bool willBombTrapPlayer(const cocos2d::Vec2& bombGrid, Player* target, int bombRange);

    // --- 音频静态配置 ---
    static bool s_isAudioOn;
    static int s_menuAudioID;
    static int s_gameAudioID;

private:
    // --- 核心组件 ---
    GameBackground* _gameBG = nullptr;
    MapLayer* _mapLayer = nullptr;
    ItemManager* _itemManager = nullptr;
    FogManager* _fogManager = nullptr;
    AIController* _aiController = nullptr;

    // --- 游戏状态数据 ---
    GameMode _gameMode = GameMode::SINGLE;
    GameDifficulty _difficulty = GameDifficulty::EASY;

    std::vector<Player*> _players;
    std::vector<BombDanger> _bombDangers; // 炸弹预警列表
    std::vector<AIState> _aiStates;       // AI 状态机

    // 预设 ID
    int _player1CharacterId = 1;
    int _player2CharacterId = 2;

    float speed = 120.0f;

    // --- 输入状态 ---
    bool keyW = false, keyS = false, keyA = false, keyD = false, keyBomb1 = false;
    bool keyUp = false, keyDown = false, keyLeft = false, keyRight = false, keyBomb2 = false;

    // --- 游戏流程标志 ---
    bool _gameOver = false;
    bool _canCheckGameOver = false;

private:
    // --- 内部初始化 ---
    void initPlayers();
    void initKeyboard();
    void createLocalPlayer(const cocos2d::Vec2& gridPos, int characterId, const std::string& name);
    void createAIPlayer(const cocos2d::Vec2& gridPos, int characterId, const std::string& name);

    // --- 内部逻辑更新 ---
    void handleInput(float dt);
    void handlePlayerMove(Player* player, bool up, bool down, bool left, bool right, bool& bombKey, float dt);

    void updateAI(float dt);
    void updateBombDangers(float dt);

    // --- 碰撞与交互 ---
    void checkFlameHit(Player* player);
    void checkItemPickup(Player* player);
    bool isGridDanger(const cocos2d::Vec2& grid);

    // --- 结算逻辑 ---
    void checkGameOver();
    void onGameOver(Player* winner);
};