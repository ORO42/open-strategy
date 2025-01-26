#include "ui_helpers.h"
#include "math_helpers.h"
#include "map_helpers.h"

void sDrawGameTextures(GameContext *gameContext)
{
    Rectangle viewportRect = gameContext->GetCameraViewportWorldRect();
    std::vector<Vector2i> cellIdxsInView = DeduceCellIdxsOverlappingRect(viewportRect, gameContext->cellWidth, gameContext->cellHeight);

    BeginMode2D(gameContext->camera);

    // Draw obstacles and units
    for (auto &cellIdx : cellIdxsInView)
    {
        if (gameContext->allObstacles.find(cellIdx) != gameContext->allObstacles.end())
        {
            entt::entity obstacleEntity = gameContext->allObstacles[cellIdx];
            auto &obstacle = gameContext->registry.get<Obstacle>(obstacleEntity);

            int atlasCoordX = obstacle.atlasCoords.x;
            int atlasCoordY = obstacle.atlasCoords.y;
            int atlasId = obstacle.atlasId;
            std::string spriteSheetName = "obstacles_sheet_" + std::to_string(atlasId);
            Rectangle sourceRect = {
                static_cast<float>(atlasCoordX * gameContext->cellWidth),
                static_cast<float>(atlasCoordY * gameContext->cellHeight),
                static_cast<float>(gameContext->cellWidth),
                static_cast<float>(gameContext->cellHeight)};
            Vector2 worldPosition = MapToWorld(cellIdx, gameContext->cellWidth, gameContext->cellHeight);
            Rectangle destRect = {
                worldPosition.x,
                worldPosition.y,
                static_cast<float>(gameContext->cellWidth),
                static_cast<float>(gameContext->cellHeight)};
            DrawTexturePro(gameContext->allTextures[spriteSheetName], sourceRect, destRect, {0.0f, 0.0f}, 0.0f, WHITE);
        }

        if (gameContext->allUnits.find(cellIdx) != gameContext->allUnits.end())
        {
            entt::entity unitEntity = gameContext->allUnits[cellIdx];
            auto &unit = gameContext->registry.get<Unit>(unitEntity);

            int atlasCoordX = unit.atlasCoords.x;
            int atlasCoordY = unit.atlasCoords.y;
            int atlasId = unit.atlasId;
            std::string spriteSheetName = "units_sheet_" + std::to_string(atlasId);
            Rectangle sourceRect = {
                static_cast<float>(atlasCoordX * gameContext->cellWidth),
                static_cast<float>(atlasCoordY * gameContext->cellHeight),
                static_cast<float>(gameContext->cellWidth),
                static_cast<float>(gameContext->cellHeight)};
            Vector2 worldPosition = MapToWorld(cellIdx, gameContext->cellWidth, gameContext->cellHeight);
            Rectangle destRect = {
                worldPosition.x,
                worldPosition.y,
                static_cast<float>(gameContext->cellWidth),
                static_cast<float>(gameContext->cellHeight)};
            DrawTexturePro(gameContext->allTextures[spriteSheetName], sourceRect, destRect, {0.0f, 0.0f}, 0.0f, WHITE);
        }
    }

    // Draw trapezoids after units and obstacles
    for (auto &cellIdx : cellIdxsInView)
    {
        if (gameContext->allUnits.find(cellIdx) != gameContext->allUnits.end())
        {
            entt::entity unitEntity = gameContext->allUnits[cellIdx];
            auto visionTrapEntity = gameContext->registry.try_get<IsoscelesTrapezoid>(unitEntity);
            if (visionTrapEntity != nullptr)
            {
                auto &visionTrapezoidComp = gameContext->registry.get<IsoscelesTrapezoid>(unitEntity);
                if (gameContext->registry.all_of<TeamRed>(unitEntity) && gameContext->myPlayer.team == Teams::TEAM_RED || gameContext->registry.all_of<TeamBlue>(unitEntity) && gameContext->myPlayer.team == Teams::TEAM_BLUE)
                {
                    DrawTriangle(visionTrapezoidComp.p1, visionTrapezoidComp.p2, visionTrapezoidComp.p3, Fade(WHITE, 0.1f));
                    DrawTriangle(visionTrapezoidComp.p1, visionTrapezoidComp.p3, visionTrapezoidComp.p4, Fade(WHITE, 0.1f));
                }
            }
        }
    }

    EndMode2D();
}

