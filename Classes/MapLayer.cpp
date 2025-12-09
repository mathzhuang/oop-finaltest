#include "MapLayer.h"

USING_NS_CC;

//================================================
// 初始化
//================================================
bool MapLayer::init()
{
    if (!Layer::init())
        return false;

    initMapData();

    // 计算地图像素大小
    float mapW = WIDTH * TILE_SIZE;
    float mapH = HEIGHT * TILE_SIZE;
    this->setContentSize(Size(mapW, mapH));

    // ★ 将地图放到屏幕中央
    auto win = Director::getInstance()->getWinSize();
    this->setPosition(Vec2(
        (win.width - mapW) * 0.5f,
        (win.height - mapH) * 0.5f
    ));

    debugDrawGrid();  // 可关

    return true;
}

//================================================
// 初始化地图数据
//================================================
void MapLayer::initMapData()
{
    int temp[HEIGHT][WIDTH] =
    {
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {0,0,0,0,2,0,2,0,2,0,0,0,0},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {0,0,0,0,2,0,2,0,2,0,0,0,0},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {0,0,0,2,2,2,0,2,2,2,0,0,0},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {0,0,0,2,2,2,0,2,2,2,0,0,0},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {0,0,0,0,2,0,2,0,2,0,0,0,0},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {0,0,0,0,2,0,2,0,2,0,0,0,0},
        {1,0,1,0,1,0,1,0,1,0,1,0,1}
    };

    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            mapData[x][y] = temp[y][x];
}

//================================================
// 返回格子类型
//================================================
int MapLayer::getTile(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT)
        return 1;   // 出界 = 墙
    return mapData[gx][gy];
}

//================================================
// 设置格子
//================================================
void MapLayer::setTile(int gx, int gy, int value)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT)
        return;
    mapData[gx][gy] = value;
}

//================================================
// 判断能不能走
//================================================
bool MapLayer::isWalkable(int gx, int gy)
{
    return getTile(gx, gy) == 0;
}

//================================================
// 世界 → 网格（注意考虑地图偏移）
//================================================
Vec2 MapLayer::worldToGrid(const Vec2& pos)
{
    Vec2 local = pos - this->getPosition();  // 地图偏移修正

    int gx = local.x / TILE_SIZE;
    int gy = local.y / TILE_SIZE;

    return Vec2(gx, gy);
}

//================================================
// 网格 → 世界（放炸弹、角色居中）
//================================================
Vec2 MapLayer::gridToWorld(int gx, int gy)
{
    float x = gx * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = gy * TILE_SIZE + TILE_SIZE * 0.5f;

    return Vec2(x, y) + this->getPosition();  // 加地图偏移
}

//================================================
// 网格线（调试）
//================================================
void MapLayer::debugDrawGrid()
{
    DrawNode* node = DrawNode::create();
    this->addChild(node);

    Color4F color(0, 1, 0, 0.2f);

    float w = WIDTH * TILE_SIZE;
    float h = HEIGHT * TILE_SIZE;

    for (int x = 0; x <= WIDTH; x++)
        node->drawLine(Vec2(x * TILE_SIZE, 0), Vec2(x * TILE_SIZE, h), color);

    for (int y = 0; y <= HEIGHT; y++)
        node->drawLine(Vec2(0, y * TILE_SIZE), Vec2(w, y * TILE_SIZE), color);
}





