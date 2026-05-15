#include "simulation.h"

#include <algorithm>
#include <cmath>

void InitializeAppState(AppState& app)
{
    app.particles.clear();
    app.particles.reserve(INITIAL_PARTICLE_COUNT);

    for (int i = 0; i < INITIAL_PARTICLE_COUNT; ++i) {
        app.particles.push_back(CreateRandomParticle(app.speed));
    }
}

void TickSimulation(AppState& app, float dt)
{
    IntegrateParticles(app.particles, dt, app.isGravityEnabled);

    if (app.isGravityEnabled) {
        ApplyGravity(app.particles, dt);
    }

    ResolveWallCollisions(app.particles, app.isGravityEnabled);

    if (app.isParticleCollisionEnabled) {
        CheckParticleCollisions(app.particles, app.collisionMode);
    } else {
        ResetParticleCollisionFlags(app.particles);
    }
}

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

float ComputeTotalKineticEnergy(const std::vector<Particle>& particles)
{
    constexpr float particleMass = 1.0f;
    float totalKineticEnergy = 0.0f;

    for (const Particle& particle : particles) {
        const float speedSquared =
            particle.velocity.x * particle.velocity.x +
            particle.velocity.y * particle.velocity.y;

        totalKineticEnergy += 0.5f * particleMass * speedSquared;
    }

    return totalKineticEnergy;
}

float ComputeTotalPotentialEnergy(const std::vector<Particle>& particles, bool isGravityEnabled)
{
    if (!isGravityEnabled) {
        return 0.0f;
    }

    constexpr float particleMass = 1.0f;
    float totalPotentialEnergy = 0.0f;

    for (const Particle& particle : particles) {
        const float heightAboveFloor = (SCREEN_HEIGHT - particle.radius) - particle.position.y;
        totalPotentialEnergy += particleMass * GRAVITY_ACCELERATION * heightAboveFloor;
    }

    return totalPotentialEnergy;
}

float ComputeTotalMechanicalEnergy(const std::vector<Particle>& particles, bool isGravityEnabled)
{
    return ComputeTotalKineticEnergy(particles) +
           ComputeTotalPotentialEnergy(particles, isGravityEnabled);
}

void ApplyGravity(std::vector<Particle>& particles, float dt)
{
    for (Particle& particle : particles) {
        particle.velocity.y += GRAVITY_ACCELERATION * dt;
    }
}

void IntegrateParticles(std::vector<Particle>& particles, float dt, bool isGravityEnabled)
{
    for (Particle& particle : particles) {
        particle.position.x += dt * particle.velocity.x;
        particle.position.y += dt * particle.velocity.y;

        if (isGravityEnabled) {
            particle.position.y += 0.5f * GRAVITY_ACCELERATION * dt * dt;
        }
    }
}

void ResolveWallCollisions(std::vector<Particle>& particles, bool gravityOn)
{
    for (Particle& particle : particles) {
        if (particle.position.x > SCREEN_WIDTH - particle.radius ||
            particle.position.x < particle.radius) {
            particle.velocity.x = -particle.velocity.x;
            particle.position.x = ClampFloat(
                particle.position.x,
                particle.radius,
                SCREEN_WIDTH - particle.radius
            );
        }

        if (particle.position.y > SCREEN_HEIGHT - particle.radius ||
            particle.position.y < particle.radius) {
            const float clampedY = ClampFloat(
                particle.position.y,
                particle.radius,
                SCREEN_HEIGHT - particle.radius
            );

            const float delta = clampedY - particle.position.y;
            particle.position.y = clampedY;

            if (!gravityOn) {
                particle.velocity.y = -particle.velocity.y;
            } else {
                const float speedSquaredAtWall =
                    particle.velocity.y * particle.velocity.y +
                    2.0f * GRAVITY_ACCELERATION * delta;
                const float speedAtWall = std::sqrt(std::max(0.0f, speedSquaredAtWall));

                if (clampedY == SCREEN_HEIGHT - particle.radius) {
                    particle.velocity.y = -speedAtWall;
                } else {
                    particle.velocity.y = speedAtWall;
                }
            }
        }
    }
}
