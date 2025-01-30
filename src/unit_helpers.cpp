#include "unit_helpers.h"
#include "math_helpers.h"
#include "map_helpers.h"

void CreateUnit(GameContext *gameContext, const std::string &type, const Vector2i &cellIdx, const Teams &team)
{
    entt::entity unitEntity = gameContext->registry.create();
    gameContext->allUnits[cellIdx] = unitEntity;

    Unit newUnit;
    newUnit.atlasId = gameContext->unitTemplates[type]["atlas_id"];
    newUnit.atlasCoords = Vector2i{gameContext->unitTemplates[type]["atlas_coords"]["x"], gameContext->unitTemplates[type]["atlas_coords"]["y"]};
    newUnit.cellIdx = cellIdx;
    newUnit.type = type;
    newUnit.givenName = "Placeholder Name";
    newUnit.isPerson = gameContext->unitTemplates[type]["is_person"];
    newUnit.isVehicle = gameContext->unitTemplates[type]["is_vehicle"];
    newUnit.isStructure = gameContext->unitTemplates[type]["is_structure"];
    newUnit.intrinsicHeight = gameContext->unitTemplates[type]["intrinsic_height"];
    newUnit.crouchHeight = gameContext->unitTemplates[type]["crouch_height"];
    newUnit.proneHeight = gameContext->unitTemplates[type]["prone_height"];
    newUnit.maxSupplies = gameContext->unitTemplates[type]["max_supplies"];
    newUnit.supplies = std::floor(newUnit.maxSupplies / 2);
    newUnit.maxHealth = gameContext->unitTemplates[type]["max_health"];
    newUnit.currentHealth = newUnit.maxHealth;
    newUnit.stopsProjectile = gameContext->unitTemplates[type]["stops_projectile"];
    newUnit.maxOccupancy = gameContext->unitTemplates[type]["max_occupancy"];
    newUnit.stance = Stances::STANDING;

    newUnit.team = team;
    if (team == Teams::TEAM_BLUE)
    {
        gameContext->registry.emplace<TeamBlue>(unitEntity);

        if (gameContext->myPlayer.team == Teams::TEAM_BLUE)
        {
            gameContext->registry.emplace<IsVisible>(unitEntity);
        }
    }
    else if (team == Teams::TEAM_RED)
    {
        gameContext->registry.emplace<TeamRed>(unitEntity);

        if (gameContext->myPlayer.team == Teams::TEAM_RED)
        {
            gameContext->registry.emplace<IsVisible>(unitEntity);
        }
    }

    if (gameContext->unitTemplates[type]["use_vision"])
    {
        Vector2 unitWorldPos = MapToWorld(cellIdx, gameContext->cellWidth, gameContext->cellHeight);
        Vector2 unitCenter = GetRectCenter(Rectangle{unitWorldPos.x, unitWorldPos.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});
        int visionBaseWidth = gameContext->unitTemplates[type]["vision_base_width"];
        int visionTopWidth = gameContext->unitTemplates[type]["vision_top_width"];
        int visionLength = gameContext->unitTemplates[type]["vision_length"];
        float cellWidthFloat = static_cast<float>(gameContext->cellWidth);
        Vector2 origin = unitCenter;
        Vector2 p1 = {origin.x - (visionBaseWidth / 2.0f) * cellWidthFloat, origin.y};
        Vector2 p2 = {origin.x + (visionBaseWidth / 2.0f) * cellWidthFloat, origin.y};
        Vector2 p3 = {origin.x + (visionTopWidth / 2.0f) * cellWidthFloat, origin.y - visionLength * cellWidthFloat};
        Vector2 p4 = {origin.x - (visionTopWidth / 2.0f) * cellWidthFloat, origin.y - visionLength * cellWidthFloat};
        gameContext->registry.emplace<IsoscelesTrapezoid>(unitEntity,
                                                          visionBaseWidth,
                                                          visionTopWidth,
                                                          visionLength,
                                                          -90.0f,
                                                          origin,
                                                          p1,
                                                          p2,
                                                          p3,
                                                          p4);
    }

    nlohmann::json unitAbilities = gameContext->unitTemplates[type]["abilities"];
    if (unitAbilities.size() > 0)
    {
        for (auto it = unitAbilities.begin(); it != unitAbilities.end(); ++it)
        {
            const std::string &key = it.key();
            const nlohmann::json &value = it.value();

            Ability newAbility;
            newAbility.type = value["type"];
            newAbility.description = value["description"];
            newAbility.requiresCell = value["requires_cell"];
            newAbility.supplyCost = value["supply_cost"];
            newAbility.maxUsesPerTurn = value["max_uses_per_turn"];
            newAbility.lastTurnUsed = -1;
            newAbility.maxCooldown = value["max_cooldown"];
            newAbility.doesBresenhamTargeting = value["does_bresenham_targeting"];
            newAbility.doesStraightLineTargeting = value["does_straight_line_targeting"];
            newAbility.range = value["range"];
            newAbility.aoeSize = value["aoe_size"];
            newAbility.fleshDamageMax = value["flesh_damage_max"];
            newAbility.fleshDamageMin = value["flesh_damage_min"];
            newAbility.armorDamageMax = value["armor_damage_max"];
            newAbility.armorDamageMin = value["armor_damage_min"];
            newAbility.terrainDamageMax = value["terrain_damage_max"];
            newAbility.terrainDamageMin = value["terrain_damage_min"];
            newAbility.firesProjectile = value["fires_projectile"];
            newAbility.isAerialProjectile = value["is_aerial_projectile"];
            newAbility.accuracyFalloff = value["accuracy_falloff"];
            newAbility.inaccuracyRadius = value["inaccuracy_radius"];
            newAbility.createsUnit = value["creates_unit"];
            newAbility.tileEffect = value["tile_effect"];
            newAbility.suppression = value["suppression"];
            newAbility.suppressionChance = value["suppression_chance"];
            newAbility.suppressionRadius = value["suppression_radius"];

            newUnit.abilities.push_back(newAbility);
        }
    }

    gameContext->registry.emplace<Unit>(unitEntity, newUnit);
}

