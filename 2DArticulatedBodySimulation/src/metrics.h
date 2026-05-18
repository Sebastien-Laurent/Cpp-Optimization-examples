#pragma once

#include "articulated_tree.h"

struct MetricsState {
    float kineticEnergy = 0.0f;
    float potentialEnergy = 0.0f;
    float mechanicalEnergy = 0.0f;
    float initialMechanicalEnergy = 0.0f;
    float energyDrift = 0.0f;
};

void InitializeMetrics(
    MetricsState& metrics,
    const ArticulatedTreeState& tree,
    float gravityAcceleration
);

void UpdateMetrics(
    MetricsState& metrics,
    const ArticulatedTreeState& tree,
    float gravityAcceleration
);
