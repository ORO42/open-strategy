#include "math_helpers.h"

Vector2i Vector2ToVector2i(const Vector2 &vector2)
{
    return (Vector2i){static_cast<int>(std::floor(vector2.x)), static_cast<int>(std::floor(vector2.y))};
}

Vector2 Vector2iToVector2(const Vector2i &vector2i)
{
    return (Vector2){static_cast<float>(vector2i.x), static_cast<float>(vector2i.y)};
}

Vector2i WorldToMap(const Vector2 &vector2, const int &cellWidth, const int &cellHeight)
{
    // Convert world coordinates to grid cell indices
    int col = static_cast<int>(std::floor(vector2.x / cellWidth));
    int row = static_cast<int>(std::floor(vector2.y / cellHeight));

    return {col, row};
}

Vector2 MapToWorld(const Vector2i &vector2i, const int &cellWidth, const int &cellHeight)
{
    // Convert grid cell indices to the world coordinates of the top-left corner of the cell
    float worldX = static_cast<float>(vector2i.x * cellWidth);
    float worldY = static_cast<float>(vector2i.y * cellHeight);

    return {worldX, worldY};
}

std::vector<Vector2i> DeduceCellIdxsOverlappingRect(const Rectangle &rect, const int &cellWidth, const int &cellHeight)
{
    // Extract rectangle bounds
    const float rectX = rect.x;
    const float rectY = rect.y;
    const float rectW = rect.width;
    const float rectH = rect.height;

    // Calculate the grid cell bounds overlapped by the rectangle
    const int startCol = static_cast<int>(rectX / cellWidth);
    const int startRow = static_cast<int>(rectY / cellHeight);
    const int endCol = static_cast<int>((rectX + rectW) / cellWidth);
    const int endRow = static_cast<int>((rectY + rectH) / cellHeight);

    std::vector<Vector2i> overlappingCells;

    // Iterate through all grid cells within the bounds
    for (int row = startRow; row <= endRow; ++row)
    {
        for (int col = startCol; col <= endCol; ++col)
        {
            overlappingCells.push_back({col, row});
        }
    }

    return overlappingCells;
}

Vector2 GetRectCenter(const Rectangle &rect)
{
    return {rect.x + rect.width / 2, rect.y + rect.height / 2};
}

int GetChebyshevDistance(const Vector2i &startCellIdx, const Vector2i &endCellIdx)
{
    return std::max(abs(startCellIdx.x - endCellIdx.x), abs(startCellIdx.y - endCellIdx.y));
}

std::vector<Vector2i> GetBresenhamCells(const Vector2i &startCellIdx, const Vector2i &endCellIdx, const int &cellWidth, const int &cellHeight)
{
    std::vector<Vector2i> cells;

    int x0 = startCellIdx.x;
    int y0 = startCellIdx.y;
    int x1 = endCellIdx.x;
    int y1 = endCellIdx.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        Vector2i cell = {x0, y0};
        cells.push_back(cell);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }

    return cells;
}

std::vector<Vector2i> GetCellsOverlappingLine(const Vector2 &startWorldPos, const Vector2 &endWorldPos, const int &cellWidth, const int &cellHeight)
{
    std::vector<Vector2i> cells;

    // Calculate direction and length of the line
    Vector2 direction = {endWorldPos.x - startWorldPos.x, endWorldPos.y - startWorldPos.y};
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    direction.x /= length;
    direction.y /= length;

    float stepSize = 1.0f;

    // Traverse the line and determine the cells it overlaps
    for (float t = 0; t <= length; t += stepSize)
    {
        Vector2 currentPos = {startWorldPos.x + direction.x * t, startWorldPos.y + direction.y * t};
        Vector2i cell = WorldToMap(currentPos, cellWidth, cellHeight);

        // Check if the cell is already added to the list
        if (std::find(cells.begin(), cells.end(), cell) == cells.end())
        {
            cells.push_back(cell);
        }
    }

    return cells;
}

Circle GenerateGridBoundCircle(const Vector2 &centerPos, const int &gridRadius, const int &cellWidth, const int &cellHeight)
{
    Circle circle;
    circle.centerPos = centerPos;
    if (gridRadius == 0)
    {
        circle.radius = cellWidth / 2;
    }
    else
    {
        circle.radius = gridRadius * cellWidth;
    }
    return circle;
}