void sUnitSelection(GameContext *gameContext)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePosScreen = GetMousePosition();
        Vector2 mousePosWorld = GetScreenToWorld2D({mousePosScreen.x, mousePosScreen.y}, gameContext->camera);
        Vector2i mousePosCellIdx = WorldToMap(mousePosWorld, gameContext->cellWidth, gameContext->cellHeight);

        if (gameContext->allUnits.find(mousePosCellIdx) != gameContext->allUnits.end())
        {
            entt::entity unitEntity = gameContext->allUnits[mousePosCellIdx];
            if (unitEntity == gameContext->selectedUnit)
            {
                gameContext->selectedUnit = entt::null;
            }
            else
            {
                gameContext->selectedUnit = unitEntity;
            }
        }
        else
        {
            gameContext->selectedUnit = entt::null;
        }
    }
}

void sMoveUnits(GameContext *gameContext)
{
    auto view = gameContext->registry.view<Unit, MovePoints>();
    for (auto entity : view)
    {
        auto &unitComp = view.get<Unit>(entity);
        entt::entity unitEntity = gameContext->allUnits[unitComp.cellIdx];
        auto &movePointsComp = view.get<MovePoints>(entity);

        for (auto cellIdx : movePointsComp.moveCellIdxs)
        {
            if (gameContext->allUnits.find(cellIdx) != gameContext->allUnits.end())
            {
                // There is already a unit at the next move point
                entt::entity encounteredUnitEntity = gameContext->allUnits[cellIdx];
                auto &encounteredUnitComp = gameContext->registry.get<Unit>(encounteredUnitEntity);

                // Swap cellIdx of the units
                std::swap(unitComp.cellIdx, encounteredUnitComp.cellIdx);

                // Update the allUnits map
                gameContext->allUnits[unitComp.cellIdx] = unitEntity;
                gameContext->allUnits[encounteredUnitComp.cellIdx] = encounteredUnitEntity;
            }
            else
            {
                // Move the unit to the new cell
                gameContext->allUnits.erase(unitComp.cellIdx);
                unitComp.cellIdx = cellIdx;
                gameContext->allUnits[cellIdx] = unitEntity;
            }

            nlohmann::json netMessage = nlohmann::json::object({{"type", MessageTypes::MOVE_UNIT},
                                                                {"from_team", gameContext->myPlayer.team},
                                                                {"entity", entity},
                                                                {"new_cell_idx_x", movePointsComp.moveCellIdxs.front().x},
                                                                {"new_cell_idx_y", movePointsComp.moveCellIdxs.front().y}});

            // remove the first move cell idx
            movePointsComp.moveCellIdxs.erase(movePointsComp.moveCellIdxs.begin());

            // Update unit vision trapezoid positions
            PositionAllTrapezoids(gameContext);
            ComputeMyTeamsVision(gameContext);

            if (movePointsComp.moveCellIdxs.size() == 0)
            {
                gameContext->registry.remove<MovePoints>(unitEntity);
            }
        }
    }
}

