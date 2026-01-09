#include "GameMap/GameMapManager.h"

using namespace ax;

ax::FastTMXTiledMap* GameMapManager::init(GameWorld* gameWorld, std::string mapName)
{
    game_world_ = gameWorld;

    // map_ = TMXTiledMap::create("map3.tmx");
    map_ = TMXTiledMap::create(mapName);
    map_->setPosition(Vec2::ZERO);
    // game_world_->addChild(map_, 0);

    tile_size_   = map_->getTileSize();
    auto tiles_f = map_->getMapSize();
    map_tiles_.x = tiles_f.x;
    map_tiles_.y = tiles_f.y;
    map_size_    = ax::Vec2(tile_size_.x * map_tiles_.x, tile_size_.y * map_tiles_.y);

    auto layer = map_->getLayer("layer0");

    int grid_size = map_tiles_.x * map_tiles_.y;
    map_grid_.resize(grid_size, false);
    for (int i = 0; i < grid_size; i++)
    {
        int tile_y = i / map_tiles_.x;
        int tile_x = i % map_tiles_.x;

        auto tile_id = layer->getTileGIDAt(Vec2(tile_x, tile_y));

        if (tile_id == ground_id)
            map_grid_[i] = true;
        else if (tile_id == water_id)
            map_grid_[i] = false;
        else
            map_grid_[i] = false;
    }

    if (path_finder_)
        delete path_finder_;
    path_finder_ = new PathFinder_FlowField();
    path_finder_->init(map_grid_, map_tiles_.x, map_tiles_.y);

    return map_;
}

void GameMapManager::setDestForMassivePath(const ax::Vec2& dst)
{
    int dx = dst.x / tile_size_.x;
    int dy = (map_size_.y - dst.y) / tile_size_.y;
    PathFinderStrategy::Point d(dx, dy);
    path_finder_->setDest(d);
}

std::optional<std::vector<ax::Vec2>> GameMapManager::getMassivePath(const ax::Vec2& src)
{
    int sx = src.x / tile_size_.x;
    int sy = (map_size_.y - src.y) / tile_size_.y;
    PathFinderStrategy::Point s(sx, sy);

    auto res = path_finder_->findPath(s);
    if (!res.has_value())
        return std::nullopt;

    std::vector<ax::Vec2> ret;
    ret.reserve(res.value().size());
    for (const auto& p : res.value())
    {
        ret.push_back(transTileToPixel(p.x, p.y));
    }

    return ret;
}

std::optional<std::vector<ax::Vec2>> GameMapManager::getSinglePath(const ax::Vec2& src, const ax::Vec2& dst)
{
    int sx = src.x / tile_size_.x;
    int sy = (map_size_.y - src.y) / tile_size_.y;

    int dx = dst.x / tile_size_.x;
    int dy = (map_size_.y - dst.y) / tile_size_.y;

    PathFinderStrategy::Point s(sx, sy);
    PathFinderStrategy::Point d(dx, dy);

    path_finder_->setDest(d);
    auto res = path_finder_->findPath(s);
    if (!res.has_value())
        return std::nullopt;

    std::vector<ax::Vec2> ret;
    ret.reserve(res.value().size());
    for (const auto& p : res.value())
    {
        ret.push_back(transTileToPixel(p.x, p.y));
    }

    return ret;
}

ax::Vec2 GameMapManager::getRandomNearbyIdleTilePos(const ax::Vec2& src)
{
    int sx = src.x / tile_size_.x;
    int sy = (map_size_.y - src.y) / tile_size_.y;

    static constexpr int nx[5] = {1, -1, 0, 0, 0};
    static constexpr int ny[5] = {0, 0, 1, -1, 0};

    int rd = ax::random<int>(0, 4);
    for (int i = 0; i < 5; i++)
    {
        int id = (rd + i) % 5;
        int x  = sx + nx[id];
        int y  = sy + ny[id];
        if (isTileValid(x, y))
            // return transTileToPixel(x, y);
            return transTileToRandomPixel(x, y);
    }

    return src;
}

ax::Vec2 GameMapManager::transTileToPixel(int tileX, int tileY)
{
    float x = (tileX + 0.5) * tile_size_.x;
    float y = map_size_.y - (tileY + 0.5) * tile_size_.y;
    // float x = tileX * tile_size_.x;
    // float y = map_size_.y - tileY * tile_size_.y;
    return ax::Vec2(x, y);
}

ax::Vec2 GameMapManager::transTileToRandomPixel(int tileX, int tileY)
{
    float x = (tileX + ax::random<float>(0.0f, 1.0f)) * tile_size_.x;
    float y = map_size_.y - (tileY + ax::random<float>(0.0f, 1.0f)) * tile_size_.y;
    return ax::Vec2(x, y);
}

bool GameMapManager::isTileValid(int tileX, int tileY)
{
    return tileX >= 0 && tileX < map_tiles_.x && tileY >= 0 && tileY < map_tiles_.y &&
           map_grid_[tileX + tileY * map_tiles_.x];
}