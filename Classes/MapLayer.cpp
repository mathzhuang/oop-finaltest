#include "MapLayer.h"
#include "cocos2d.h"
#include "ItemManager.h"

USING_NS_CC;

// --- 生命周期与初始化 ---

bool MapLayer::init()
{
    if (!Layer::init()) return false;

    // 1. 初始化地图数据（随机生成）
    initMapData();

    // 2. 设置地图尺寸与位置
    float mapW = WIDTH * TILE_SIZE;
    float mapH = HEIGHT * TILE_SIZE;
    this->setContentSize(Size(mapW, mapH));

    auto win = Director::getInstance()->getWinSize();
    float UI_WIDTH = 610.0f;
    this->setPosition(Vec2(UI_WIDTH, (win.height - mapH) * 0.5f));

    // 3. 创建渲染层
    _groundLayer = Node::create();
    _wallLayer = Node::create();
    this->addChild(_groundLayer, 0); // 地板层
    this->addChild(_wallLayer, 1);   // 墙体层

    // 4. 填充精灵
    initGround();
    initWalls();

    // 5. 调试网格
    debugDrawGrid();

    return true;
}

void MapLayer::initMapData()
{
    const int hardWallRate = 20; // 硬墙 20%
    const int softWallRate = 40; // 软墙 40%

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            // 保护出生点
            if (isSpawnArea(x, y)) {
                mapData[x][y] = 0;
                continue;
            }

            // 随机生成
            int r = RandomHelper::random_int(1, 100);
            if (r <= hardWallRate)
                mapData[x][y] = 1; // 硬墙
            else if (r <= hardWallRate + softWallRate)
                mapData[x][y] = 2; // 软墙
            else
                mapData[x][y] = 0; // 空地
        }
    }

    // 修正四角连通性，保证玩家不被困死
    fixSpawnArea(1, 1);
    fixSpawnArea(1, HEIGHT - 2);
    fixSpawnArea(WIDTH - 2, 1);
    fixSpawnArea(WIDTH - 2, HEIGHT - 2);
}

void MapLayer::initGround()
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            auto grass = Sprite::create("ground_grass.png");
            grass->setPosition(
                x * TILE_SIZE + TILE_SIZE * 0.5f,
                y * TILE_SIZE + TILE_SIZE * 0.5f
            );
            grass->setScale(TILE_SIZE / grass->getContentSize().width);
            _groundLayer->addChild(grass);
        }
    }
}

void MapLayer::initWalls()
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            wallSprites[x][y] = nullptr;
            int type = mapData[x][y];
            if (type == 0) continue;

            auto wall = createWallSprite(type);
            if (!wall) continue;

            wall->setPosition(
                x * TILE_SIZE + TILE_SIZE * 0.5f,
                y * TILE_SIZE + TILE_SIZE * 0.5f
            );
            _wallLayer->addChild(wall);
            wallSprites[x][y] = wall;
        }
    }
}

// --- 坐标转换与查询接口 ---

Vec2 MapLayer::worldToGrid(const Vec2& pos)
{
    // 修正地图 Layer 的偏移量
    Vec2 local = pos - this->getPosition();
    int gx = local.x / TILE_SIZE;
    int gy = local.y / TILE_SIZE;
    return Vec2(gx, gy);
}

Vec2 MapLayer::gridToWorld(int gx, int gy)
{
    float x = gx * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = gy * TILE_SIZE + TILE_SIZE * 0.5f;
    // 加上地图 Layer 的偏移量
    return Vec2(x, y) + this->getPosition();
}

void MapLayer::setTile(int gx, int gy, int value)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT) return;
    mapData[gx][gy] = value;
}

int MapLayer::getTile(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT) return 1; // 越界视为硬墙
    return mapData[gx][gy];
}

bool MapLayer::isWalkable(int gx, int gy)
{
    return getTile(gx, gy) == 0;
}

bool MapLayer::isNearSoftWall(const Vec2& grid) const
{
    std::vector<Vec2> dirs = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };
    for (auto d : dirs) {
        int nx = grid.x + d.x;
        int ny = grid.y + d.y;
        if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT) continue;
        if (mapData[nx][ny] == 2) return true; // 发现软墙
    }
    return false;
}

// --- 地图生成辅助逻辑 ---

bool MapLayer::isSpawnArea(int x, int y)
{
    // 四角保护区判定
    if (x >= 1 && x <= 2 && y >= 1 && y <= 2) return true; // 左下
    if (x >= 1 && x <= 2 && y >= HEIGHT - 3 && y <= HEIGHT - 2) return true; // 左上
    if (x >= WIDTH - 3 && x <= WIDTH - 2 && y >= 1 && y <= 2) return true; // 右下
    if (x >= WIDTH - 3 && x <= WIDTH - 2 && y >= HEIGHT - 3 && y <= HEIGHT - 2) return true; // 右上
    return false;
}

