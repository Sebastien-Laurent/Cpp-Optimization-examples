#include "collision.h"
#include "collision_common.h"

CollisionStats CheckParticleCollisionsBruteForce(std::vector<Particle>& particles)
{
    ResetParticleCollisionFlags(particles);

    CollisionStats stats;

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            ++stats.candidateChecks;

            if (ResolveParticleCollision(particles[i], particles[j])) {
                ++stats.actualCollisions;
            }
        }
    }

    return stats;
}
