#pragma once

#include "raylib.h"

#include <vector>

struct TreeLink {
    int parent = -1;
    float length = 1.0f;
    float mass = 1.0f;
};

struct ArticulatedTreeState {
    Vector2 anchor = { 0.0f, 0.0f };
    std::vector<TreeLink> links;
    std::vector<float> q;
    std::vector<float> qdot;
};

using EndpointJacobians = std::vector<std::vector<Vector2>>;
using EndpointHessians = std::vector<std::vector<std::vector<Vector2>>>;

ArticulatedTreeState CreateDoublePendulumTree(
    Vector2 anchor,
    float linkLength,
    float linkMass,
    float theta0,
    float theta1
);
ArticulatedTreeState CreateChainTree(
    Vector2 anchor,
    int linkCount,
    float linkLength,
    float linkMass,
    const std::vector<float>& initialAngles
);

std::vector<float> GetTreeWorldAngles(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
);
std::vector<Vector2> GetTreeJointPositions(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
);
std::vector<Vector2> GetTreeLinkEndPositions(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
);
EndpointJacobians ComputeEndpointJacobians(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
);
EndpointHessians ComputeEndpointHessians(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
);
std::vector<Vector2> GetTreeLinkEndVelocities(const ArticulatedTreeState& tree);
