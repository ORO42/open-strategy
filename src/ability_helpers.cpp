#include "ability_helpers.h"
#include "map_helpers.h"
#include "math_helpers.h"
#include "unit_helpers.h"
#include "ui_helpers.h"

void sCycleSelectedAbility(GameContext *gameContext)
{
    if (gameContext->selectedUnit != entt::null)
    {
        auto &selectedUnitComp = gameContext->registry.get<Unit>(gameContext->selectedUnit);
        auto &abilities = selectedUnitComp.abilities;
        int abilitiesSize = abilities.size();

        if (abilitiesSize > 0)
        {
            // Handle down arrow key
            if (IsKeyPressed(KEY_DOWN))
            {
                // Increment the index
                int newIdx = selectedUnitComp.selectedAbilityIdx + 1;

                // Wrap around or move to the neutral spot
                if (newIdx > abilitiesSize) // Beyond the last ability
                {
                    selectedUnitComp.selectedAbilityIdx = -1; // Neutral spot
                    selectedUnitComp.selectedAbility = nullptr;
                }
                else if (newIdx == abilitiesSize) // Last ability to neutral
                {
                    selectedUnitComp.selectedAbilityIdx = -1; // Neutral spot
                    selectedUnitComp.selectedAbility = nullptr;
                }
                else // Valid ability index
                {
                    selectedUnitComp.selectedAbilityIdx = newIdx;
                    selectedUnitComp.selectedAbility = &abilities[newIdx];
                }
            }

            // Handle up arrow key
            if (IsKeyPressed(KEY_UP))
            {
                // Decrement the index
                int newIdx = selectedUnitComp.selectedAbilityIdx - 1;

                // Wrap around or move to the neutral spot
                if (newIdx < -1) // Below neutral spot, wrap to the last ability
                {
                    selectedUnitComp.selectedAbilityIdx = abilitiesSize - 1;
                    selectedUnitComp.selectedAbility = &abilities[abilitiesSize - 1];
                }
                else if (newIdx == -1) // Back to the neutral spot
                {
                    selectedUnitComp.selectedAbilityIdx = -1;
                    selectedUnitComp.selectedAbility = nullptr;
                }
                else // Valid ability index
                {
                    selectedUnitComp.selectedAbilityIdx = newIdx;
                    selectedUnitComp.selectedAbility = &abilities[newIdx];
                }
            }
        }
    }
}

