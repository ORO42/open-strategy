#include "util_helpers.h"

Vector2 Vector2StringToVector2(const std::string &vector2String)
{
    std::string cleanedString = vector2String;
    // Strip spaces and parentheses
    cleanedString.erase(std::remove_if(cleanedString.begin(), cleanedString.end(), [](char c)
                                       { return ::isspace(c) || c == '(' || c == ')'; }),
                        cleanedString.end());

    std::string xString = cleanedString.substr(0, cleanedString.find(","));
    std::string yString = cleanedString.substr(cleanedString.find(",") + 1, cleanedString.length());
    return {std::stof(xString), std::stof(yString)};
}