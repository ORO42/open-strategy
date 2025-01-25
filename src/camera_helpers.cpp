#include "camera_helpers.h"

void sCameraKeyInput(GameContext *gameContext)
{
    const float moveSpeed = gameContext->cameraMoveSpeed;

    if (IsKeyDown(KEY_W))
    {
        gameContext->camera.target.y -= moveSpeed;
    }

    if (IsKeyDown(KEY_S))
    {
        gameContext->camera.target.y += moveSpeed;
    }

    if (IsKeyDown(KEY_A))
    {
        gameContext->camera.target.x -= moveSpeed;
    }

    if (IsKeyDown(KEY_D))
    {
        gameContext->camera.target.x += moveSpeed;
    }

    const float zoomIncrement = 0.2f;
    const float minZoom = 0.1f;

    if (IsKeyPressed(KEY_EQUAL))
    {
        gameContext->camera.zoom += zoomIncrement;
    }

    if (IsKeyPressed(KEY_MINUS))
    {
        gameContext->camera.zoom = fmaxf(gameContext->camera.zoom - zoomIncrement, minZoom);
    }
}