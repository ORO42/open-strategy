#include "unit_helpers.h"
#include "math_helpers.h"

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

    if (team == Teams::TEAM_BLUE)
    {
        gameContext->registry.emplace<TeamBlue>(unitEntity);
    }
    else if (team == Teams::TEAM_RED)
    {
        gameContext->registry.emplace<TeamRed>(unitEntity);
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

            // remove the first move cell idx
            movePointsComp.moveCellIdxs.erase(movePointsComp.moveCellIdxs.begin());

            if (movePointsComp.moveCellIdxs.size() == 0)
            {
                gameContext->registry.remove<MovePoints>(unitEntity);
            }
        }
    }
}