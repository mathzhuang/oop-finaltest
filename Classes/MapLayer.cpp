#include "MapLayer.h"
#include "cocos2d.h"
#include "Classes/ItemManager.h"

USING_NS_CC;

//================================================
// 初始化
//================================================
bool MapLayer::init()
{
    if (!Layer::init())
        return false;

    initMapData();

    // ===============================
    // 地图尺寸 & 位置（你原来的）
    // ===============================
    float mapW = WIDTH * TILE_SIZE;
    float mapH = HEIGHT * TILE_SIZE;
    this->setContentSize(Size(mapW, mapH));

    auto win = Director::getInstance()->getWinSize();
    float UI_WIDTH = 610.0f;

    this->setPosition(Vec2(
        UI_WIDTH,
        (win.height - mapH) * 0.5f
    ));

    // ===============================
    // 创建地图两层
    // ===============================
    _groundLayer = Node::create();
    _wallLayer = Node::create();

    this->addChild(_groundLayer, 0); // 草地
    this->addChild(_wallLayer, 1);   // 墙体

    initGround();
    initWalls();

    debugDrawGrid(); // 调试可关

    return true;
}

//================================================
// 初始化地图数据
//================================================
void MapLayer::initMapData()
{
    const int spawnAreaSize = 1; // 出生点周围一圈保护
    const int hardWallRate = 20; // 硬墙概率 %
    const int softWallRate = 40; // 软墙概率 %
    // 空地概率 = 100 - hardWallRate - softWallRate

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            // 边界全部硬墙
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1)
            {
                mapData[x][y] = 1;
                continue;
            }

            // 出生点保护区
            if (isSpawnArea(x, y))
            {
                mapData[x][y] = 0;
                continue;
            }

            // 随机生成硬墙 / 软墙 / 空地
            int r = RandomHelper::random_int(1, 100);
            if (r <= hardWallRate)
                mapData[x][y] = 1; // 硬墙
            else if (r <= hardWallRate + softWallRate)
                mapData[x][y] = 2; // 软墙
            else
                mapData[x][y] = 0; // 空地
        }
    }
}

void MapLayer::generateRandomMap()
{
    float softWallRate = 0.4f; // 软墙比例（30~50 都合理）

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            // ① 最外圈：硬墙
            if (x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT - 1)
            {
                mapData[x][y] = 1;
                continue;
            }

            // ② 内部棋盘硬墙（经典 Bomberman）
            if (x % 2 == 0 && y % 2 == 0)
            {
                mapData[x][y] = 1;
                continue;
            }

            // ③ 出生点保护
            if (isSpawnArea(x, y))
            {
                mapData[x][y] = 0;
                continue;
            }

            // ④ 随机软墙 / 地面
            float r = RandomHelper::random_real(0.0f, 1.0f);
            mapData[x][y] = (r < softWallRate) ? 2 : 0;
        }
    }
}
bool MapLayer::isSpawnArea(int x, int y)
{
    // 左上角玩家
    if ((x == 1 && y == 1) ||
        (x == 2 && y == 1) ||
        (x == 1 && y == 2))
        return true;

    // 右下角 AI / 玩家
    if ((x == WIDTH - 2 && y == HEIGHT - 2) ||
        (x == WIDTH - 3 && y == HEIGHT - 2) ||
        (x == WIDTH - 2 && y == HEIGHT - 3))
        return true;

    return false;
}

//================================================
// 返回格子类型
//================================================

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

            grass->setScale(
                TILE_SIZE / grass->getContentSize().width
            );

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
Sprite* MapLayer::createWallSprite(int type)
{
    std::string filename;

    if (type == 1)
        filename = "wall_iron.png";
    else if (type == 2)
        filename = "wall_soft.png";
    else
        return nullptr;

    auto sp = Sprite::create(filename);
    if (!sp) return nullptr;

    sp->setScale(
        TILE_SIZE / sp->getContentSize().width
    );

    return sp;
}
void MapLayer::destroySoftWall(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT)
        return;

    if (mapData[gx][gy] != 2) // 只炸软墙
        return;

    mapData[gx][gy] = 0;

    auto wall = wallSprites[gx][gy];
    if (wall)
    {
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

    // 统一坐标系：把道具直接加到 Scene 层
    auto scene = this->getParent();
    if (scene)
    {
        auto itemMgr = scene->getChildByName<ItemManager*>("ItemManager");
        if (itemMgr)
        {
            Vec2 worldPos = gridToWorld(gx, gy); // world 坐标
            itemMgr->dropItem(worldPos);
        }
    }
}
void MapLayer::removeWallAt(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT) return;

    if (wallSprites[gx][gy])
    {
        wallSprites[gx][gy]->removeFromParent();
        wallSprites[gx][gy] = nullptr;
    }
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
int MapLayer::getTile(int gx, int gy)
{
    if (gx < 0 || gx >= WIDTH || gy < 0 || gy >= HEIGHT)
        return 1; // 超出边界视作不可破坏墙
    return mapData[gx][gy];
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

    return Vec2(x, y) + this->getPosition();

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

bool MapLayer::isNearSoftWall(const Vec2& grid) const
{
    std::vector<Vec2> dirs = {
        Vec2(1,0), Vec2(-1,0), Vec2(0,1), Vec2(0,-1)
    };

    for (auto d : dirs)
    {
        int nx = grid.x + d.x;
        int ny = grid.y + d.y;

        if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT)
            continue;

        if (mapData[nx][ny] == 2) // 软墙
            return true;
    }
    return false;
}



