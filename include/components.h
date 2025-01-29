#pragma once

#include "raylib.h"
#include "entt.hpp"
#include "vector2_extensions.h"

enum struct Teams
{
    TEAM_BLUE,
    TEAM_RED,
};

enum struct Stances
{
    STANDING,
    CROUCHED,
    PRONE,
    NONE,
};

struct Player
{
    std::string name;
    Teams team;
    int supplies;
};

struct Obstacle
{
    std::string type;
    std::string displayName;
    int moveCostSupplies;
    int intrinsicHeight;
    Vector2i cellIdx;
    Rectangle collisionRect;
    bool isDestructible;
    int maxHealth;
    int currentHealth;
    bool stopsProjectile;
    int atlasId;
    Vector2i atlasCoords;
    bool unitStandsOnTop;
};

struct Ability
{
    std::string type;
    std::string description;
    bool requiresCell;
    int supplyCost;
    int maxUsesPerTurn;
    int usesThisTurn = 0;
    int maxCooldown;
    int lastTurnUsed = -1;
    bool doesBresenhamTargeting;
    bool doesStraightLineTargeting;
    int range;
    int aoeSize;
    int fleshDamageMax;
    int fleshDamageMin;
    int armorDamageMax;
    int armorDamageMin;
    int terrainDamageMax;
    int terrainDamageMin;
    bool firesProjectile;
    bool isAerialProjectile;
    float accuracyFalloff;
    int inaccuracyRadius;
    std::string createsUnit;
    std::string tileEffect;
    float suppression;
    float suppressionChance;
    int suppressionRadius;
};

struct Unit
{
    std::string type;
    std::string description;
    std::string givenName;
    Vector2i cellIdx;
    int intrinsicHeight;
    int crouchHeight;
    int proneHeight;
    int maxSupplies;
    int supplies;
    Rectangle collisionRect;
    float facingAngle = -90.0f;
    int maxHealth;
    int currentHealth;
    int maxOccupancy;
    bool stopsProjectile;
    Stances stance;
    bool isPerson;
    bool isVehicle;
    bool isStructure;
    int atlasId;
    Vector2i atlasCoords;
    std::vector<Ability> abilities;
    int selectedAbilityIdx = -1;
    Ability *selectedAbility = nullptr;
    Teams team;
};

struct MovePoints
{
    std::vector<Vector2i> moveCellIdxs;
};

struct TeamBlue
{
};

struct TeamRed
{
};

struct ShouldDestroy
{
};

struct IsVisible
{
};

struct PopupText
{
    std::string text;
    Vector2 position;
    Color color; // NOTE: if my team deals damage, GREEN, if my team takes damage, RED
    bool useFade;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::duration<double> maxDuration;
};

struct Circle
{
    Vector2 centerPos;
    float radius;
};

struct CellSummary
{
    entt::entity unit = entt::null;
    entt::entity obstacle = entt::null;

    bool unitStopsProjectile = false;
    bool obstacleStopsProjectile = false;

    int terrainLevel;
    int terrainHeight;
    int unitIntrinsicHeight;
    int topMostObstacleIntrinsicHeight;
    int totalHeightIncludingTopMostObstacleExcludingUnit;
    int totalHeightofUnit;
    int totalHeightForCellIdx;
};

struct PathSummary
{
    std::map<Vector2i, CellSummary> cellSummaries;
};

struct IsoscelesTrapezoid
{
    int baseWidth = 1;
    int topWidth = 1;
    int length = 1;

    float facingAngle; // direction the top edge of the trapezoid is facing

    Vector2 originPos; // center of base
    Vector2 p1;        // base left
    Vector2 p2;        // base right
    Vector2 p3;        // top right
    Vector2 p4;        // top left
};

struct ScrollPanel
{
    Rectangle rect;
    std::vector<std::vector<std::string>> items;
};