#include "integrators.h"

#include "linear_algebra.h"
#include "tree_dynamics.h"

#include <algorithm>
#include <cmath>

constexpr float NEWTON_DIFFERENCE_STEP = 0.0001f;

const char* GetIntegratorModeLabel(IntegratorMode mode)
{
    switch (mode) {
    case IntegratorMode::ExplicitEuler:
        return "Explicit Euler";
    case IntegratorMode::SemiImplicitEuler:
        return "Semi-Implicit Euler";
    case IntegratorMode::Rk4:
        return "RK4";
    case IntegratorMode::ImplicitEuler:
        return "Implicit Euler";
    case IntegratorMode::ImplicitMidpoint:
        return "Implicit Midpoint";
    }

    return "Unknown";
}

IntegratorMode GetNextIntegratorMode(IntegratorMode mode)
{
    switch (mode) {
    case IntegratorMode::ExplicitEuler:
        return IntegratorMode::SemiImplicitEuler;
    case IntegratorMode::SemiImplicitEuler:
        return IntegratorMode::Rk4;
    case IntegratorMode::Rk4:
        return IntegratorMode::ImplicitEuler;
    case IntegratorMode::ImplicitEuler:
        return IntegratorMode::ImplicitMidpoint;
    case IntegratorMode::ImplicitMidpoint:
        return IntegratorMode::ExplicitEuler;
    }

    return IntegratorMode::ExplicitEuler;
}

static std::vector<float> ComputeDerivative(
    const ArticulatedTreeState& tree,
    const std::vector<float>& state,
    float gravityAcceleration
)
{
    const std::vector<float> q = UnpackQ(state);
    const std::vector<float> qdot = UnpackQdot(state);
    const std::vector<float> qddot = ComputeTreeAcceleration(
        tree,
        q,
        qdot,
        gravityAcceleration
    );

    return PackState(qdot, qddot);
}

static std::vector<float> AddScaled(
    const std::vector<float>& state,
    const std::vector<float>& derivative,
    float scale
)
{
    std::vector<float> result(state.size(), 0.0f);
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = state[i] + derivative[i] * scale;
    }

    return result;
}

static std::vector<float> ComputeImplicitEulerResidual(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q0,
    const std::vector<float>& qdot0,
    const std::vector<float>& x,
    float gravityAcceleration,
    float dt
)
{
    const std::vector<float> qNext = UnpackQ(x);
    const std::vector<float> qdotNext = UnpackQdot(x);
    const std::vector<float> acceleration = ScaleVector(SubtractVectors(qdotNext, qdot0), 1.0f / dt);
    const DenseMatrix massMatrix = ComputeMassMatrix(tree, qNext);
    const std::vector<float> gravityForces =
        ComputeGravityForces(tree, qNext, gravityAcceleration);
    const std::vector<float> velocityForces =
        ComputeVelocityForces(tree, qNext, qdotNext);
    const size_t dof = q0.size();

    std::vector<float> residual(2 * dof, 0.0f);
    for (size_t i = 0; i < dof; ++i) {
        residual[i] = qNext[i] - q0[i] - dt * qdotNext[i];
        residual[dof + i] = velocityForces[i] - gravityForces[i];
        for (size_t j = 0; j < dof; ++j) {
            residual[dof + i] += massMatrix[i][j] * acceleration[j];
        }
    }

    return residual;
}

static std::vector<float> ComputeImplicitMidpointResidual(
    const ArticulatedTreeState& tree,
    const std::vector<float>& q0,
    const std::vector<float>& qdot0,
    const std::vector<float>& x,
    float gravityAcceleration,
    float dt
)
{
    const std::vector<float> qNext = UnpackQ(x);
    const std::vector<float> qdotNext = UnpackQdot(x);
    const std::vector<float> qMid = ScaleVector(AddVectors(q0, qNext), 0.5f);
    const std::vector<float> qdotMid = ScaleVector(AddVectors(qdot0, qdotNext), 0.5f);
    const std::vector<float> acceleration = ScaleVector(SubtractVectors(qdotNext, qdot0), 1.0f / dt);
    const DenseMatrix massMatrix = ComputeMassMatrix(tree, qMid);
    const std::vector<float> gravityForces =
        ComputeGravityForces(tree, qMid, gravityAcceleration);
    const std::vector<float> velocityForces =
        ComputeVelocityForces(tree, qMid, qdotMid);
    const size_t dof = q0.size();

    std::vector<float> residual(2 * dof, 0.0f);
    for (size_t i = 0; i < dof; ++i) {
        residual[i] = qNext[i] - q0[i] - dt * qdotMid[i];
        residual[dof + i] = velocityForces[i] - gravityForces[i];
        for (size_t j = 0; j < dof; ++j) {
            residual[dof + i] += massMatrix[i][j] * acceleration[j];
        }
    }

    return residual;
}

