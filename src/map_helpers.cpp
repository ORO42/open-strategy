#include "map_helpers.h"
#include "util_helpers.h"
#include "math_helpers.h"
#include "obstacle_helpers.h"
#include "unit_helpers.h"

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
        BuildMap(gameContext);
        CreateUnit(gameContext, "rifleman", {2, 2}, Teams::TEAM_BLUE);
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
    return (std::max(0, terrainLevel - 1) * gameContext->cliffIntrinsicHeight) + GetTopMostObstacleIntrinsicHeightForCellIdx(gameContext, cellIdx);
}

int GetTotalHeightOfUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext))
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

    // Get terrain height
    int terrainHeight = GetTerrainHeightForCellIdx(gameContext, cellIdx);

    // Initialize obstacle and unit heights
    int obstacleHeight = 0;
    int unitHeight = 0;

    // Check for obstacle at cellIdx
    if (gameContext->allObstacles.find(cellIdx) != gameContext->allObstacles.end())
    {
        entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
        auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);

        // If obstacle is a cliff or wall, terrainHeight should take precedence
        if (obstacleComp.displayName == "cliff" || obstacleComp.displayName == "wall")
        {
            obstacleHeight = terrainHeight; // Terrain height as obstacle height
        }
        else
        {
            obstacleHeight = GetTopMostObstacleIntrinsicHeightForCellIdx(gameContext, cellIdx);
        }

        // Add unit height if unit stands on top of the obstacle
        if (obstacleComp.unitStandsOnTop)
        {
            unitHeight = GetUnitIntrinsicHeightForCellIdx(gameContext, cellIdx);
        }

        // If terrainLevel is 0 and an obstacle exists, ignore terrain height
        if (gameContext->terrainLevels[cellIdx] == 0)
        {
            terrainHeight = 0;
        }
    }
    else
    {
        // No obstacle; only consider unit height
        unitHeight = GetUnitIntrinsicHeightForCellIdx(gameContext, cellIdx);
    }

    // Total height is terrain + obstacle + unit
    return terrainHeight + obstacleHeight + unitHeight;
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

    return cellSummary;
}

Vector2i HasElevationLOS(GameContext *gameContext, const float &perCellWidthFeet, const Vector2i &observerCellIdx, const Vector2i &targetCellIdx, const Vector2i &betweenCellIdx)
{
    int totalHeightAtObserver = GetTotalHeightForCellIdx(gameContext, observerCellIdx);
    int totalHeightAtTarget = GetTotalHeightForCellIdx(gameContext, targetCellIdx);
    int totalHeightAtBetweenCell = GetTotalHeightForCellIdx(gameContext, betweenCellIdx);

    // Check if observer and target are adjacent
    if (GetChebyshevDistance(observerCellIdx, targetCellIdx) == 1)
    {
        return Vector2i{-1, -1};
    }

    // Calculate the horizontal distance between observer and target
    float horizontalDistance = std::sqrt(
        std::pow((targetCellIdx.x - observerCellIdx.x) * perCellWidthFeet, 2) +
        std::pow((targetCellIdx.y - observerCellIdx.y) * perCellWidthFeet, 2));

    // Calculate the slope of the line of sight
    float slope = (totalHeightAtTarget - totalHeightAtObserver) / horizontalDistance;

    // Calculate the expected height at the betweenCellIdx based on the slope
    float distanceToBetweenCell = std::sqrt(
        std::pow((betweenCellIdx.x - observerCellIdx.x) * perCellWidthFeet, 2) +
        std::pow((betweenCellIdx.y - observerCellIdx.y) * perCellWidthFeet, 2));
    float expectedHeightAtBetweenCell = totalHeightAtObserver + slope * distanceToBetweenCell;

    // Check if the betweenCellIdx height is above the expected height (blocking LOS)
    if (totalHeightAtBetweenCell > expectedHeightAtBetweenCell)
    {
        return betweenCellIdx; // Blocking cell
    }

    // If the between cell is not blocking, return {-1, -1}
    return Vector2i{-1, -1};
}