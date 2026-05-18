#include "articulated_tree.h"

#include <cmath>

ArticulatedTreeState CreateDoublePendulumTree(
    Vector2 anchor,
    float linkLength,
    float linkMass,
    float theta0,
    float theta1
)
{
    return CreateChainTree(anchor, 2, linkLength, linkMass, { theta0, theta1 });
}

ArticulatedTreeState CreateChainTree(
    Vector2 anchor,
    int linkCount,
    float linkLength,
    float linkMass,
    const std::vector<float>& initialAngles
)
{
    ArticulatedTreeState tree;
    tree.anchor = anchor;
    tree.links.reserve(static_cast<size_t>(linkCount));
    tree.q.reserve(static_cast<size_t>(linkCount));

    for (int i = 0; i < linkCount; ++i) {
        tree.links.push_back({ i == 0 ? -1 : i - 1, linkLength, linkMass });
        tree.q.push_back(
            i < static_cast<int>(initialAngles.size()) ?
                initialAngles[static_cast<size_t>(i)] :
                0.0f
        );
    }

    tree.qdot.assign(static_cast<size_t>(linkCount), 0.0f);
    return tree;
}

static bool IsAncestorOfOrSame(const ArticulatedTreeState& tree, size_t ancestor, size_t link)
{
    int current = static_cast<int>(link);
    while (current >= 0) {
        if (static_cast<size_t>(current) == ancestor) {
            return true;
        }

        current = tree.links[static_cast<size_t>(current)].parent;
    }

    return false;
}

std::vector<float> GetTreeWorldAngles(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
)
{
    std::vector<float> angles(tree.links.size(), 0.0f);

    for (size_t i = 0; i < tree.links.size(); ++i) {
        const int parent = tree.links[i].parent;
        angles[i] = q[i] + (parent >= 0 ? angles[static_cast<size_t>(parent)] : 0.0f);
    }

    return angles;
}

std::vector<Vector2> GetTreeJointPositions(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
)
{
    const std::vector<float> angles = GetTreeWorldAngles(tree, q);
    std::vector<Vector2> joints(tree.links.size(), tree.anchor);
    std::vector<Vector2> endpoints(tree.links.size(), tree.anchor);

    for (size_t i = 0; i < tree.links.size(); ++i) {
        const int parent = tree.links[i].parent;
        joints[i] = parent >= 0 ? endpoints[static_cast<size_t>(parent)] : tree.anchor;
        endpoints[i] = {
            joints[i].x + tree.links[i].length * std::sin(angles[i]),
            joints[i].y + tree.links[i].length * std::cos(angles[i])
        };
    }

    return joints;
}

std::vector<Vector2> GetTreeLinkEndPositions(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
)
{
    const std::vector<float> angles = GetTreeWorldAngles(tree, q);
    std::vector<Vector2> endpoints(tree.links.size(), tree.anchor);

    for (size_t i = 0; i < tree.links.size(); ++i) {
        const int parent = tree.links[i].parent;
        const Vector2 joint = parent >= 0 ? endpoints[static_cast<size_t>(parent)] : tree.anchor;
        endpoints[i] = {
            joint.x + tree.links[i].length * std::sin(angles[i]),
            joint.y + tree.links[i].length * std::cos(angles[i])
        };
    }

    return endpoints;
}

EndpointJacobians ComputeEndpointJacobians(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
)
{
    const size_t dof = tree.q.size();
    const std::vector<float> angles = GetTreeWorldAngles(tree, q);
    EndpointJacobians jacobians(
        tree.links.size(),
        std::vector<Vector2>(dof, { 0.0f, 0.0f })
    );

    for (size_t endpoint = 0; endpoint < tree.links.size(); ++endpoint) {
        for (size_t coordinate = 0; coordinate < dof; ++coordinate) {
            Vector2 derivative = { 0.0f, 0.0f };

            int pathLink = static_cast<int>(endpoint);
            while (pathLink >= 0) {
                const size_t pathIndex = static_cast<size_t>(pathLink);
                if (IsAncestorOfOrSame(tree, coordinate, pathIndex)) {
                    derivative.x += tree.links[pathIndex].length * std::cos(angles[pathIndex]);
                    derivative.y -= tree.links[pathIndex].length * std::sin(angles[pathIndex]);
                }

                pathLink = tree.links[pathIndex].parent;
            }

            jacobians[endpoint][coordinate] = derivative;
        }
    }

    return jacobians;
}

EndpointHessians ComputeEndpointHessians(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
)
{
    const size_t dof = tree.q.size();
    const std::vector<float> angles = GetTreeWorldAngles(tree, q);
    EndpointHessians hessians(
        tree.links.size(),
        std::vector<std::vector<Vector2>>(
            dof,
            std::vector<Vector2>(dof, { 0.0f, 0.0f })
        )
    );

    for (size_t endpoint = 0; endpoint < tree.links.size(); ++endpoint) {
        for (size_t coordinateA = 0; coordinateA < dof; ++coordinateA) {
            for (size_t coordinateB = 0; coordinateB < dof; ++coordinateB) {
                Vector2 derivative = { 0.0f, 0.0f };

                int pathLink = static_cast<int>(endpoint);
                while (pathLink >= 0) {
                    const size_t pathIndex = static_cast<size_t>(pathLink);
                    if (IsAncestorOfOrSame(tree, coordinateA, pathIndex) &&
                        IsAncestorOfOrSame(tree, coordinateB, pathIndex)) {
                        derivative.x -= tree.links[pathIndex].length * std::sin(angles[pathIndex]);
                        derivative.y -= tree.links[pathIndex].length * std::cos(angles[pathIndex]);
                    }

                    pathLink = tree.links[pathIndex].parent;
                }

                hessians[endpoint][coordinateA][coordinateB] = derivative;
            }
        }
    }

    return hessians;
}

std::vector<Vector2> GetTreeLinkEndVelocities(const ArticulatedTreeState& tree)
{
    const EndpointJacobians jacobians = ComputeEndpointJacobians(tree, tree.q);
    std::vector<Vector2> velocities(tree.links.size(), { 0.0f, 0.0f });

    for (size_t link = 0; link < tree.links.size(); ++link) {
        for (size_t coordinate = 0; coordinate < tree.qdot.size(); ++coordinate) {
            velocities[link].x += jacobians[link][coordinate].x * tree.qdot[coordinate];
            velocities[link].y += jacobians[link][coordinate].y * tree.qdot[coordinate];
        }
    }

    return velocities;
}
