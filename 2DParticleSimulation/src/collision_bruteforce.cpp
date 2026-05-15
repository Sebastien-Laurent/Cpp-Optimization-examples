#include "collision.h"

#include <cmath>

namespace {

void ResolveCollision(Particle& pA, Particle& pB)
{
    const Vector2 delta = {
        pB.position.x - pA.position.x,
        pB.position.y - pA.position.y
    };

    const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
    const float radiusSum = pA.radius + pB.radius;

    if (distanceSquared > radiusSum * radiusSum) {
        return;
    }

    pA.isColliding = true;
    pB.isColliding = true;

    const float distance = std::sqrt(distanceSquared);

    if (distance == 0.0f) {
        return;
    }

    const Vector2 normal = {
        delta.x / distance,
        delta.y / distance
    };

    const float overlap = radiusSum - distance;
    pA.position.x -= normal.x * overlap * 0.5f;
    pA.position.y -= normal.y * overlap * 0.5f;
    pB.position.x += normal.x * overlap * 0.5f;
    pB.position.y += normal.y * overlap * 0.5f;

    const Vector2 relativeVelocity = {
        pA.velocity.x - pB.velocity.x,
        pA.velocity.y - pB.velocity.y
    };

    const float velocityAlongNormal =
        relativeVelocity.x * normal.x +
        relativeVelocity.y * normal.y;

    if (velocityAlongNormal <= 0.0f) {
        return;
    }

    const Vector2 impulse = {
        velocityAlongNormal * normal.x,
        velocityAlongNormal * normal.y
    };

    pA.velocity.x -= impulse.x;
    pA.velocity.y -= impulse.y;

    pB.velocity.x += impulse.x;
    pB.velocity.y += impulse.y;
}

} // namespace

const char* GetCollisionModeLabel(CollisionMode mode)
{
    switch (mode) {
        case CollisionMode::BruteForce:
            return "Brute Force";
        case CollisionMode::SpatialGrid:
            return "Spatial Grid TODO";
    }

    return "Unknown";
}

CollisionMode GetNextCollisionMode(CollisionMode mode)
{
    switch (mode) {
        case CollisionMode::BruteForce:
            return CollisionMode::SpatialGrid;
        case CollisionMode::SpatialGrid:
            return CollisionMode::BruteForce;
    }

    return CollisionMode::BruteForce;
}

void ResetParticleCollisionFlags(std::vector<Particle>& particles)
{
    for (Particle& particle : particles) {
        particle.isColliding = false;
    }
}

void CheckParticleCollisions(std::vector<Particle>& particles, CollisionMode mode)
{
    switch (mode) {
        case CollisionMode::BruteForce:
            CheckParticleCollisionsBruteForce(particles);
            break;
        case CollisionMode::SpatialGrid:
            CheckParticleCollisionsSpatialGrid(particles);
            break;
    }
}

void CheckParticleCollisionsBruteForce(std::vector<Particle>& particles)
{
    ResetParticleCollisionFlags(particles);

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            ResolveCollision(particles[i], particles[j]);
        }
    }
}
