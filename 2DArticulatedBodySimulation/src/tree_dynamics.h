#pragma once

#include "articulated_tree.h"
#include "linear_algebra.h"

DenseMatrix ComputeMassMatrix(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
);
std::vector<float> ComputeGravityForces(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q,
    float gravityAcceleration
);
std::vector<float> ComputeVelocityForces(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q,
    const std::vector<float>& qdot
);
std::vector<float> ComputeTreeAcceleration(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q,
    const std::vector<float>& qdot,
    float gravityAcceleration
);
