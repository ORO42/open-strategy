#pragma once

#include "game_context.h"

void sDrawGameTextures(GameContext *gameContext);
void sDrawPlayerDetails(GameContext *gameContext);
void sDrawUnitDetails(GameContext *gameContext);
void sDrawHoveredCellInfo(GameContext *gameContext);
void sDrawNextTurnTip(GameContext *gameContext);
void sDrawSelectedUnitAbilities(GameContext *gameContext);
void sDrawSelectedUnitIndicator(GameContext *gameContext);
void sDrawHoveredCellIndicator(GameContext *gameContext);
void sDrawIndicatorLine(GameContext *gameContext);
void CreatePopupText(GameContext *gameContext, std::string text, Vector2 position, Color color, bool useFade, std::chrono::duration<double> maxDuration);
void sDrawPopupText(GameContext *gameContext);