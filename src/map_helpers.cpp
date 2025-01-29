#include "map_helpers.h"
#include "util_helpers.h"
#include "math_helpers.h"
#include "obstacle_helpers.h"
#include "unit_helpers.h"
#include "networking_helpers.h"

void BuildMap(GameContext *gameContext)
{
    std::string mapName = gameContext->gameSetup["mode_config"]["selected_map"];
    // nlohmann::json mapData = LoadJsonFromFile("maps/" + mapName + ".json"); TODO use this one once .json is stripped out of incoming mapName
    nlohmann::json mapData = LoadJsonFromFile("maps/" + mapName);
    nlohmann::json cellData = mapData["cell_data"];

    gameContext->currentMap = mapName;
    gameContext->mapWidth = mapData["meta"]["map_dimensions"]["map_width"];
    gameContext->mapHeight = mapData["meta"]["map_dimensions"]["map_height"];

    for (auto it = cellData.begin(); it != cellData.end(); ++it)
    {
        const std::string &key = it.key();
        Vector2 cellIdxVector2 = Vector2StringToVector2(key);
        Vector2i cellIdxVector2i = Vector2ToVector2i(cellIdxVector2);
        const nlohmann::json &value = it.value();

        std::string atlasCoordsString = value["cell_atlas_coords"];
        Vector2 atlasCoords = Vector2StringToVector2(atlasCoordsString);
        int atlasId = value["cell_source_id"];
        nlohmann::json templateData = gameContext->GetObstacleTemplateByAtlasCoords(atlasId, atlasCoords);
        std::string type = templateData["type"];
        CreateObstacle(gameContext, type, cellIdxVector2i);
    }

    int running_height = 0;
    for (int y = 1; y < gameContext->mapHeight - 1; y++)
    {
        for (int x = 1; x < gameContext->mapWidth - 1; x++)
        {
            Vector2i mapPosition = {x, y};
            auto &obstacle = gameContext->registry.get<Obstacle>(gameContext->allObstacles[mapPosition]);
            std::string type = obstacle.type;
            nlohmann::json obstacleTemplate = gameContext->obstacleTemplates[type];
            bool incrementTerrainHeight = obstacleTemplate["increment_terrain_height"];
            bool decrementTerrainHeight = obstacleTemplate["decrement_terrain_height"];
            bool continueTerrainHeight = obstacleTemplate["continue_terrain_height"];
            if (incrementTerrainHeight)
            {
                running_height++;
            }

            gameContext->terrainLevels[mapPosition] = running_height;

            if (decrementTerrainHeight)
            {
                running_height--;
            }
        }
    }
}

bool CheckMouseInMapBounds(GameContext *gameContext)
{
    Rectangle mapRect = {0, 0, static_cast<float>(gameContext->mapWidth * gameContext->cellWidth), static_cast<float>(gameContext->mapHeight * gameContext->cellHeight)};
    return CheckCollisionPointRec(GetScreenToWorld2D(GetMousePosition(), gameContext->camera), mapRect);
}

void Startup(GameContext *gameContext)
{
    std::string configSelectedMap = gameContext->gameSetup["mode_config"]["selected_map"];
    std::string configLoadSave = gameContext->gameSetup["mode_config"]["load_save"];
    std::string configConnectTo = gameContext->gameSetup["mode_config"]["connect_to"];
    if (configSelectedMap.size() > 0)
    {
        std::cout << "Creating new game" << std::endl;

        // Initialize ENet
        InitEnet();

        // Create a host (server)
        enetHost = CreateEnetHost(12345, 2, 2);

        // Connect my peer
        enetPeer = ConnectToEnetPeer(enetHost, "127.0.0.1", 12345, 2);
        if (enetPeer == NULL)
        {
            std::cout << "Host failed to connect" << std::endl;
            CleanupEnetHost(enetHost);
        }

        BuildMap(gameContext);
        CreateUnit(gameContext, "rifleman", {2, 2}, Teams::TEAM_BLUE);
        CreateUnit(gameContext, "rifleman", {4, 2}, Teams::TEAM_RED);

        return;
    }

    if (configLoadSave.size() > 0)
    {
        std::cout << "Loading new game" << std::endl;
        return;
    }

    if (configConnectTo.size() > 0)
    {
        std::cout << "Connecting to networked game" << std::endl;
        // Initialize ENet
        InitEnet();

        // Create a client host (connect to another player)
        enetHost = CreateEnetHost(0, 1, 2); // 0 means don't need to listen

        // Connect to the provided IP
        enetPeer = ConnectToEnetPeer(enetHost, configConnectTo.c_str(), 12345, 2);
        if (!enetPeer)
        {
            std::cerr << "Failed to connect to " << configConnectTo << "\n";
            CleanupEnetHost(enetHost);
            return;
        }

        // Wait for connection event (this is a blocking call until it connects)
        ENetEvent event;
        if (enet_host_service(enetHost, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            std::cout << "Successfully connected to " << configConnectTo << "\n";
        }
        else
        {
            std::cerr << "Connection to " << configConnectTo << " failed.\n";
            enet_peer_reset(enetPeer);
            CleanupEnetHost(enetHost);
            return;
        }
        return;
    }
    std::cout << "All game setup config options are empty. Aborting game startup." << std::endl;
}

////////////////////////////////
// Extrusion-based Line-of-Sight
////////////////////////////////

int GetTerrainLevelForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext))
    {
        return 0;
    }
    return gameContext->terrainLevels[cellIdx];
}

int GetTerrainHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext))
    {
        return 0;
    }
    return gameContext->terrainLevels[cellIdx] * gameContext->cliffIntrinsicHeight;
}

int GetUnitIntrinsicHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext) || gameContext->allUnits.find(cellIdx) == gameContext->allUnits.end())
    {
        return 0;
    }

    entt::entity unitEntity = gameContext->allUnits[cellIdx];
    auto &unitComp = gameContext->registry.get<Unit>(unitEntity);
    switch (unitComp.stance)
    {
    case Stances::CROUCHED:
        return unitComp.crouchHeight;
        break;
    case Stances::STANDING:
        return unitComp.intrinsicHeight;
        break;
    case Stances::PRONE:
        return unitComp.proneHeight;
        break;
    case Stances::NONE:
        return unitComp.intrinsicHeight;
        break;
    default:
        std::cout << "Stance not handled by switch" << std::endl;
    }
}

int GetTopMostObstacleIntrinsicHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext) || gameContext->allObstacles.find(cellIdx) == gameContext->allObstacles.end())
    {
        return 0;
    }

    entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
    auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);
    return obstacleComp.intrinsicHeight;
}

int GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext) || gameContext->allObstacles.find(cellIdx) == gameContext->allObstacles.end())
    {
        return 0;
    }

    entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
    auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);

    int terrainLevel = GetTerrainLevelForCellIdx(gameContext, cellIdx);
    int terrainHeight = GetTerrainHeightForCellIdx(gameContext, cellIdx);
    if (obstacleComp.displayName == "cliff" || obstacleComp.displayName == "wall")
    {
        return terrainHeight;
    }

    // If there is an obstacle other than cliff or wall
    return terrainHeight + GetTopMostObstacleIntrinsicHeightForCellIdx(gameContext, cellIdx);
}

int GetTotalHeightOfUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext) || gameContext->allUnits.find(cellIdx) == gameContext->allUnits.end())
    {
        return 0;
    }

    if (gameContext->allObstacles.find(cellIdx) != gameContext->allObstacles.end())
    {
        entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
        auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);
        if (obstacleComp.unitStandsOnTop)
        {
            return GetUnitIntrinsicHeightForCellIdx(gameContext, cellIdx) + GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(gameContext, cellIdx);
        }
    }

    return GetTerrainHeightForCellIdx(gameContext, cellIdx) + GetUnitIntrinsicHeightForCellIdx(gameContext, cellIdx);
}

int GetTotalHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext))
    {
        return 0;
    }

    if (gameContext->allUnits.find(cellIdx) != gameContext->allUnits.end())
    {
        entt::entity unitEntity = gameContext->allUnits[cellIdx];
        auto &unitcomp = gameContext->registry.get<Unit>(unitEntity);

        return GetTotalHeightOfUnitForCellIdx(gameContext, cellIdx);
    }
    else
    {
        if (gameContext->allObstacles.find(cellIdx) != gameContext->allObstacles.end())
        {
            entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
            auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);

            return GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(gameContext, cellIdx);
        }
    }

    return GetTotalHeightOfUnitForCellIdx(gameContext, cellIdx) + GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(gameContext, cellIdx);
}

CellSummary GetCellSummary(GameContext *gameContext, const Vector2i &cellIdx)
{
    CellSummary cellSummary;

    cellSummary.terrainLevel = GetTerrainLevelForCellIdx(gameContext, cellIdx);
    cellSummary.terrainHeight = GetTerrainHeightForCellIdx(gameContext, cellIdx);
    cellSummary.unitIntrinsicHeight = GetUnitIntrinsicHeightForCellIdx(gameContext, cellIdx);
    cellSummary.topMostObstacleIntrinsicHeight = GetTopMostObstacleIntrinsicHeightForCellIdx(gameContext, cellIdx);
    cellSummary.totalHeightIncludingTopMostObstacleExcludingUnit = GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(gameContext, cellIdx);
    cellSummary.totalHeightofUnit = GetTotalHeightOfUnitForCellIdx(gameContext, cellIdx);
    cellSummary.totalHeightForCellIdx = GetTotalHeightForCellIdx(gameContext, cellIdx);

    if (gameContext->allUnits.find(cellIdx) != gameContext->allUnits.end())
    {
        entt::entity unitEntity = gameContext->allUnits[cellIdx];
        auto &unitcomp = gameContext->registry.get<Unit>(unitEntity);

        cellSummary.unit = unitEntity;

        if (unitcomp.stopsProjectile)
        {
            cellSummary.unitStopsProjectile = true;
        }
    }
    if (gameContext->allObstacles.find(cellIdx) != gameContext->allObstacles.end())
    {
        entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
        auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);

        cellSummary.obstacle = obstacleEntity;

        if (obstacleComp.stopsProjectile)
        {
            cellSummary.obstacleStopsProjectile = true;
        }
    }

    return cellSummary;
}