void MapLayer::fixSpawnArea(int sx, int sy)
{
    // 1. 确保周围有足够的空间
    std::vector<Vec2> dirs = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };
    int walkable = 0;
    for (auto d : dirs) {
        if (getTile(sx + d.x, sy + d.y) == 0) walkable++;
    }

    for (auto d : dirs) {
        if (walkable >= 4) break;
        int nx = sx + d.x;
        int ny = sy + d.y;
        if (getTile(nx, ny) != 1) continue; // 不破坏硬墙，只把软墙变空地
        mapData[nx][ny] = 0;
        walkable++;
    }

    // 2. 确保直线通道畅通（方便炸墙）
    clearLineEscape(sx, sy);
}

void MapLayer::clearLineEscape(int x, int y)
{
    // 清除十字方向两格内的障碍
    if (getTile(x + 1, y) != 0 && getTile(x + 2, y) != 0) mapData[x + 1][y] = 0;
    if (getTile(x - 1, y) != 0 && getTile(x - 2, y) != 0) mapData[x - 1][y] = 0;
    if (getTile(x, y + 1) != 0 && getTile(x, y + 2) != 0) mapData[x][y + 1] = 0;
    if (getTile(x, y - 1) != 0 && getTile(x, y - 2) != 0) mapData[x][y - 1] = 0;
}

bool MapLayer::checkMapPlayable()
{
    int empty = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (mapData[x][y] == 0) empty++;
        }
    }
    return ((float)empty / (WIDTH * HEIGHT)) > 0.30f;
}

int MapLayer::countReachableTiles(const Vec2& start)
{
    // BFS 计算连通区域大小
    std::queue<Vec2> q;
    std::vector<std::vector<bool>> visited(WIDTH, std::vector<bool>(HEIGHT, false));
    q.push(start);
    visited[start.x][start.y] = true;
    int count = 1;

    std::vector<Vec2> dirs = { Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1) };
    while (!q.empty()) {
        Vec2 cur = q.front(); q.pop();
        for (auto d : dirs) {
            int nx = cur.x + d.x;
            int ny = cur.y + d.y;
            if (nx < 1 || nx >= WIDTH - 1 || ny < 1 || ny >= HEIGHT - 1) continue;
            if (visited[nx][ny]) continue;
            if (mapData[nx][ny] == 1) continue; // 硬墙阻断

            visited[nx][ny] = true;
            count++;
            q.push(Vec2(nx, ny));
        }
    }
    return count;
}

// --- 墙体管理 ---

Sprite* MapLayer::createWallSprite(int type)
{
    std::string filename;
    if (type == 1) filename = "wall_iron.png";
    else if (type == 2) filename = "wall_soft.png";
    else return nullptr;

    auto sp = Sprite::create(filename);
    if (!sp) return nullptr;
    sp->setScale(TILE_SIZE / sp->getContentSize().width);
    return sp;
}

void MapLayer::removeWallAt(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT) return;
    if (wallSprites[gx][gy]) {
        wallSprites[gx][gy]->removeFromParent();
        wallSprites[gx][gy] = nullptr;
    }
}

void MapLayer::destroySoftWall(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT) return;
    if (mapData[gx][gy] != 2) return; // 只炸软墙

    // 1. 更新数据
    mapData[gx][gy] = 0;

    // 2. 播放销毁动画
    auto wall = wallSprites[gx][gy];
    if (wall) {
        auto boom = Spawn::create(
            ScaleTo::create(0.2f, 0.0f),
            FadeOut::create(0.2f),
            nullptr
        );
        wall->runAction(Sequence::create(
            boom,
            RemoveSelf::create(),
            nullptr
        ));
        wallSprites[gx][gy] = nullptr;
    }

    // 3. 触发道具掉落 (调用 ItemManager)
    auto scene = this->getParent();
    if (scene) {
        auto itemMgr = scene->getChildByName<ItemManager*>("ItemManager");
        if (itemMgr) {
            Vec2 worldPos = gridToWorld(gx, gy);
            itemMgr->dropItem(worldPos); // 注意：这里传的是 WorldPos
        }
    }
}

// --- 调试工具 ---

void MapLayer::debugDrawGrid()
{
    DrawNode* node = DrawNode::create();
    this->addChild(node);
    Color4F color(0, 1, 0, 0.2f); // 淡绿色

    float w = WIDTH * TILE_SIZE;
    float h = HEIGHT * TILE_SIZE;

    for (int x = 0; x <= WIDTH; x++)
        node->drawLine(Vec2(x * TILE_SIZE, 0), Vec2(x * TILE_SIZE, h), color);

    for (int y = 0; y <= HEIGHT; y++)
        node->drawLine(Vec2(0, y * TILE_SIZE), Vec2(w, y * TILE_SIZE), color);
}