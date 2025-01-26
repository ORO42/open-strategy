#pragma once

#include "game_context.h"

void BuildMap(GameContext *gameContext);
void Startup(GameContext *gameContext);
bool CheckMouseInMapBounds(GameContext *gameContext);

int GetTerrainLevelForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
int GetTerrainHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
int GetUnitIntrinsicHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
int GetTopMostObstacleIntrinsicHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
int GetTotalHeightIncludingTopMostObstacleExcludingUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
int GetTotalHeightOfUnitForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
int GetTotalHeightForCellIdx(GameContext *gameContext, const Vector2i &cellIdx);
CellSummary GetCellSummary(GameContext *gameContext, const Vector2i &cellIdx);
Vector2i HasElevationLOS(GameContext *gameContext, const float &perCellWidthFeet, const Vector2i &observerCellIdx, const Vector2i &targetCellIdx, const Vector2i &betweenCellIdx);
