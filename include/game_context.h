#pragma once

#include "components.h"
#include "json.hpp"
#include "file_helpers.h"

struct GameContext
{
    int screenWidth = 1280;
    int screenHeight = 800;
    std::string windowTitle = "Open Strategy";

    entt::registry registry;

    int baseFontSize = 16;
    int turnCount = 0;

    Camera2D camera;
    float cameraMoveSpeed = 10.0f;

    nlohmann::json gameSetup;
    nlohmann::json obstacleTemplates;
    nlohmann::json unitTemplates;

    int cellWidth;
    int cellHeight;

    int defaultCellAtlasId;
    Vector2i defaultCellAtlasCoords;

    int cliffIntrinsicHeight;

    std::string currentMap;
    int mapWidth;
    int mapHeight;

    std::unordered_map<std::string, Texture2D> allTextures;
    std::unordered_map<Vector2i, entt::entity> allObstacles;
    std::unordered_map<Vector2i, entt::entity> allUnits;
    std::unordered_map<Vector2i, int> terrainLevels;

    Player myPlayer;

    entt::entity selectedUnit = entt::null;

    GameContext()
    {
        camera = {0};
        camera.target = (Vector2){0, 0};
        camera.offset = (Vector2){0, 0};
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;

        myPlayer.team = Teams::TEAM_BLUE;
        myPlayer.name = "Player 1";
        myPlayer.supplies = 100;
    }

    void LoadAndSetConfig()
    {
        gameSetup = LoadJsonFromFile("config/game_setup.json");
        cellWidth = gameSetup["cell_config"]["cell_width"];
        cellHeight = gameSetup["cell_config"]["cell_height"];
        defaultCellAtlasId = gameSetup["cell_config"]["default_cell_atlas_id"];
        defaultCellAtlasCoords = {gameSetup["cell_config"]["default_cell_atlas_coords"]["x"], gameSetup["cell_config"]["default_cell_atlas_coords"]["y"]};
        cliffIntrinsicHeight = gameSetup["cell_config"]["cliff_intrinsic_height"];

        obstacleTemplates = LoadJsonFromFile("config/obstacle_templates.json");
        unitTemplates = LoadJsonFromFile("config/unit_templates.json");
    }

    void LoadAllTextures()
    {
        std::vector<std::string> obstacleSubdirectories = GetSubdirectoryNamesInDirectory("textures/obstacles");
        for (const auto &subdirectory : obstacleSubdirectories)
        {
            std::vector<std::string> textureFiles = GetFileNamesInDirectory("textures/obstacles/" + subdirectory);
            for (const auto &textureFile : textureFiles)
            {
                std::string texturePath = "textures/obstacles/" + subdirectory + "/" + textureFile;
                std::string textureKey = "obstacles_sheet_" + subdirectory;
                allTextures[textureKey] = LoadTexture(texturePath.c_str());
            }
        }

        std::vector<std::string> unitSubdirectories = GetSubdirectoryNamesInDirectory("textures/units");
        for (const auto &subdirectory : unitSubdirectories)
        {
            std::vector<std::string> textureFiles = GetFileNamesInDirectory("textures/units/" + subdirectory);
            for (const auto &textureFile : textureFiles)
            {
                std::string texturePath = "textures/units/" + subdirectory + "/" + textureFile;
                std::string textureKey = "units_sheet_" + subdirectory;
                allTextures[textureKey] = LoadTexture(texturePath.c_str());
            }
        }
    }

    void UnloadAllTextures()
    {
        {
            for (auto &texture : allTextures)
            {
                UnloadTexture(texture.second);
            }
            allTextures.clear();
        }
    }

    nlohmann::json GetObstacleTemplateByAtlasCoords(const int &atlasId, const Vector2 &atlasCoords)
    {
        for (const auto &templateData : obstacleTemplates.items())
        {
            if (templateData.value()["atlas_id"] == atlasId && (templateData.value()["atlas_coords"]["x"] == atlasCoords.x && templateData.value()["atlas_coords"]["y"] == atlasCoords.y))
            {
                return templateData.value();
            }
        }
        return nlohmann::json(); // Return an empty json object if no match is found
    }

    Rectangle GetCameraViewportWorldRect()
    {
        Vector2 topLeft = GetScreenToWorld2D(Vector2{0, 0}, camera);
        Vector2 bottomRight = GetScreenToWorld2D(Vector2{static_cast<float>(screenWidth), static_cast<float>(screenHeight)}, camera);
        return Rectangle{topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y};
    }
};