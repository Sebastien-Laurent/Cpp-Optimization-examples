#include "collision.h"
#include "collision_common.h"

CollisionStats CheckParticleCollisionsBruteForce(
    std::vector<Particle>& particles,
    float restitution
)
{
    ResetParticleCollisionFlags(particles);

    CollisionStats stats;

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            ++stats.candidateChecks;

            if (ResolveParticleCollision(particles[i], particles[j], restitution)) {
                ++stats.actualCollisions;
            }
        }
    }

    return stats;
}