void sDrawPlayerDetails(GameContext *gameContext)
{
    if (gameContext->myPlayer.team == Teams::TEAM_BLUE)
    {
        DrawText("Blue Team", 10.0f, 10.0f, gameContext->baseFontSize, BLUE);
    }
    else if (gameContext->myPlayer.team == Teams::TEAM_RED)
    {
        DrawText("Red Team", 10.0f, 10.0f, gameContext->baseFontSize, RED);
    }

    std::string playerSuppliesString = "Supplies: " + std::to_string(gameContext->myPlayer.supplies);
    DrawText(playerSuppliesString.c_str(), 10.0f, 10.0f + gameContext->baseFontSize + 5.0f, gameContext->baseFontSize, WHITE);
}

void sDrawUnitDetails(GameContext *gameContext)
{
    Vector2 uiStartPos = {10.0f, 70.0f};                  // Initial position for drawing UI text
    float lineSpacing = gameContext->baseFontSize + 5.0f; // Spacing between lines

    // Draw selected unit details
    if (gameContext->selectedUnit != entt::null)
    {
        auto &selectedUnitComp = gameContext->registry.get<Unit>(gameContext->selectedUnit);
        std::vector<std::string> selectedUnitStrings;

        // Collect selected unit details
        if (gameContext->registry.all_of<TeamBlue>(gameContext->selectedUnit))
        {
            selectedUnitStrings.push_back("Team: Blue Team");
        }
        if (gameContext->registry.all_of<TeamRed>(gameContext->selectedUnit))
        {
            selectedUnitStrings.push_back("Team: Red Team");
        }
        selectedUnitStrings.push_back("Type: " + selectedUnitComp.type);
        selectedUnitStrings.push_back("Name: " + selectedUnitComp.givenName);
        selectedUnitStrings.push_back("Cell Idx: (" + std::to_string(selectedUnitComp.cellIdx.x) + ", " + std::to_string(selectedUnitComp.cellIdx.y) + ")");
        selectedUnitStrings.push_back("Health: " + std::to_string(selectedUnitComp.currentHealth) + " / " + std::to_string(selectedUnitComp.maxHealth));
        selectedUnitStrings.push_back("Supplies: " + std::to_string(selectedUnitComp.supplies) + " / " + std::to_string(selectedUnitComp.maxSupplies));
        selectedUnitStrings.push_back("Stops Projectile: " + std::string(selectedUnitComp.stopsProjectile ? "True" : "False"));
        selectedUnitStrings.push_back("Height: " + std::to_string(selectedUnitComp.intrinsicHeight));

        if (selectedUnitComp.maxOccupancy > 0)
        {
            selectedUnitStrings.push_back("Max Occupancy: " + std::to_string(selectedUnitComp.maxOccupancy));
        }
        if (selectedUnitComp.stance == Stances::STANDING)
        {
            selectedUnitStrings.push_back("Stance: Standing");
        }
        else if (selectedUnitComp.stance == Stances::CROUCHED)
        {
            selectedUnitStrings.push_back("Stance: Crouched");
        }
        else if (selectedUnitComp.stance == Stances::PRONE)
        {
            selectedUnitStrings.push_back("Stance: Prone");
        }
        if (selectedUnitComp.isPerson)
        {
            selectedUnitStrings.push_back("Person");
        }
        if (selectedUnitComp.isVehicle)
        {
            selectedUnitStrings.push_back("Vehicle");
        }
        if (selectedUnitComp.isStructure)
        {
            selectedUnitStrings.push_back("Structure");
        }

        // Draw selected unit details
        DrawText("Selected Unit:", uiStartPos.x, uiStartPos.y, gameContext->baseFontSize, GRAY);
        for (size_t i = 0; i < selectedUnitStrings.size(); ++i)
        {
            Color color = WHITE;
            if (selectedUnitStrings[i].find("Blue Team") != std::string::npos)
            {
                color = BLUE;
            }
            if (selectedUnitStrings[i].find("Red Team") != std::string::npos)
            {
                color = RED;
            }
            DrawText(selectedUnitStrings[i].c_str(), uiStartPos.x, uiStartPos.y + lineSpacing * (i + 1), gameContext->baseFontSize, color);
        }

        // Update the starting position for the next section
        uiStartPos.y += lineSpacing * (selectedUnitStrings.size() + 2);
    }

    // Check if the mouse is hovering over a unit
    Vector2 mousePosScreen = GetMousePosition();
    Vector2 mousePosWorld = GetScreenToWorld2D(mousePosScreen, gameContext->camera);
    Vector2i mousePosCellIdx = WorldToMap(mousePosWorld, gameContext->cellWidth, gameContext->cellHeight);

    if (gameContext->allUnits.find(mousePosCellIdx) != gameContext->allUnits.end())
    {
        entt::entity hoveredUnit = gameContext->allUnits[mousePosCellIdx];
        auto &hoveredUnitComp = gameContext->registry.get<Unit>(hoveredUnit);

        std::vector<std::string> hoveredUnitStrings;

        // Collect hovered unit details
        if (gameContext->registry.all_of<TeamBlue>(hoveredUnit))
        {
            hoveredUnitStrings.push_back("Team: Blue Team");
        }
        if (gameContext->registry.all_of<TeamRed>(hoveredUnit))
        {
            hoveredUnitStrings.push_back("Team: Red Team");
        }
        hoveredUnitStrings.push_back("Type: " + hoveredUnitComp.type);
        hoveredUnitStrings.push_back("Name: " + hoveredUnitComp.givenName);
        hoveredUnitStrings.push_back("Cell Idx: (" + std::to_string(hoveredUnitComp.cellIdx.x) + ", " + std::to_string(hoveredUnitComp.cellIdx.y) + ")");
        hoveredUnitStrings.push_back("Health: " + std::to_string(hoveredUnitComp.currentHealth) + " / " + std::to_string(hoveredUnitComp.maxHealth));
        hoveredUnitStrings.push_back("Supplies: " + std::to_string(hoveredUnitComp.supplies) + " / " + std::to_string(hoveredUnitComp.maxSupplies));
        hoveredUnitStrings.push_back("Stops Projectile: " + std::string(hoveredUnitComp.stopsProjectile ? "True" : "False"));
        hoveredUnitStrings.push_back("Height: " + std::to_string(hoveredUnitComp.intrinsicHeight));

        if (hoveredUnitComp.maxOccupancy > 0)
        {
            hoveredUnitStrings.push_back("Max Occupancy: " + std::to_string(hoveredUnitComp.maxOccupancy));
        }
        if (hoveredUnitComp.stance == Stances::STANDING)
        {
            hoveredUnitStrings.push_back("Stance: Standing");
        }
        else if (hoveredUnitComp.stance == Stances::CROUCHED)
        {
            hoveredUnitStrings.push_back("Stance: Crouched");
        }
        else if (hoveredUnitComp.stance == Stances::PRONE)
        {
            hoveredUnitStrings.push_back("Stance: Prone");
        }
        if (hoveredUnitComp.isPerson)
        {
            hoveredUnitStrings.push_back("Person");
        }
        if (hoveredUnitComp.isVehicle)
        {
            hoveredUnitStrings.push_back("Vehicle");
        }
        if (hoveredUnitComp.isStructure)
        {
            hoveredUnitStrings.push_back("Structure");
        }

        // Draw hovered unit details
        DrawText("Hovered Unit:", uiStartPos.x, uiStartPos.y, gameContext->baseFontSize, GRAY);
        for (size_t i = 0; i < hoveredUnitStrings.size(); ++i)
        {
            Color color = WHITE;
            if (hoveredUnitStrings[i].find("Blue Team") != std::string::npos)
            {
                color = BLUE;
            }
            if (hoveredUnitStrings[i].find("Red Team") != std::string::npos)
            {
                color = RED;
            }
            DrawText(hoveredUnitStrings[i].c_str(), uiStartPos.x, uiStartPos.y + lineSpacing * (i + 1), gameContext->baseFontSize, color);
        }
    }
}