template <typename ResidualFn>
static std::vector<float> SolveNewton(
    const std::vector<float>& initialGuess,
    int nonlinearIterations,
    ResidualFn computeResidual
)
{
    std::vector<float> x = initialGuess;
    const size_t n = x.size();

    for (int iteration = 0; iteration < nonlinearIterations; ++iteration) {
        const std::vector<float> residual = computeResidual(x);
        DenseMatrix jacobian(n, std::vector<float>(n, 0.0f));

        for (size_t column = 0; column < n; ++column) {
            std::vector<float> shifted = x;
            shifted[column] += NEWTON_DIFFERENCE_STEP;
            const std::vector<float> shiftedResidual = computeResidual(shifted);

            for (size_t row = 0; row < n; ++row) {
                jacobian[row][column] =
                    (shiftedResidual[row] - residual[row]) / NEWTON_DIFFERENCE_STEP;
            }
        }

        std::vector<float> rhs = ScaleVector(residual, -1.0f);
        std::vector<float> delta(n, 0.0f);
        if (!SolveLinearSystem(jacobian, rhs, delta)) {
            break;
        }

        float maxDelta = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            x[i] += delta[i];
            maxDelta = std::max(maxDelta, std::fabs(delta[i]));
        }

        if (maxDelta <= 0.000001f) {
            break;
        }
    }

    return x;
}

static void StepExplicitEuler(
    ArticulatedTreeState& tree,
    float gravityAcceleration,
    float dt
)
{
    const std::vector<float> acceleration = ComputeTreeAcceleration(
        tree,
        tree.q,
        tree.qdot,
        gravityAcceleration
    );

    tree.q = AddVectors(tree.q, ScaleVector(tree.qdot, dt));
    tree.qdot = AddVectors(tree.qdot, ScaleVector(acceleration, dt));
}

static void StepSemiImplicitEuler(
    ArticulatedTreeState& tree,
    float gravityAcceleration,
    float dt
)
{
    const std::vector<float> acceleration = ComputeTreeAcceleration(
        tree,
        tree.q,
        tree.qdot,
        gravityAcceleration
    );

    tree.qdot = AddVectors(tree.qdot, ScaleVector(acceleration, dt));
    tree.q = AddVectors(tree.q, ScaleVector(tree.qdot, dt));
}

static void StepRk4(ArticulatedTreeState& tree, float gravityAcceleration, float dt)
{
    const std::vector<float> state = PackState(tree.q, tree.qdot);
    const std::vector<float> k1 = ComputeDerivative(tree, state, gravityAcceleration);
    const std::vector<float> k2 = ComputeDerivative(
        tree,
        AddScaled(state, k1, 0.5f * dt),
        gravityAcceleration
    );
    const std::vector<float> k3 = ComputeDerivative(
        tree,
        AddScaled(state, k2, 0.5f * dt),
        gravityAcceleration
    );
    const std::vector<float> k4 = ComputeDerivative(
        tree,
        AddScaled(state, k3, dt),
        gravityAcceleration
    );

    std::vector<float> nextState(state.size(), 0.0f);
    for (size_t i = 0; i < nextState.size(); ++i) {
        nextState[i] = state[i] + dt * (k1[i] + 2.0f * k2[i] + 2.0f * k3[i] + k4[i]) / 6.0f;
    }

    tree.q = UnpackQ(nextState);
    tree.qdot = UnpackQdot(nextState);
}

static void StepImplicitEuler(
    ArticulatedTreeState& tree,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
)
{
    const std::vector<float> q0 = tree.q;
    const std::vector<float> qdot0 = tree.qdot;
    const std::vector<float> acceleration0 = ComputeTreeAcceleration(
        tree,
        q0,
        qdot0,
        gravityAcceleration
    );
    const std::vector<float> initialGuess = PackState(
        AddVectors(q0, ScaleVector(qdot0, dt)),
        AddVectors(qdot0, ScaleVector(acceleration0, dt))
    );

    const std::vector<float> solution = SolveNewton(
        initialGuess,
        nonlinearIterations,
        [&](const std::vector<float>& x) {
            return ComputeImplicitEulerResidual(
                tree,
                q0,
                qdot0,
                x,
                gravityAcceleration,
                dt
            );
        }
    );

    tree.q = UnpackQ(solution);
    tree.qdot = UnpackQdot(solution);
}

static void StepImplicitMidpoint(
    ArticulatedTreeState& tree,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
)
{
    const std::vector<float> q0 = tree.q;
    const std::vector<float> qdot0 = tree.qdot;
    const std::vector<float> acceleration0 = ComputeTreeAcceleration(
        tree,
        q0,
        qdot0,
        gravityAcceleration
    );
    const std::vector<float> initialGuess = PackState(
        AddVectors(q0, ScaleVector(qdot0, dt)),
        AddVectors(qdot0, ScaleVector(acceleration0, dt))
    );

    const std::vector<float> solution = SolveNewton(
        initialGuess,
        nonlinearIterations,
        [&](const std::vector<float>& x) {
            return ComputeImplicitMidpointResidual(
                tree,
                q0,
                qdot0,
                x,
                gravityAcceleration,
                dt
            );
        }
    );

    tree.q = UnpackQ(solution);
    tree.qdot = UnpackQdot(solution);
}

void StepArticulatedTree(
    ArticulatedTreeState& tree,
    IntegratorMode integratorMode,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
)
{
    switch (integratorMode) {
    case IntegratorMode::ExplicitEuler:
        StepExplicitEuler(tree, gravityAcceleration, dt);
        break;
    case IntegratorMode::SemiImplicitEuler:
        StepSemiImplicitEuler(tree, gravityAcceleration, dt);
        break;
    case IntegratorMode::Rk4:
        StepRk4(tree, gravityAcceleration, dt);
        break;
    case IntegratorMode::ImplicitEuler:
        StepImplicitEuler(tree, gravityAcceleration, dt, nonlinearIterations);
        break;
    case IntegratorMode::ImplicitMidpoint:
        StepImplicitMidpoint(tree, gravityAcceleration, dt, nonlinearIterations);
        break;
    }
}