Rectangle GenerateCellNeighborRect(const Vector2i &centerCellIdx, const int &neighCt, const int &cellWidth, const int &cellHeight)
{
    // Calculate the top-left corner of the rectangle
    int startX = centerCellIdx.x - neighCt;
    int startY = centerCellIdx.y - neighCt;

    // Calculate the width and height of the rectangle
    int width = (neighCt * 2 + 1) * cellWidth;
    int height = (neighCt * 2 + 1) * cellHeight;

    // Create and return the rectangle
    return Rectangle{static_cast<float>(startX * cellWidth), static_cast<float>(startY * cellHeight), static_cast<float>(width), static_cast<float>(height)};
}

std::vector<Vector2i> GetCellIdxsOverlappingCircle(const Circle &circle, const int &cellWidth, const int &cellHeight)
{
    std::vector<Vector2i> cellIdxs;

    // Calculate the bounding box of the circle
    float left = circle.centerPos.x - circle.radius;
    float right = circle.centerPos.x + circle.radius;
    float top = circle.centerPos.y - circle.radius;
    float bottom = circle.centerPos.y + circle.radius;

    // Convert the bounding box corners to cell indices
    Vector2i topLeftCell = WorldToMap({left, top}, cellWidth, cellHeight);
    Vector2i bottomRightCell = WorldToMap({right, bottom}, cellWidth, cellHeight);

    // Iterate over all cells within the bounding box
    for (int row = topLeftCell.y; row <= bottomRightCell.y; ++row)
    {
        for (int col = topLeftCell.x; col <= bottomRightCell.x; ++col)
        {
            Vector2i currentCell = {col, row};

            // Calculate the center of the current cell
            Vector2 cellCenter = {
                col * cellWidth + cellWidth / 2.0f,
                row * cellHeight + cellHeight / 2.0f};

            // Check if the cell's center is within the circle
            float distanceSquared = (cellCenter.x - circle.centerPos.x) * (cellCenter.x - circle.centerPos.x) +
                                    (cellCenter.y - circle.centerPos.y) * (cellCenter.y - circle.centerPos.y);
            if (distanceSquared <= circle.radius * circle.radius)
            {
                cellIdxs.push_back(currentCell);
            }
        }
    }

    return cellIdxs;
}

bool Chance(const double &probability)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(0.0, 1.0);
    return distrib(gen) < probability;
}

Vector2 RotatePoint(Vector2 origin, Vector2 point, float angle)
{
    float rad = angle * (M_PI / 180.0f); // Convert to radians
    float s = std::sin(rad);
    float c = std::cos(rad);

    // Translate point back to origin
    point.x -= origin.x;
    point.y -= origin.y;

    // Rotate point
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;

    // Translate point back
    point.x = xnew + origin.x;
    point.y = ynew + origin.y;

    return point;
}

float AngleDifference(float angle1, float angle2)
{
    float diff = std::fmod(angle2 - angle1, 360.0f);
    if (diff < -180.0f)
        diff += 360.0f;
    else if (diff > 180.0f)
        diff -= 360.0f;

    return diff;
}

float GetAngleBetweenPoints(Vector2 p1, Vector2 p2)
{
    float deltaY = p2.y - p1.y;
    float deltaX = p2.x - p1.x;
    float angleInRadians = atan2(deltaY, deltaX);
    float angleInDegrees = angleInRadians * (180.0f / M_PI); // Convert radians to degrees
    return angleInDegrees;
}

void RotateTrapezoid(IsoscelesTrapezoid &trapezoid, float angle)
{
    trapezoid.p1 = RotatePoint(trapezoid.originPos, trapezoid.p1, angle);
    trapezoid.p2 = RotatePoint(trapezoid.originPos, trapezoid.p2, angle);
    trapezoid.p3 = RotatePoint(trapezoid.originPos, trapezoid.p3, angle);
    trapezoid.p4 = RotatePoint(trapezoid.originPos, trapezoid.p4, angle);
}