void sDrawHoveredCellInfo(GameContext *gameContext)
{
    if (!CheckMouseInMapBounds(gameContext))
    {
        return;
    }
    Vector2 mousePosScreen = GetMousePosition();
    Vector2 mousePosWorld = GetScreenToWorld2D(mousePosScreen, gameContext->camera);
    Vector2i mousePosCellIdx = WorldToMap(mousePosWorld, gameContext->cellWidth, gameContext->cellHeight);

    CellSummary cellSummary = GetCellSummary(gameContext, mousePosCellIdx);

    std::string cellInfo = gameContext->currentMap + " : " + "(" + std::to_string(mousePosCellIdx.x) + ", " + std::to_string(mousePosCellIdx.y) + ")";
    cellInfo += " : Level: " + std::to_string(cellSummary.terrainLevel) + " ";
    cellInfo += std::to_string(cellSummary.totalObstacleHeight) + "ft; ";
    cellInfo += std::to_string(cellSummary.totalHeight) + "ft; ";
    if (cellSummary.obstacle != entt::null)
    {
        auto &obstacleComp = gameContext->registry.get<Obstacle>(cellSummary.obstacle);
        cellInfo += " : " + obstacleComp.displayName;
    }
    int textWidth = MeasureText(cellInfo.c_str(), gameContext->baseFontSize);
    int textHeight = gameContext->baseFontSize;
    int posX = gameContext->screenWidth - textWidth - 10;
    int posY = gameContext->screenHeight - textHeight - 10;
    DrawText(cellInfo.c_str(), posX, posY, gameContext->baseFontSize, WHITE);
}

