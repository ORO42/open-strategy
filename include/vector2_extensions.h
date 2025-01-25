#pragma once

#include "raylib.h"
#include "components.h"
#include <unordered_map>

struct Vector2i
{
    int x, y;
};

// .find() operations will be O(1) average case instead of having to iterate through the container.
// The unordered_map will use the hash function to jump directly to the correct bucket, and then use the equality operator to find the exact match if there are multiple entries in that bucket.
inline bool operator==(const Vector2 &a, const Vector2 &b)
{
    return a.x == b.x && a.y == b.y;
}

namespace std
{
    template <>
    struct hash<Vector2>
    {
        size_t operator()(const Vector2 &v) const
        {
            return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1);
        }
    };
}

// Equality operator for Vector2i
inline bool operator==(const Vector2i &a, const Vector2i &b)
{
    return a.x == b.x && a.y == b.y;
}

// Specialize std::hash for Vector2i
namespace std
{
    template <>
    struct hash<Vector2i>
    {
        size_t operator()(const Vector2i &v) const
        {
            // Combine the hashes of x and y
            return hash<int>()(v.x) ^ (hash<int>()(v.y) << 1);
        }
    };
}
