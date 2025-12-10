#ifndef __MAP_LAYER_H__
#define __MAP_LAYER_H__

#include "cocos2d.h"

class MapLayer : public cocos2d::Layer
{
public:
    CREATE_FUNC(MapLayer);
    virtual bool init() override;

    // 你之后需要：判断能不能走
    bool isWalkable(int gx, int gy);

    // 获取格子类型，后续地图 brick/iron/item 都能加
    int getTile(int gx, int gy);

    // 设置格子（比如炸弹爆炸后把砖块打掉）
    void setTile(int gx, int gy, int value);

    // 世界坐标 → 网格坐标
    cocos2d::Vec2 worldToGrid(const cocos2d::Vec2& pos);

    // 网格坐标 → 世界坐标（放炸弹、放道具都会用）
    cocos2d::Vec2 gridToWorld(int gx, int gy);



private:
    static const int WIDTH = 13;
    static const int HEIGHT = 13;
    static const int TILE_SIZE = 108;  // 后面你换美术素材也方便调

    // 地图数据
    int mapData[WIDTH][HEIGHT];

    // 初始化数据
    void initMapData();

    // 调试：画地格（可关）
    void debugDrawGrid();
};

#endif
