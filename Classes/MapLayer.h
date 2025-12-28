#pragma once
#include "cocos2d.h"
#include "GameMode.h"

class MapLayer : public cocos2d::Layer
{
public:
    // --- 生命周期 ---
    CREATE_FUNC(MapLayer);
    virtual bool init() override;

    // --- 游戏模式设置 ---
    void setGameMode(GameMode mode) { _currentMode = mode; }
    GameMode getGameMode() const { return _currentMode; }

    // --- 坐标转换接口 ---
    cocos2d::Vec2 worldToGrid(const cocos2d::Vec2& pos);
    cocos2d::Vec2 gridToWorld(int gx, int gy);

    // --- 地图查询与逻辑 ---
    bool isWalkable(int gx, int gy);
    int  getTile(int gx, int gy);
    void setTile(int gx, int gy, int value);
    bool isNearSoftWall(const cocos2d::Vec2& grid) const;

    // --- 地图交互 ---
    // 炸毁软墙 (调用 ItemManager 掉落道具)
    void destroySoftWall(int gx, int gy);
    // 仅移除墙体显示 (不触发掉落逻辑)
    void removeWallAt(int gx, int gy);

    // --- 地图常量 ---
    static const int WIDTH = 13;
    static const int HEIGHT = 13;
    static const int TILE_SIZE = 108;

    // 格子类型定义
    static const int TILE_EMPTY = 0;
    static const int TILE_IRON_WALL = 1;
    static const int TILE_SOFT_WALL = 2;
    static const int TILE_FLAME = 300;

private:
    // --- 内部数据 ---
    int mapData[WIDTH][HEIGHT];
    GameMode _currentMode = GameMode::SINGLE;

    // --- 渲染组件 ---
    cocos2d::Node* _groundLayer = nullptr;
    cocos2d::Node* _wallLayer = nullptr;
    cocos2d::Sprite* wallSprites[WIDTH][HEIGHT] = { nullptr };

private:
    // --- 初始化流程 ---
    void initMapData();
    void initGround();
    void initWalls();
    cocos2d::Sprite* createWallSprite(int type);
    void debugDrawGrid();

    // --- 地图生成算法 ---
    bool isSpawnArea(int x, int y);
    void fixSpawnArea(int sx, int sy);
    void clearLineEscape(int x, int y);
    bool checkMapPlayable();
    int  countReachableTiles(const cocos2d::Vec2& start);
};