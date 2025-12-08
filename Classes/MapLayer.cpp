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
    debugDrawGrid();

    return true;
}

//================================================
// 初始化地图数据
// 0 = 空地
// 1 = 固砖（永远不能炸）
// 2 = 软砖（可炸）
//================================================
void MapLayer::initMapData()
{
    // 你之后可以直接从 JSON / TMX / txt 加载
    // 现在给一份简易可用的 13x13 基础泡泡堂地图
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
        return 1;   // 出界当成墙
    return mapData[gx][gy];
}

//================================================
// 设置格子（比如炸弹炸掉砖块）
//================================================
void MapLayer::setTile(int gx, int gy, int value)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT)
        return;
    mapData[gx][gy] = value;
}

//================================================
// 判断玩家能不能走
// 0 = 空地可走
// 1 = 固砖不可走
// 2 = 软砖不可走（可炸）
//================================================
bool MapLayer::isWalkable(int gx, int gy)
{
    return getTile(gx, gy) == 0;
}

//================================================
// 世界 → 网格
//================================================
Vec2 MapLayer::worldToGrid(const Vec2& pos)
{
    int gx = pos.x / TILE_SIZE;
    int gy = pos.y / TILE_SIZE;
    return Vec2(gx, gy);
}

//================================================
// 网格 → 世界（用于放炸弹、道具）
//================================================
Vec2 MapLayer::gridToWorld(int gx, int gy)
{
    float x = gx * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = gy * TILE_SIZE + TILE_SIZE * 0.5f;
    return Vec2(x, y);
}

//================================================
// 网格线调试（你以后可以关掉）
//================================================
void MapLayer::debugDrawGrid()
{
    DrawNode* node = DrawNode::create();
    this->addChild(node);

    Color4F color(0, 1, 0, 0.2f);

    for (int x = 0; x <= WIDTH; x++)
    {
        node->drawLine(
            Vec2(x * TILE_SIZE, 0),
            Vec2(x * TILE_SIZE, HEIGHT * TILE_SIZE),
            color
        );
    }

    for (int y = 0; y <= HEIGHT; y++)
    {
        node->drawLine(
            Vec2(0, y * TILE_SIZE),
            Vec2(WIDTH * TILE_SIZE, y * TILE_SIZE),
            color
        );
    }

    // ★ 关键：地图尺寸
    this->setContentSize(Size(WIDTH * TILE_SIZE, HEIGHT * TILE_SIZE));

    // ★ 关键：地图居中
    auto win = Director::getInstance()->getWinSize();
    this->setPosition(Vec2(
        (win.width - this->getContentSize().width) * 0.5f,
        (win.height - this->getContentSize().height) * 0.5f
    ));
}


