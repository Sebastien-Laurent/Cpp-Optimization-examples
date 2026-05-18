#include "metrics.h"

#include <cmath>

static float Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

static float ComputeKineticEnergy(const DoublePendulumState& pendulum)
{
    const std::array<Vector2, 2> velocities = GetDoublePendulumBobVelocities(pendulum);

    return 0.5f * pendulum.mass[0] * Dot(velocities[0], velocities[0]) +
        0.5f * pendulum.mass[1] * Dot(velocities[1], velocities[1]);
}

static float ComputePotentialEnergy(
    const DoublePendulumState& pendulum,
    float gravityAcceleration
)
{
    return
        (pendulum.mass[0] + pendulum.mass[1]) * gravityAcceleration * pendulum.length[0] *
            (1.0f - std::cos(pendulum.theta[0])) +
        pendulum.mass[1] * gravityAcceleration * pendulum.length[1] *
            (1.0f - std::cos(pendulum.theta[1]));
}

void InitializeMetrics(
    MetricsState& metrics,
    const DoublePendulumState& pendulum,
    float gravityAcceleration
)
{
    UpdateMetrics(metrics, pendulum, gravityAcceleration);
    metrics.initialMechanicalEnergy = metrics.mechanicalEnergy;
    metrics.energyDrift = 0.0f;
}

void UpdateMetrics(
    MetricsState& metrics,
    const DoublePendulumState& pendulum,
    float gravityAcceleration
)
{
    metrics.kineticEnergy = ComputeKineticEnergy(pendulum);
    metrics.potentialEnergy = ComputePotentialEnergy(pendulum, gravityAcceleration);
    metrics.mechanicalEnergy = metrics.kineticEnergy + metrics.potentialEnergy;
    metrics.energyDrift = metrics.mechanicalEnergy - metrics.initialMechanicalEnergy;
}
