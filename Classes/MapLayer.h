#ifndef __MAP_LAYER_H__
#define __MAP_LAYER_H__

#include "cocos2d.h"

class MapLayer : public cocos2d::Layer
{
public:
    CREATE_FUNC(MapLayer);
    virtual bool init() override;

    // =========================
    // 地图逻辑接口
    // =========================

    // 是否可行走
    bool isWalkable(int gx, int gy);

    // 获取格子类型
    int getTile(int gx, int gy);

    // 设置格子类型（炸墙等）
    void setTile(int gx, int gy, int value);

    // 世界坐标 → 网格坐标
    cocos2d::Vec2 worldToGrid(const cocos2d::Vec2& pos);

    // 网格坐标 → 世界坐标
    cocos2d::Vec2 gridToWorld(int gx, int gy);

    // 炸毁软墙（给 Bomb / Flame 调用）
    void destroySoftWall(int gx, int gy);
    // 移除某个格子的墙体显示
    void removeWallAt(int gx, int gy);

    bool isNearSoftWall(const cocos2d::Vec2& grid) const;

    // =========================
    // 地图常量
    // =========================
    static const int WIDTH = 13;
    static const int HEIGHT = 13;
    static const int TILE_SIZE = 108;
    static const int TILE_EMPTY = 0;
    static const int TILE_IRON_WALL = 1;
    static const int TILE_SOFT_WALL = 2;
    static const int TILE_FLAME = 300; // 与你之前的 Tag 保持一致

private:
    
    // =========================
    // 地图数据（只管逻辑）
    // =========================
    int mapData[WIDTH][HEIGHT];

    // =========================
    // 显示层
    // =========================
    cocos2d::Node* _groundLayer = nullptr;   // 草地层
    cocos2d::Node* _wallLayer = nullptr;   // 墙体层

    cocos2d::Sprite* wallSprites[WIDTH][HEIGHT] = { nullptr };

    // =========================
    // 初始化函数
    // =========================
    void initMapData();
    void initGround();
    void initWalls();
    cocos2d::Sprite* createWallSprite(int type);

    // 调试网格
    void debugDrawGrid();

    //随机地图
    void generateRandomMap();
    bool isSpawnArea(int x, int y);
    void generateRandomMapSafe();
    void fixSpawnArea(int sx, int sy);
    void clearLineEscape(int x, int y);
    bool checkMapPlayable();
    bool hasEscapePath(const cocos2d::Vec2& start);
    int countReachableTiles(const cocos2d::Vec2& start);
};

#endif // __MAP_LAYER_H__