// Vector2i HasElevationLOS(GameContext *gameContext, const float &perCellWidthFeet, const Vector2i &observerCellIdx, const Vector2i &targetCellIdx, const Vector2i &betweenCellIdx)
// {
//     int totalHeightAtObserver = GetTotalHeightForCellIdx(gameContext, observerCellIdx);
//     int totalHeightAtTarget = GetTotalHeightForCellIdx(gameContext, targetCellIdx);
//     int totalHeightAtBetweenCell = GetTotalHeightForCellIdx(gameContext, betweenCellIdx);

//     Vector2 tlCornerObserverRect2D = MapToWorld(observerCellIdx, gameContext->cellWidth, gameContext->cellHeight);
//     Vector2 tlCornerBetweenRect2D = MapToWorld(betweenCellIdx, gameContext->cellWidth, gameContext->cellHeight);
//     Vector2 tlCornerTargetRect2D = MapToWorld(targetCellIdx, gameContext->cellWidth, gameContext->cellHeight);

//     // Check if observer and target are adjacent
//     if (GetChebyshevDistance(observerCellIdx, targetCellIdx) == 1)
//     {
//         return Vector2i{-1, -1};
//     }

//     // TODO:
//     // 1. Project total height at observer to a cube that is 3.28 feet wide and totalHeightAtObserver tall.
//     // 2. Project total height at between cell to a cube that is 3.28 feet wide and totalHeightAtBetweenCell tall.
//     // 3. Project total height at target to a cube that is 3.28 feet wide and totalHeightAtTarget tall.
//     // 4. Create a 3d line raycast between the top of the observer cube and the top of the target cube
//     // 5. If the 3d raycast intersects the between cube, return betweenCellIdx, else return {-1, -1}

//     // If the between cell is not blocking, return {-1, -1}
//     return Vector2i{-1, -1};
// }

Vector2i HasElevationLOS(GameContext *gameContext, const float &perCellWidthFeet, const Vector2i &observerCellIdx, const Vector2i &targetCellIdx, const Vector2i &betweenCellIdx)
{
    // Check if observer and target are adjacent
    if (GetChebyshevDistance(observerCellIdx, targetCellIdx) == 1)
    {
        return Vector2i{-1, -1};
    }

    int totalHeightAtObserver = GetTotalHeightForCellIdx(gameContext, observerCellIdx);
    int totalHeightAtTarget = GetTotalHeightForCellIdx(gameContext, targetCellIdx);
    int totalHeightAtBetweenCell = GetTotalHeightForCellIdx(gameContext, betweenCellIdx);

    Vector2 tlCornerObserverRect2D = MapToWorld(observerCellIdx, gameContext->cellWidth, gameContext->cellHeight);
    Vector2 tlCornerBetweenRect2D = MapToWorld(betweenCellIdx, gameContext->cellWidth, gameContext->cellHeight);
    Vector2 tlCornerTargetRect2D = MapToWorld(targetCellIdx, gameContext->cellWidth, gameContext->cellHeight);

    // Project 2D cells to 3D cubes
    BoundingBox observerBox = CreateGridCellBoundingBox(tlCornerObserverRect2D.x, tlCornerObserverRect2D.y, gameContext->cellWidth, gameContext->cellHeight, totalHeightAtObserver);
    BoundingBox betweenBox = CreateGridCellBoundingBox(tlCornerBetweenRect2D.x, tlCornerBetweenRect2D.y, gameContext->cellWidth, gameContext->cellHeight, totalHeightAtBetweenCell);
    BoundingBox targetBox = CreateGridCellBoundingBox(tlCornerTargetRect2D.x, tlCornerTargetRect2D.y, gameContext->cellWidth, gameContext->cellHeight, totalHeightAtTarget);

    // Cast ray from top center of observerBox to top center of targetBox
    Vector3 observerTopCenter;
    observerTopCenter.x = observerBox.min.x + (gameContext->cellWidth / 2);
    observerTopCenter.y = observerBox.min.y + (gameContext->cellHeight / 2);
    observerTopCenter.z = observerBox.max.z;

    Vector3 targetTopCenter;
    targetTopCenter.x = targetBox.min.x + (gameContext->cellWidth / 2);
    targetTopCenter.y = targetBox.min.y + (gameContext->cellHeight / 2);
    targetTopCenter.z = targetBox.max.z;

    Ray ray;
    ray.position = observerTopCenter;
    ray.direction = MyVector3Normalize(MyVector3Subtract(targetTopCenter, observerTopCenter));

    RayCollision collision = GetRayCollisionBox(ray, betweenBox);

    if (collision.hit)
    {
        return betweenCellIdx;
    }

    // If the between cell is not blocking, return {-1, -1}
    return Vector2i{-1, -1};
}