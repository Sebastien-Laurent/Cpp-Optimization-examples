#include "particle.h"
#include "simulation.h"

#include <cmath>
#include <random>

Particle CreateRandomParticle(float speed)
{
    return {
        RandomPosition(PARTICLE_RADIUS),
        RandomVelocity(speed),
        PARTICLE_RADIUS,
    };
}

Vector2 RandomVelocity(float speed)
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI);

    const float angle = angleDist(rng);
    return {
        speed * std::cos(angle),
        speed * std::sin(angle),
    };
}

Vector2 RandomPosition(float radius)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> posHeightDist(radius, SCREEN_HEIGHT - radius);
    std::uniform_real_distribution<float> posWidthDist(radius, SCREEN_WIDTH - radius);

    return {
        posWidthDist(rng),
        posHeightDist(rng),
    };
}
