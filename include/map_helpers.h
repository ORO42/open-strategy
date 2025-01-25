#pragma once

#include "game_context.h"

void BuildMap(GameContext *gameContext);
void Startup(GameContext *gameContext);
bool CheckMouseInMapBounds(GameContext *gameContext);
CellSummary GetCellSummary(GameContext *gameContext, Vector2i cellIdx);