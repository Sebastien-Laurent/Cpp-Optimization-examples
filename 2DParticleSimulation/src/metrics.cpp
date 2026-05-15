#include "metrics.h"

EnergyMetrics ComputeEnergyMetrics(
    const std::vector<Particle>& particles,
    bool isGravityEnabled,
    float screenHeight,
    float gravityAcceleration
)
{
    constexpr float particleMass = 1.0f;
    EnergyMetrics metrics;

    for (const Particle& particle : particles) {
        const float speedSquared =
            particle.velocity.x * particle.velocity.x +
            particle.velocity.y * particle.velocity.y;

        metrics.kineticEnergy += 0.5f * particleMass * speedSquared;

        if (isGravityEnabled) {
            const float heightAboveFloor = (screenHeight - particle.radius) - particle.position.y;
            metrics.potentialEnergy += particleMass * gravityAcceleration * heightAboveFloor;
        }
    }

    metrics.mechanicalEnergy = metrics.kineticEnergy + metrics.potentialEnergy;
    return metrics;
}

void InitializeMetrics(
    MetricsState& metrics,
    const std::vector<Particle>& particles,
    bool isGravityEnabled,
    float screenHeight,
    float gravityAcceleration
)
{
    metrics = {};
    const EnergyMetrics energy = ComputeEnergyMetrics(
        particles,
        isGravityEnabled,
        screenHeight,
        gravityAcceleration
    );

    metrics.kineticEnergy = energy.kineticEnergy;
    metrics.potentialEnergy = energy.potentialEnergy;
    metrics.mechanicalEnergy = energy.mechanicalEnergy;
}

void UpdateMetrics(
    MetricsState& metrics,
    const std::vector<Particle>& particles,
    bool isGravityEnabled,
    const CollisionStats& collisionStats,
    float dt,
    float screenHeight,
    float gravityAcceleration
)
{
    const EnergyMetrics energy = ComputeEnergyMetrics(
        particles,
        isGravityEnabled,
        screenHeight,
        gravityAcceleration
    );

    metrics.sampleTime += dt;
    metrics.kineticEnergySum += energy.kineticEnergy;
    metrics.potentialEnergySum += energy.potentialEnergy;
    metrics.mechanicalEnergySum += energy.mechanicalEnergy;
    metrics.collisionSampleCount += collisionStats.actualCollisions;
    metrics.collisionCandidateCheckSampleCount += collisionStats.candidateChecks;
    ++metrics.energySampleCount;

    if (metrics.sampleTime < METRICS_SAMPLE_PERIOD) {
        return;
    }

    const double sampleCount = static_cast<double>(metrics.energySampleCount);
    metrics.kineticEnergy = static_cast<float>(metrics.kineticEnergySum / sampleCount);
    metrics.potentialEnergy = static_cast<float>(metrics.potentialEnergySum / sampleCount);
    metrics.mechanicalEnergy = static_cast<float>(metrics.mechanicalEnergySum / sampleCount);
    metrics.collisionRate =
        static_cast<float>(metrics.collisionSampleCount) / metrics.sampleTime;
    metrics.collisionCandidateCheckRate =
        static_cast<float>(metrics.collisionCandidateCheckSampleCount) / metrics.sampleTime;

    metrics.sampleTime = 0.0f;
    metrics.kineticEnergySum = 0.0;
    metrics.potentialEnergySum = 0.0;
    metrics.mechanicalEnergySum = 0.0;
    metrics.collisionSampleCount = 0;
    metrics.collisionCandidateCheckSampleCount = 0;
    metrics.energySampleCount = 0;
}
