#pragma once

#include "particle.h"

#include <vector>

enum class CollisionMode {
    BruteForce,
    SpatialGrid,
};

const char* GetCollisionModeLabel(CollisionMode mode);
CollisionMode GetNextCollisionMode(CollisionMode mode);

void ResetParticleCollisionFlags(std::vector<Particle>& particles);
void CheckParticleCollisions(std::vector<Particle>& particles, CollisionMode mode);
void CheckParticleCollisionsBruteForce(std::vector<Particle>& particles);
void CheckParticleCollisionsSpatialGrid(std::vector<Particle>& particles);
