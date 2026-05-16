#pragma once

#include "particle.h"

#include <vector>

enum class CollisionMode {
    BruteForce,
    SpatialGrid,
};

struct CollisionStats {
    size_t candidateChecks = 0;
    size_t actualCollisions = 0;
};

const char* GetCollisionModeLabel(CollisionMode mode);
CollisionMode GetNextCollisionMode(CollisionMode mode);

void ResetParticleCollisionFlags(std::vector<Particle>& particles);
CollisionStats CheckParticleCollisions(
    std::vector<Particle>& particles,
    CollisionMode mode,
    float restitution
);
CollisionStats CheckParticleCollisionsBruteForce(
    std::vector<Particle>& particles,
    float restitution
);
CollisionStats CheckParticleCollisionsSpatialGrid(
    std::vector<Particle>& particles,
    float restitution
);
