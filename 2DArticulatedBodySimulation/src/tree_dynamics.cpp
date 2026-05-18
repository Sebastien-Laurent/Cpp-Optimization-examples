#include "tree_dynamics.h"

static float Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

DenseMatrix ComputeMassMatrix(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q
)
{
    const size_t dof = tree.q.size();
    DenseMatrix massMatrix(dof, std::vector<float>(dof, 0.0f));
    const EndpointJacobians jacobians = ComputeEndpointJacobians(tree, q);

    for (size_t link = 0; link < tree.links.size(); ++link) {
        for (size_t row = 0; row < dof; ++row) {
            for (size_t column = 0; column < dof; ++column) {
                massMatrix[row][column] +=
                    tree.links[link].mass * Dot(jacobians[link][row], jacobians[link][column]);
            }
        }
    }

    return massMatrix;
}

std::vector<float> ComputeGravityForces(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q,
    float gravityAcceleration
)
{
    const size_t dof = tree.q.size();
    std::vector<float> forces(dof, 0.0f);
    const EndpointJacobians jacobians = ComputeEndpointJacobians(tree, q);
    const Vector2 gravityForceDirection = { 0.0f, gravityAcceleration };

    for (size_t link = 0; link < tree.links.size(); ++link) {
        for (size_t coordinate = 0; coordinate < dof; ++coordinate) {
            forces[coordinate] +=
                tree.links[link].mass *
                Dot(jacobians[link][coordinate], gravityForceDirection);
        }
    }

    return forces;
}

std::vector<float> ComputeVelocityForces(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q,
    const std::vector<float>& qdot
)
{
    const size_t dof = tree.q.size();
    const EndpointJacobians jacobians = ComputeEndpointJacobians(tree, q);
    const EndpointHessians hessians = ComputeEndpointHessians(tree, q);
    std::vector<DenseMatrix> massDerivatives(
        dof,
        DenseMatrix(dof, std::vector<float>(dof, 0.0f))
    );

    for (size_t derivativeCoordinate = 0; derivativeCoordinate < dof; ++derivativeCoordinate) {
        for (size_t row = 0; row < dof; ++row) {
            for (size_t column = 0; column < dof; ++column) {
                for (size_t endpoint = 0; endpoint < tree.links.size(); ++endpoint) {
                    massDerivatives[derivativeCoordinate][row][column] +=
                        tree.links[endpoint].mass *
                        (
                            Dot(hessians[endpoint][row][derivativeCoordinate], jacobians[endpoint][column]) +
                            Dot(jacobians[endpoint][row], hessians[endpoint][column][derivativeCoordinate])
                        );
                }
            }
        }
    }

    std::vector<float> velocityForces(dof, 0.0f);
    for (size_t i = 0; i < dof; ++i) {
        for (size_t j = 0; j < dof; ++j) {
            for (size_t k = 0; k < dof; ++k) {
                const float christoffel =
                    0.5f *
                    (massDerivatives[k][i][j] +
                     massDerivatives[j][i][k] -
                     massDerivatives[i][j][k]);
                velocityForces[i] += christoffel * qdot[j] * qdot[k];
            }
        }
    }

    return velocityForces;
}

std::vector<float> ComputeTreeAcceleration(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q,
    const std::vector<float>& qdot,
    float gravityAcceleration
)
{
    const DenseMatrix massMatrix = ComputeMassMatrix(tree, q);
    const std::vector<float> gravityForces =
        ComputeGravityForces(tree, q, gravityAcceleration);
    const std::vector<float> velocityForces = ComputeVelocityForces(tree, q, qdot);
    const std::vector<float> rhs = SubtractVectors(gravityForces, velocityForces);

    std::vector<float> acceleration(q.size(), 0.0f);
    SolveLinearSystem(massMatrix, rhs, acceleration);
    return acceleration;
}
