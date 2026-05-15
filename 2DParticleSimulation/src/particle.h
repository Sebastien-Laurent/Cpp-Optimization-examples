#pragma once

#include "raylib.h"

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool isColliding = false;
};

Particle CreateRandomParticle(float speed);
Vector2 RandomVelocity(float speed);
Vector2 RandomPosition(float radius);
