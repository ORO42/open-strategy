#pragma once

#include "game_context.h"

void CreateUnit(GameContext *gameContext, const std::string &type, const Vector2i &cellIdx, const Teams &team);
void sUnitSelection(GameContext *gameContext);
void sMoveUnits(GameContext *gameContext);