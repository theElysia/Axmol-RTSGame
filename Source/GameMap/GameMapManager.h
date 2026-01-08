#pragma once

#include "axmol.h"
#include "GameMap/PathFinderStrategy.h"
#include "BaseTemplates.h"
#include <optional>

class GameWorld;

class GameMapManager : public Singleton<GameMapManager>
{
public:
    static constexpr int water_id  = 42;
    static constexpr int ground_id = 16;

public:
    ax::FastTMXTiledMap* init(GameWorld* gameWorld, std::string mapName);

    void setDestForMassivePath(const ax::Vec2& dst);
    std::optional<std::vector<ax::Vec2>> getMassivePath(const ax::Vec2& src);
    std::optional<std::vector<ax::Vec2>> getSinglePath(const ax::Vec2& src, const ax::Vec2& dst);
    ax::Vec2 getTileSize() { return tile_size_; }
    ax::Vec2 getMapSize() { return map_size_; }

    ax::Vec2 getRandomNearbyIdleTilePos(const ax::Vec2& src);
    ax::Vec2 transTileToPixel(int tileX, int tileY);

private:
    friend class Singleton<GameMapManager>;
    GameMapManager() = default;

    bool isTileValid(int tileX, int tileY);

private:
    GameWorld* game_world_    = nullptr;
    ax::FastTMXTiledMap* map_ = nullptr;
    ax::Vec2 tile_size_, map_size_;  // pixel
    struct
    {
        int x, y;
    } map_tiles_;
    std::vector<bool> map_grid_;  // ground=true
    PathFinderStrategy* path_finder_ = nullptr;
};