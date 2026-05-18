#pragma once

#include "raylib.h"

float ClampFloat(float value, float minValue, float maxValue);
float Dot(Vector2 a, Vector2 b);
float Length(Vector2 value);
float LengthSquared(Vector2 value);
Vector2 Add(Vector2 a, Vector2 b);
Vector2 Subtract(Vector2 a, Vector2 b);
Vector2 Scale(Vector2 value, float scale);
Vector2 NormalizeOrZero(Vector2 value);
