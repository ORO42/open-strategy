#include "file_helpers.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

std::vector<std::string> GetFileNamesInDirectory(const std::string &directoryPath)
{
    std::vector<std::string> fileNames;
    for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_regular_file())
        {
            fileNames.push_back(entry.path().filename().string());
        }
    }
    return fileNames;
}

std::vector<std::string> GetSubdirectoryNamesInDirectory(const std::string &directoryPath)
{
    std::vector<std::string> subdirectoryNames;
    for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_directory())
        {
            subdirectoryNames.push_back(entry.path().filename().string());
        }
    }
    return subdirectoryNames;
}

nlohmann::json LoadJsonFromFile(const std::string &filePath)
{
    // JSON object to hold the data
    nlohmann::json jsonData;

    try
    {
        // Open the file
        std::ifstream file(filePath);

        // Check if the file is open
        if (!file.is_open())
        {
            throw std::ios_base::failure("Failed to open the file: " + filePath);
        }

        // Parse the file into the json object
        file >> jsonData;

        // Close the file
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reading JSON file: " << e.what() << "\n";
        throw; // Re-throw the exception for the caller to handle
    }

    return jsonData;
}