void sDrawNextTurnTip(GameContext *gameContext)
{
    std::string string = "Next Turn (Shift + Enter) " + std::to_string(gameContext->turnCount) + " -> " + std::to_string(gameContext->turnCount + 1);
    int textWidth = MeasureText(string.c_str(), gameContext->baseFontSize);
    int textHeight = gameContext->baseFontSize;
    int posX = gameContext->screenWidth - textWidth - 10;
    int posY = gameContext->screenHeight - textHeight - 30;
    DrawText(string.c_str(), posX, posY, gameContext->baseFontSize, WHITE);
}

void sDrawSelectedUnitAbilities(GameContext *gameContext)
{
    Vector2 uiStartPos = {250.0f, 10.0f};                 // Initial position for drawing UI text
    float lineSpacing = gameContext->baseFontSize + 5.0f; // Spacing between lines

    if (gameContext->selectedUnit != entt::null)
    {
        auto &selectedUnitComp = gameContext->registry.get<Unit>(gameContext->selectedUnit);
        auto &abilities = selectedUnitComp.abilities;

        if (abilities.size() > 0)
        {
            int i = 0;
            for (auto &ability : abilities)
            {
                Color color = WHITE; // Default text color
                bool doUnderline = false;

                // Highlight the selected ability in red
                if (selectedUnitComp.selectedAbility != nullptr && selectedUnitComp.selectedAbility->type == ability.type)
                {
                    doUnderline = true;
                }

                // if (selectedUnitComp.selectedAbility->supplyCost > selectedUnitComp.supplies ||
                //     (selectedUnitComp.selectedAbility->maxCooldown > 0 && gameContext->turnCount - selectedUnitComp.selectedAbility->lastTurnUsed))
                // {
                // }
                if (selectedUnitComp.supplies < ability.supplyCost)
                {
                    color = LIGHTGRAY;
                }

                // Draw the ability name at the calculated position
                Vector2 textPos = {uiStartPos.x, uiStartPos.y + i * lineSpacing};
                int textWidth = MeasureText(ability.type.c_str(), gameContext->baseFontSize);
                DrawText(ability.type.c_str(), textPos.x, textPos.y, gameContext->baseFontSize, color);
                if (doUnderline)
                {
                    DrawLine(textPos.x, textPos.y + gameContext->baseFontSize, textPos.x + textWidth, textPos.y + gameContext->baseFontSize, RED);
                    DrawText(ability.description.c_str(), textPos.x + textWidth + 10.0f, textPos.y, gameContext->baseFontSize, LIGHTGRAY);
                }

                i++; // Move to the next line for the next ability
            }
        }
    }
}