void PositionAllTrapezoids(GameContext *gameContext)
{
    auto view = gameContext->registry.view<Unit, IsoscelesTrapezoid>();
    for (auto &entity : view)
    {
        auto &unitComp = gameContext->registry.get<Unit>(entity);
        auto &visionTrapComp = gameContext->registry.get<IsoscelesTrapezoid>(entity);

        float northAngle = -90.0f;

        Vector2 unitWorldPos = MapToWorld(unitComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight);
        Vector2 unitCenter = GetRectCenter(Rectangle{unitWorldPos.x, unitWorldPos.y, static_cast<float>(gameContext->cellWidth), static_cast<float>(gameContext->cellHeight)});

        float cellWidthFloat = static_cast<float>(gameContext->cellWidth);

        int visionBaseWidth = visionTrapComp.baseWidth;
        int visionTopWidth = visionTrapComp.topWidth;
        int visionLength = visionTrapComp.length;

        Vector2 origin = unitCenter;
        Vector2 p1 = {origin.x - (visionBaseWidth / 2.0f) * cellWidthFloat, origin.y};
        Vector2 p2 = {origin.x + (visionBaseWidth / 2.0f) * cellWidthFloat, origin.y};
        Vector2 p3 = {origin.x + (visionTopWidth / 2.0f) * cellWidthFloat, origin.y - visionLength * cellWidthFloat};
        Vector2 p4 = {origin.x - (visionTopWidth / 2.0f) * cellWidthFloat, origin.y - visionLength * cellWidthFloat};

        IsoscelesTrapezoid tempTrap = IsoscelesTrapezoid{visionBaseWidth, visionTopWidth, visionLength, northAngle, origin, p1, p2, p3, p4};

        float angleDelta = AngleDifference(northAngle, visionTrapComp.facingAngle);

        RotateTrapezoid(tempTrap, angleDelta);
        visionTrapComp.originPos = origin;
        visionTrapComp.p1 = tempTrap.p1;
        visionTrapComp.p2 = tempTrap.p2;
        visionTrapComp.p3 = tempTrap.p3;
        visionTrapComp.p4 = tempTrap.p4;
    }
}

template <typename MyTeamComponent, typename EnemyTeamComponent>
void ComputeTeamVision(GameContext *gameContext)
{
    auto myTeamView = gameContext->registry.view<Unit, IsoscelesTrapezoid, MyTeamComponent>();
    auto enemyView = gameContext->registry.view<Unit, IsoscelesTrapezoid, EnemyTeamComponent>();

    // Remove visibility from all enemies first
    for (auto entity : enemyView)
    {
        if (gameContext->registry.all_of<IsVisible>(entity))
        {
            gameContext->registry.remove<IsVisible>(entity);
        }
    }

    // Check visibility for each unit in my team
    for (auto entity : myTeamView)
    {
        const auto &unitComp = gameContext->registry.get<Unit>(entity);
        const auto &visionTrap = gameContext->registry.get<IsoscelesTrapezoid>(entity);

        Vector2 unitWorldPos = MapToWorld(unitComp.cellIdx, gameContext->cellWidth, gameContext->cellHeight);
        Rectangle unitRect = {unitWorldPos.x, unitWorldPos.y,
                              static_cast<float>(gameContext->cellWidth),
                              static_cast<float>(gameContext->cellHeight)};
        Vector2 unitCenter = GetRectCenter(unitRect);

        // Check against all enemy units
        for (auto enemyEntity : enemyView)
        {
            const auto &enemyUnit = gameContext->registry.get<Unit>(enemyEntity);
            Vector2 enemyWorldPos = MapToWorld(enemyUnit.cellIdx, gameContext->cellWidth, gameContext->cellHeight);
            Rectangle enemyRect = {enemyWorldPos.x, enemyWorldPos.y,
                                   static_cast<float>(gameContext->cellWidth),
                                   static_cast<float>(gameContext->cellHeight)};

            if (CheckCollisionTrapezoidRectangle(visionTrap, enemyRect))
            {
                Vector2 enemyCenter = GetRectCenter(enemyRect);
                std::vector<Vector2i> lineCells = GetCellsOverlappingLine(unitCenter, enemyCenter,
                                                                          gameContext->cellWidth,
                                                                          gameContext->cellHeight);
                if (!lineCells.empty())
                    lineCells.erase(lineCells.begin());

                Vector2i blockingCell = {-1, -1};
                for (const auto &cell : lineCells)
                {
                    blockingCell = HasElevationLOS(gameContext, 3.28f, unitComp.cellIdx,
                                                   enemyUnit.cellIdx, cell);
                }

                if ((blockingCell.x == -1 && blockingCell.y == -1) ||
                    blockingCell == enemyUnit.cellIdx)
                {
                    if (!gameContext->registry.all_of<IsVisible>(enemyEntity))
                    {
                        gameContext->registry.emplace<IsVisible>(enemyEntity);
                    }
                }
            }
            else
            {
                if (gameContext->registry.all_of<IsVisible>(enemyEntity))
                {
                    gameContext->registry.remove<IsVisible>(enemyEntity);
                }
            }
        }
    }
}

void ComputeMyTeamsVision(GameContext *gameContext)
{
    switch (gameContext->myPlayer.team)
    {
    case Teams::TEAM_BLUE:
        ComputeTeamVision<TeamBlue, TeamRed>(gameContext);
        break;
    case Teams::TEAM_RED:
        ComputeTeamVision<TeamRed, TeamBlue>(gameContext);
        break;
    }
}