void sUseAbilities(GameContext *gameContext)
{
    if (gameContext->selectedUnit == entt::null)
    {
        return;
    }

    entt::entity selectedUnitEntity = gameContext->selectedUnit;
    auto &selectedUnitComp = gameContext->registry.get<Unit>(selectedUnitEntity);

    if (selectedUnitComp.selectedAbility == nullptr)
    {
        return;
    }

    if (selectedUnitComp.selectedAbility->requiresCell && !CheckMouseInMapBounds(gameContext))
    {
        return;
    }

    Vector2 mousePosScreen = GetMousePosition();
    Vector2 mousePosWorld = GetScreenToWorld2D(mousePosScreen, gameContext->camera);
    Vector2i mousePosCellIdx = WorldToMap(mousePosWorld, gameContext->cellWidth, gameContext->cellHeight);

    Vector2 selectedUnitWorldPos = MapToWorld(selectedUnitComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight);
    Vector2 selectedUnitCenter = GetRectCenter(Rectangle{selectedUnitWorldPos.x, selectedUnitWorldPos.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});

    Vector2 mouseRectCellIdxToWorld = MapToWorld(mousePosCellIdx, gameContext->cellWidth, gameContext->cellHeight);
    Vector2 mouseRectCenter = GetRectCenter(Rectangle{mouseRectCellIdxToWorld.x, mouseRectCellIdxToWorld.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});

    int chebDist = GetChebyshevDistance(selectedUnitComp.cellIdx, mousePosCellIdx);

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        if (selectedUnitComp.selectedAbility->supplyCost > selectedUnitComp.supplies)
        {
            std::cout << "Not enough supplies" << std::endl;
            return;
        }

        if (selectedUnitComp.selectedAbility->maxUsesPerTurn > -1 && selectedUnitComp.selectedAbility->usesThisTurn >= selectedUnitComp.selectedAbility->maxUsesPerTurn)
        {
            std::cout << "Ability max uses per turn reached" << std::endl;
            return;
        }

        if (selectedUnitComp.selectedAbility->maxCooldown > -1 && gameContext->turnCount - selectedUnitComp.selectedAbility->lastTurnUsed < selectedUnitComp.selectedAbility->maxCooldown)
        {
            std::cout << "Ability on cooldown" << std::endl;
            return;
        }

        if (selectedUnitComp.selectedAbility->range > -1 && chebDist > selectedUnitComp.selectedAbility->range)
        {
            std::cout << "Ability out of range" << std::endl;
            return;
        }

        selectedUnitComp.selectedAbility->usesThisTurn++;
        selectedUnitComp.selectedAbility->lastTurnUsed = gameContext->turnCount;
        selectedUnitComp.supplies -= selectedUnitComp.selectedAbility->supplyCost;
    }

    BeginMode2D(gameContext->camera);
    const CellSummary selectedUnitCellSummary = GetCellSummary(gameContext, selectedUnitComp.cellIdx);
    int currMoveCost = 0;
    std::vector<Vector2i> finalMovePoints;
    std::vector<Vector2i> bresenhamCells;
    // NOTE: Bresenham's is only used for the "move" ability
    if (selectedUnitComp.selectedAbility->doesBresenhamTargeting)
    {
        bresenhamCells = GetBresenhamCells(selectedUnitComp.cellIdx, mousePosCellIdx, gameContext->cellWidth, gameContext->cellHeight);
        bresenhamCells.erase(bresenhamCells.begin()); // Remove the first cell, since we don't count the unit's cell

        for (auto &cell : bresenhamCells)
        {
            const CellSummary cellSummary = GetCellSummary(gameContext, cell);
            if (cellSummary.obstacle != entt::null)
            {
                auto &obstacleComp = gameContext->registry.get<Obstacle>(cellSummary.obstacle);
                currMoveCost += obstacleComp.moveCostSupplies;
            }
            if (chebDist > selectedUnitComp.selectedAbility->range || currMoveCost > selectedUnitComp.supplies)
            {
                break;
            }
            finalMovePoints.push_back(cell);
            DrawRectangleRec({static_cast<float>(cell.x) * gameContext->cellWidth, static_cast<float>(cell.y) * gameContext->cellHeight, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)}, Fade(BLUE, 0.2f));
        }
    }

    // Handle inaccuracyRadius
    Vector2i finalCellIdx = mousePosCellIdx;
    Vector2 finalCenter = mouseRectCenter;
    float accuracyP = 1.0;
    bool didAccRollSucceed = true;
    // TODO: create "miss" popup at desired target if acc roll fails
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (selectedUnitComp.selectedAbility->inaccuracyRadius > 0)
        {
            Rectangle rect = GenerateCellNeighborRect(selectedUnitComp.cellIdx, selectedUnitComp.selectedAbility->inaccuracyRadius, gameContext->cellWidth, gameContext->cellHeight);
            std::vector<Vector2i> cellsInInaccuracyRadius = DeduceCellIdxsOverlappingRect(rect, gameContext->cellWidth, gameContext->cellHeight);
            accuracyP -= chebDist * selectedUnitComp.selectedAbility->accuracyFalloff;
            if (!Chance(accuracyP))
            {
                didAccRollSucceed = false;
                Vector2i randomCellIdx = GetRandomItemFromVector(cellsInInaccuracyRadius);
                finalCellIdx = randomCellIdx;
                Vector2 finalCellIdxToWorld = MapToWorld(randomCellIdx, gameContext->cellWidth, gameContext->cellHeight);
                finalCenter = GetRectCenter(Rectangle{finalCellIdxToWorld.x, finalCellIdxToWorld.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});
            }
        }
    }

    CellSummary finalCellSummary = GetCellSummary(gameContext, finalCellIdx);

    // TODO: Implement corner->corner casting for more forgiving LOS
    std::vector<Vector2i> straightLineCells;
    Vector2i blockingCellIdx = {-1, -1};
    if (selectedUnitComp.selectedAbility->doesStraightLineTargeting)
    {
        straightLineCells = GetCellsOverlappingLine(selectedUnitCenter, finalCenter, gameContext->cellWidth, gameContext->cellHeight);
        straightLineCells.erase(straightLineCells.begin()); // Remove the first cell, since we don't count the unit's cell

        for (auto &cell : straightLineCells)
        {
            const CellSummary cellSummary = GetCellSummary(gameContext, cell);
            if (cellSummary.obstacle != entt::null)
            {
                auto &obstacleComp = gameContext->registry.get<Obstacle>(cellSummary.obstacle);
            }
            DrawRectangleRec({static_cast<float>(cell.x) * gameContext->cellWidth, static_cast<float>(cell.y) * gameContext->cellHeight, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)}, Fade(BLUE, 0.2f));

            blockingCellIdx = HasElevationLOS(gameContext, 3.28f, selectedUnitComp.cellIdx, mousePosCellIdx, cell);
            if (blockingCellIdx.x != -1 && blockingCellIdx.y != -1)
            {
                // There is a blocking cell
                finalCellIdx = blockingCellIdx;
                Vector2 finalCellIdxToWorld = MapToWorld(finalCellIdx, gameContext->cellWidth, gameContext->cellHeight);
                finalCenter = GetRectCenter(Rectangle{finalCellIdxToWorld.x, finalCellIdxToWorld.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});
                Circle blockingIndicator = GenerateGridBoundCircle(finalCenter, 0, gameContext->cellWidth, gameContext->cellHeight);
                DrawCircle(blockingIndicator.centerPos.x, blockingIndicator.centerPos.y, blockingIndicator.radius, BLACK);
                break;
            }
        }
    }

    // if (selectedUnitComp.selectedAbility->firesProjectile)
    // {
    //     if (!selectedUnitComp.selectedAbility->isAerialProjectile)
    //     {
    //         // If prev acc roll failed, choose a random
    //     }
    // }

    if (selectedUnitComp.selectedAbility->range > 0)
    {
        Rectangle rect = GenerateCellNeighborRect(selectedUnitComp.cellIdx, selectedUnitComp.selectedAbility->range, gameContext->cellWidth, gameContext->cellHeight);
        DrawRectangleRec(rect, Fade(WHITE, 0.2f));
    }

    if (selectedUnitComp.selectedAbility->aoeSize > 0)
    {
        Rectangle rect = GenerateCellNeighborRect(mousePosCellIdx, selectedUnitComp.selectedAbility->aoeSize, gameContext->cellWidth, gameContext->cellHeight);
        DrawRectangleRec(rect, Fade(WHITE, 0.2f));
    }

    if (selectedUnitComp.selectedAbility->inaccuracyRadius > 0)
    {
        Rectangle rect = GenerateCellNeighborRect(mousePosCellIdx, selectedUnitComp.selectedAbility->inaccuracyRadius, gameContext->cellWidth, gameContext->cellHeight);
        DrawRectangleRec(rect, Fade(ORANGE, 0.2f));
    }
    EndMode2D();

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (selectedUnitComp.selectedAbility->type == "move" && bresenhamCells.size() > 0)
        {
            bool finalIsCliff = false;
            const CellSummary endCellSummary = GetCellSummary(gameContext, finalMovePoints.back());
            if (endCellSummary.obstacle != entt::null)
            {
                auto &obstacleComp = gameContext->registry.get<Obstacle>(endCellSummary.obstacle);
                obstacleComp.displayName == "cliff" ? finalIsCliff = true : finalIsCliff = false;
            }
            if (!finalIsCliff)
            {
                if (!gameContext->registry.all_of<MovePoints>(selectedUnitEntity))
                {
                    gameContext->registry.emplace<MovePoints>(selectedUnitEntity, bresenhamCells);
                    std::cout << "Supplies before move: " << " " << selectedUnitComp.supplies << " " << currMoveCost << std::endl;
                    selectedUnitComp.supplies -= currMoveCost;
                    std::cout << "Supplies after move: " << " " << selectedUnitComp.supplies << " " << currMoveCost << std::endl;
                }
            }
        }
        if (selectedUnitComp.selectedAbility->type == "rotate")
        {
            auto visionTrapEntity = gameContext->registry.try_get<IsoscelesTrapezoid>(selectedUnitEntity);
            if (visionTrapEntity != nullptr)
            {
                auto &visionTrapezoidComp = gameContext->registry.get<IsoscelesTrapezoid>(selectedUnitEntity);
                visionTrapEntity->facingAngle = GetAngleBetweenPoints(selectedUnitCenter, mouseRectCenter);
                PositionAllTrapezoids(gameContext);
            }
        }
        if (selectedUnitComp.selectedAbility->firesProjectile && !selectedUnitComp.selectedAbility->isAerialProjectile)
        {
            CellSummary targetCellSummary = GetCellSummary(gameContext, finalCellIdx);

            bool damagedObstacle = false;

            if (targetCellSummary.obstacle != entt::null)
            {
                auto &obstacleComp = gameContext->registry.get<Obstacle>(targetCellSummary.obstacle);

                if (obstacleComp.isDestructible)
                {
                    int obstacleDamage = selectedUnitComp.selectedAbility->terrainDamageMin + std::rand() % (selectedUnitComp.selectedAbility->terrainDamageMax - selectedUnitComp.selectedAbility->terrainDamageMin + 1);
                    obstacleComp.currentHealth -= obstacleDamage;
                    damagedObstacle = true;
                    CreatePopupText(gameContext, std::to_string(obstacleDamage), MapToWorld(obstacleComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight), LIGHTGRAY, false, std::chrono::seconds(1));
                }
            }
            if (targetCellSummary.unit != entt::null)
            {
                auto &unitComp = gameContext->registry.get<Unit>(targetCellSummary.unit);
                int finalUnitDamage;

                if (unitComp.isPerson)
                {
                    finalUnitDamage = selectedUnitComp.selectedAbility->fleshDamageMin + std::rand() % (selectedUnitComp.selectedAbility->fleshDamageMax - selectedUnitComp.selectedAbility->fleshDamageMin + 1);
                }

                if (unitComp.isStructure || unitComp.isVehicle)
                {
                    finalUnitDamage = selectedUnitComp.selectedAbility->armorDamageMin + std::rand() % (selectedUnitComp.selectedAbility->armorDamageMax - selectedUnitComp.selectedAbility->armorDamageMin + 1);
                }

                unitComp.currentHealth -= finalUnitDamage;

                CreatePopupText(gameContext, std::to_string(finalUnitDamage), MapToWorld(unitComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight), GREEN, false, std::chrono::seconds(1));
            }

            // TODO: handle AOE
        }
    }
}