void sDrawSelectedUnitIndicator(GameContext *gameContext)
{
    if (gameContext->selectedUnit != entt::null)
    {
        auto &selectedUnitComp = gameContext->registry.get<Unit>(gameContext->selectedUnit);
        Vector2 selectedUnitWorldPos = MapToWorld(selectedUnitComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight);
        Rectangle rect = Rectangle{selectedUnitWorldPos.x, selectedUnitWorldPos.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)};
        BeginMode2D(gameContext->camera);
        DrawRectangleLinesEx(rect, 2.0f, YELLOW);
        EndMode2D();
    }
}

void sDrawHoveredCellIndicator(GameContext *gameContext)
{
    if (!CheckMouseInMapBounds(gameContext))
    {
        return;
    }
    Vector2 mousePosScreen = GetMousePosition();
    Vector2 mousePosWorld = GetScreenToWorld2D(mousePosScreen, gameContext->camera);
    Vector2i mousePosCellIdx = WorldToMap(mousePosWorld, gameContext->cellWidth, gameContext->cellHeight);

    Vector2 mouseRectCellIdxToWorld = MapToWorld(mousePosCellIdx, gameContext->cellWidth, gameContext->cellHeight);
    // Vector2 mouseRectCenter = GetRectCenter(Rectangle{mouseRectCellIdxToWorld.x, mouseRectCellIdxToWorld.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});
    Rectangle rect = Rectangle{mouseRectCellIdxToWorld.x, mouseRectCellIdxToWorld.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)};

    BeginMode2D(gameContext->camera);
    DrawRectangleLinesEx(rect, 2.0f, YELLOW);
    EndMode2D();
}

void sDrawIndicatorLine(GameContext *gameContext)
{
    if (gameContext->selectedUnit != entt::null && CheckMouseInMapBounds(gameContext))
    {
        auto &selectedUnitComp = gameContext->registry.get<Unit>(gameContext->selectedUnit);

        Vector2 selectedUnitWorldPos = MapToWorld(selectedUnitComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight);
        Vector2 selectedUnitCenter = GetRectCenter(Rectangle{selectedUnitWorldPos.x, selectedUnitWorldPos.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});

        Vector2 mousePosScreen = GetMousePosition();
        Vector2 mousePosWorld = GetScreenToWorld2D(mousePosScreen, gameContext->camera);
        Vector2i mousePosCellIdx = WorldToMap(mousePosWorld, gameContext->cellWidth, gameContext->cellHeight);

        Vector2 mouseRectCellIdxToWorld = MapToWorld(mousePosCellIdx, gameContext->cellWidth, gameContext->cellHeight);
        Vector2 mouseRectCenter = GetRectCenter(Rectangle{mouseRectCellIdxToWorld.x, mouseRectCellIdxToWorld.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});

        BeginMode2D(gameContext->camera);
        DrawLine(selectedUnitCenter.x, selectedUnitCenter.y, mouseRectCenter.x, mouseRectCenter.y, Fade(WHITE, 0.3f));
        EndMode2D();
    }
}

void sDrawTargetingDetails(GameContext *gameContext)
{
    // Distance, terrain level diff, height diff, is vision blocked,
}