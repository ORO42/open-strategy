#pragma once

#include <iostream>
#include <filesystem>
#include <vector>
#include "json.hpp"

std::vector<std::string> GetFileNamesInDirectory(const std::string &directoryPath);
std::vector<std::string> GetSubdirectoryNamesInDirectory(const std::string &directoryPath);
nlohmann::json LoadJsonFromFile(const std::string &filePath);