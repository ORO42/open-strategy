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

CellSummary GetCellSummary(GameContext *gameContext, Vector2i cellIdx)
{
    CellSummary cellSummary;

    cellSummary.terrainLevel = gameContext->terrainLevels[cellIdx];

    if (gameContext->allUnits.find(cellIdx) != gameContext->allUnits.end())
    {
        entt::entity unitEntity = gameContext->allUnits[cellIdx];
        auto &unitComp = gameContext->registry.get<Unit>(unitEntity);

        cellSummary.unit = unitEntity;

        if (unitComp.stopsProjectile)
        {
            cellSummary.unitStopsProjectile = true;
        }

        if (unitComp.stance == Stances::STANDING || unitComp.stance == Stances::NONE)
        {
            cellSummary.unitHeight = unitComp.intrinsicHeight;
        }
        else if (unitComp.stance == Stances::CROUCHED)
        {
            cellSummary.unitHeight = unitComp.crouchHeight;
        }
        else if (unitComp.stance == Stances::PRONE)
        {
            cellSummary.unitHeight = unitComp.proneHeight;
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

        cellSummary.obstacleHeight = obstacleComp.intrinsicHeight;

        cellSummary.totalObstacleHeight = (cellSummary.terrainLevel * gameContext->cliffIntrinsicHeight) + obstacleComp.intrinsicHeight;
    }

    cellSummary.totalHeight = cellSummary.totalObstacleHeight + cellSummary.unitHeight;

    return cellSummary;
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

int GetUnitHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
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

int GetTopMostObstacleHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext) || gameContext->allObstacles.find(cellIdx) == gameContext->allObstacles.end())
    {
        return 0;
    }

    entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
    auto &obstacleComp = gameContext->registry.get<Unit>(obstacleEntity);
    return obstacleComp.intrinsicHeight;
}

int GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
    if (!CheckMouseInMapBounds(gameContext) || gameContext->allObstacles.find(cellIdx) == gameContext->allObstacles.end())
    {
        return 0;
    }

    int terrainHeight = GetTerrainHeightForCellIdx(gameContext, cellIdx);

    entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
    auto &obstacleComp = gameContext->registry.get<Obstacle>(obstacleEntity);
    return ((std::max(0, terrainHeight - 1) * gameContext->cliffIntrinsicHeight) + GetTopMostObstacleHeightForCellIdx(gameContext, cellIdx));
}

// int GetTotalHeightOfUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
// {
//     if (!CheckMouseInMapBounds(gameContext))
//     {
//         return 0;
//     }

//     return
// }

int GetTotalHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx)
{
}

Vector2i HasElevationLOS(const float &perCellWidthFeet, const Vector2i &observerCellIdx, const Vector2i &targetCellIdx, const Vector2i &betweenCellIdx)
{
}