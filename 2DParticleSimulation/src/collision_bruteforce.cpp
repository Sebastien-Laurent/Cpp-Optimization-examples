#include "collision.h"
#include "collision_common.h"

void CheckParticleCollisionsBruteForce(std::vector<Particle>& particles)
{
    ResetParticleCollisionFlags(particles);

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            ResolveParticleCollision(particles[i], particles[j]);
        }
    }
}
