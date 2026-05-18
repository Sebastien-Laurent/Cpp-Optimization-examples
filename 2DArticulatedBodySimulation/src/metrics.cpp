#include "metrics.h"

static float Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

static float ComputeKineticEnergy(const ArticulatedTreeState& tree)
{
    const std::vector<Vector2> velocities = GetTreeLinkEndVelocities(tree);
    float energy = 0.0f;

    for (size_t link = 0; link < tree.links.size(); ++link) {
        energy += 0.5f * tree.links[link].mass * Dot(velocities[link], velocities[link]);
    }

    return energy;
}

static float ComputePotentialEnergy(
    const ArticulatedTreeState& tree,
    float gravityAcceleration
)
{
    const std::vector<Vector2> endpoints = GetTreeLinkEndPositions(tree, tree.q);
    float energy = 0.0f;

    for (size_t link = 0; link < tree.links.size(); ++link) {
        energy += -tree.links[link].mass * gravityAcceleration * endpoints[link].y;
    }

    return energy;
}

void InitializeMetrics(
    MetricsState& metrics,
    const ArticulatedTreeState& tree,
    float gravityAcceleration
)
{
    UpdateMetrics(metrics, tree, gravityAcceleration);
    metrics.initialMechanicalEnergy = metrics.mechanicalEnergy;
    metrics.energyDrift = 0.0f;
}

void UpdateMetrics(
    MetricsState& metrics,
    const ArticulatedTreeState& tree,
    float gravityAcceleration
)
{
    metrics.kineticEnergy = ComputeKineticEnergy(tree);
    metrics.potentialEnergy = ComputePotentialEnergy(tree, gravityAcceleration);
    metrics.mechanicalEnergy = metrics.kineticEnergy + metrics.potentialEnergy;
    metrics.energyDrift = metrics.mechanicalEnergy - metrics.initialMechanicalEnergy;
}
