#include "destruction_helpers.h"
#include "obstacle_helpers.h"
#include "unit_helpers.h"

void sDestroyGameObjects(GameContext *gameContext)
{
    // Handle unit destruction
    bool didDestroyUnit = false;
    auto unitView = gameContext->registry.view<Unit>();
    for (auto &entity : unitView)
    {
        auto &unitComp = gameContext->registry.get<Unit>(entity);
        if (unitComp.currentHealth <= 0)
        {
            didDestroyUnit = true;
            if (entity == gameContext->selectedUnit)
            {
                gameContext->selectedUnit = entt::null;
            }
            gameContext->allUnits.erase(unitComp.cellIdx);
            gameContext->registry.destroy(entity);
        }
    }

    if (didDestroyUnit)
    {
        ComputeMyTeamsVision(gameContext);
    }

    // Handle obstacle destruction
    auto obstacleView = gameContext->registry.view<Obstacle>();
    for (auto &entity : obstacleView)
    {
        auto &obstacleComp = gameContext->registry.get<Obstacle>(entity);
        if (obstacleComp.isDestructible && obstacleComp.currentHealth <= 0)
        {
            gameContext->allObstacles.erase(obstacleComp.cellIdx);
            CreateObstacle(gameContext, "ground", obstacleComp.cellIdx);
            gameContext->registry.destroy(entity);
        }
    }
}