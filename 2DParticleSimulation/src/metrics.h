#pragma once

#include "collision.h"
#include "particle.h"

#include <cstddef>
#include <vector>

constexpr float METRICS_SAMPLE_PERIOD = 1.0f;

struct EnergyMetrics {
    float kineticEnergy = 0.0f;
    float potentialEnergy = 0.0f;
    float mechanicalEnergy = 0.0f;
};

struct MetricsState {
    float kineticEnergy = 0.0f;
    float potentialEnergy = 0.0f;
    float mechanicalEnergy = 0.0f;
    float collisionRate = 0.0f;
    float collisionCandidateCheckRate = 0.0f;

    float sampleTime = 0.0f;
    double kineticEnergySum = 0.0;
    double potentialEnergySum = 0.0;
    double mechanicalEnergySum = 0.0;
    size_t collisionSampleCount = 0;
    size_t collisionCandidateCheckSampleCount = 0;
    size_t energySampleCount = 0;
};

EnergyMetrics ComputeEnergyMetrics(
    const std::vector<Particle>& particles,
    bool isGravityEnabled,
    float screenHeight,
    float gravityAcceleration
);

void InitializeMetrics(
    MetricsState& metrics,
    const std::vector<Particle>& particles,
    bool isGravityEnabled,
    float screenHeight,
    float gravityAcceleration
);

void UpdateMetrics(
    MetricsState& metrics,
    const std::vector<Particle>& particles,
    bool isGravityEnabled,
    const CollisionStats& collisionStats,
    float dt,
    float screenHeight,
    float gravityAcceleration
);
