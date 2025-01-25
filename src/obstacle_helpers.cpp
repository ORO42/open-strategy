#include "obstacle_helpers.h"

void CreateObstacle(GameContext *gameContext, const std::string &type, const Vector2i &cellIdx)
{
    entt::entity obstacleEntity = gameContext->registry.create();
    gameContext->allObstacles[cellIdx] = obstacleEntity;

    Obstacle newObstacle;
    newObstacle.atlasId = gameContext->obstacleTemplates[type]["atlas_id"];
    newObstacle.atlasCoords = Vector2i{gameContext->obstacleTemplates[type]["atlas_coords"]["x"], gameContext->obstacleTemplates[type]["atlas_coords"]["y"]};
    newObstacle.type = type;
    newObstacle.cellIdx = cellIdx;
    newObstacle.displayName = gameContext->obstacleTemplates[type]["display_name"];
    newObstacle.intrinsicHeight = gameContext->obstacleTemplates[type]["intrinsic_height"];
    newObstacle.unitStandsOnTop = gameContext->obstacleTemplates[type]["unit_stands_on_top"];
    newObstacle.stopsProjectile = gameContext->obstacleTemplates[type]["stops_projectile"];
    newObstacle.isDestructible = gameContext->obstacleTemplates[type]["is_destructible"];
    newObstacle.maxHealth = gameContext->obstacleTemplates[type]["max_health"];
    newObstacle.currentHealth = gameContext->obstacleTemplates[type]["max_health"];
    newObstacle.moveCostSupplies = gameContext->obstacleTemplates[type]["move_cost"];

    gameContext->registry.emplace<Obstacle>(obstacleEntity, newObstacle);
}