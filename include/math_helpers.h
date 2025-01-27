#pragma once

#include "game_context.h"
#include <random>

Vector2i Vector2ToVector2i(const Vector2 &vector2);
Vector2 Vector2iToVector2(const Vector2i &vector2i);
Vector2i WorldToMap(const Vector2 &vector2, const int &cellWidth, const int &cellHeight);
Vector2 MapToWorld(const Vector2i &vector2i, const int &cellWidth, const int &cellHeight);
std::vector<Vector2i> DeduceCellIdxsOverlappingRect(const Rectangle &rect, const int &cellWidth, const int &cellHeight);
Vector2 GetRectCenter(const Rectangle &rect);
int GetChebyshevDistance(const Vector2i &startCellIdx, const Vector2i &endCellIdx);
std::vector<Vector2i> GetBresenhamCells(const Vector2i &startCellIdx, const Vector2i &endCellIdx, const int &cellWidth, const int &cellHeight);
std::vector<Vector2i> GetCellsOverlappingLine(const Vector2 &startWorldPos, const Vector2 &endWorldPos, const int &cellWidth, const int &cellHeight);
Circle GenerateGridBoundCircle(const Vector2 &centerPos, const int &gridRadius, const int &cellWidth, const int &cellHeight);
Rectangle GenerateCellNeighborRect(const Vector2i &centerCellIdx, const int &neighCt, const int &cellWidth, const int &cellHeight);
std::vector<Vector2i> GetCellIdxsOverlappingCircle(const Circle &circle, const int &cellWidth, const int &cellHeight);
bool Chance(const double &probability);
Vector2 RotatePoint(Vector2 origin, Vector2 point, float angle);
void RotateTrapezoid(IsoscelesTrapezoid &trapezoid, float angle);
float AngleDifference(float angle1, float angle2);
float GetAngleBetweenPoints(Vector2 p1, Vector2 p2);
BoundingBox CreateGridCellBoundingBox(float x, float y, float width, float height, float boxHeight);
Vector3 MyVector3Normalize(Vector3 v);
Vector3 MyVector3Subtract(Vector3 v1, Vector3 v2);

template <typename T>
T GetRandomItemFromVector(const std::vector<T> &vec)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, vec.size() - 1);
    return vec[distrib(gen)];
}