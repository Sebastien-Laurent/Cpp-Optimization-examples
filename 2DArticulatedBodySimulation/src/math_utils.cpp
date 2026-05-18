#include "math_utils.h"

#include <cmath>

float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) {
        return minValue;
    }

    if (value > maxValue) {
        return maxValue;
    }

    return value;
}

float Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

float LengthSquared(Vector2 value)
{
    return Dot(value, value);
}

float Length(Vector2 value)
{
    return std::sqrt(LengthSquared(value));
}

Vector2 Add(Vector2 a, Vector2 b)
{
    return { a.x + b.x, a.y + b.y };
}

Vector2 Subtract(Vector2 a, Vector2 b)
{
    return { a.x - b.x, a.y - b.y };
}

Vector2 Scale(Vector2 value, float scale)
{
    return { value.x * scale, value.y * scale };
}

Vector2 NormalizeOrZero(Vector2 value)
{
    const float length = Length(value);
    if (length <= 0.00001f) {
        return { 0.0f, 0.0f };
    }

    return Scale(value, 1.0f / length);
}
