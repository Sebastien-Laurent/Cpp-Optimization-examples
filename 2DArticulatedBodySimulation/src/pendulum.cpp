#include "pendulum.h"

#include <algorithm>
#include <cmath>

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

static Vec2State Add(Vec2State a, Vec2State b)
{
    return { a[0] + b[0], a[1] + b[1] };
}

static Vec2State Subtract(Vec2State a, Vec2State b)
{
    return { a[0] - b[0], a[1] - b[1] };
}

static Vec2State Scale(Vec2State value, float scale)
{
    return { value[0] * scale, value[1] * scale };
}

static Vec4State PackState(Vec2State theta, Vec2State omega)
{
    return { theta[0], theta[1], omega[0], omega[1] };
}

static Vec2State UnpackTheta(Vec4State state)
{
    return { state[0], state[1] };
}

static Vec2State UnpackOmega(Vec4State state)
{
    return { state[2], state[3] };
}

static void ComputeMassMatrix(
    const DoublePendulumState& pendulum,
    const Vec2State& theta,
    float massMatrix[2][2]
)
{
    const float m1 = pendulum.mass[0];
    const float m2 = pendulum.mass[1];
    const float l1 = pendulum.length[0];
    const float l2 = pendulum.length[1];
    const float coupling = m2 * l1 * l2 * std::cos(theta[0] - theta[1]);

    massMatrix[0][0] = (m1 + m2) * l1 * l1;
    massMatrix[0][1] = coupling;
    massMatrix[1][0] = coupling;
    massMatrix[1][1] = m2 * l2 * l2;
}

static Vec2State ComputeBiasForces(
    const DoublePendulumState& pendulum,
    const Vec2State& theta,
    const Vec2State& omega,
    float gravityAcceleration
)
{
    const float m1 = pendulum.mass[0];
    const float m2 = pendulum.mass[1];
    const float l1 = pendulum.length[0];
    const float l2 = pendulum.length[1];
    const float delta = theta[0] - theta[1];
    const float coupling = m2 * l1 * l2 * std::sin(delta);

    return {
        coupling * omega[1] * omega[1] +
            (m1 + m2) * gravityAcceleration * l1 * std::sin(theta[0]),
        -coupling * omega[0] * omega[0] +
            m2 * gravityAcceleration * l2 * std::sin(theta[1])
    };
}

static Vec2State Solve2x2(float matrix[2][2], Vec2State rhs)
{
    const float determinant = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
    if (std::fabs(determinant) <= 0.000001f) {
        return { 0.0f, 0.0f };
    }

    return {
        (rhs[0] * matrix[1][1] - matrix[0][1] * rhs[1]) / determinant,
        (matrix[0][0] * rhs[1] - rhs[0] * matrix[1][0]) / determinant
    };
}

Vec2State ComputeDoublePendulumAcceleration(
    const DoublePendulumState& pendulum,
    const Vec2State& theta,
    const Vec2State& omega,
    float gravityAcceleration
)
{
    float massMatrix[2][2];
    ComputeMassMatrix(pendulum, theta, massMatrix);

    const Vec2State bias = ComputeBiasForces(
        pendulum,
        theta,
        omega,
        gravityAcceleration
    );

    return Solve2x2(massMatrix, { -bias[0], -bias[1] });
}

std::array<Vector2, 2> GetDoublePendulumBobPositions(const DoublePendulumState& pendulum)
{
    const Vector2 first = {
        pendulum.anchor.x + pendulum.length[0] * std::sin(pendulum.theta[0]),
        pendulum.anchor.y + pendulum.length[0] * std::cos(pendulum.theta[0])
    };
    const Vector2 second = {
        first.x + pendulum.length[1] * std::sin(pendulum.theta[1]),
        first.y + pendulum.length[1] * std::cos(pendulum.theta[1])
    };

    return { first, second };
}

std::array<Vector2, 2> GetDoublePendulumBobVelocities(const DoublePendulumState& pendulum)
{
    const Vector2 first = {
        pendulum.length[0] * pendulum.omega[0] * std::cos(pendulum.theta[0]),
        -pendulum.length[0] * pendulum.omega[0] * std::sin(pendulum.theta[0])
    };
    const Vector2 second = {
        first.x + pendulum.length[1] * pendulum.omega[1] * std::cos(pendulum.theta[1]),
        first.y - pendulum.length[1] * pendulum.omega[1] * std::sin(pendulum.theta[1])
    };

    return { first, second };
}

static Vec4State ComputeDerivative(
    const DoublePendulumState& pendulum,
    Vec4State state,
    float gravityAcceleration
)
{
    const Vec2State theta = UnpackTheta(state);
    const Vec2State omega = UnpackOmega(state);
    const Vec2State acceleration = ComputeDoublePendulumAcceleration(
        pendulum,
        theta,
        omega,
        gravityAcceleration
    );

    return { omega[0], omega[1], acceleration[0], acceleration[1] };
}

static Vec4State AddScaled(Vec4State state, Vec4State derivative, float scale)
{
    return {
        state[0] + derivative[0] * scale,
        state[1] + derivative[1] * scale,
        state[2] + derivative[2] * scale,
        state[3] + derivative[3] * scale
    };
}

static void StepExplicitEuler(DoublePendulumState& pendulum, float gravityAcceleration, float dt)
{
    const Vec2State acceleration = ComputeDoublePendulumAcceleration(
        pendulum,
        pendulum.theta,
        pendulum.omega,
        gravityAcceleration
    );

    pendulum.theta = Add(pendulum.theta, Scale(pendulum.omega, dt));
    pendulum.omega = Add(pendulum.omega, Scale(acceleration, dt));
}

static void StepSemiImplicitEuler(DoublePendulumState& pendulum, float gravityAcceleration, float dt)
{
    const Vec2State acceleration = ComputeDoublePendulumAcceleration(
        pendulum,
        pendulum.theta,
        pendulum.omega,
        gravityAcceleration
    );

    pendulum.omega = Add(pendulum.omega, Scale(acceleration, dt));
    pendulum.theta = Add(pendulum.theta, Scale(pendulum.omega, dt));
}

static void StepRk4(DoublePendulumState& pendulum, float gravityAcceleration, float dt)
{
    const Vec4State state = PackState(pendulum.theta, pendulum.omega);
    const Vec4State k1 = ComputeDerivative(pendulum, state, gravityAcceleration);
    const Vec4State k2 = ComputeDerivative(
        pendulum,
        AddScaled(state, k1, 0.5f * dt),
        gravityAcceleration
    );
    const Vec4State k3 = ComputeDerivative(
        pendulum,
        AddScaled(state, k2, 0.5f * dt),
        gravityAcceleration
    );
    const Vec4State k4 = ComputeDerivative(
        pendulum,
        AddScaled(state, k3, dt),
        gravityAcceleration
    );

    const Vec4State nextState = {
        state[0] + dt * (k1[0] + 2.0f * k2[0] + 2.0f * k3[0] + k4[0]) / 6.0f,
        state[1] + dt * (k1[1] + 2.0f * k2[1] + 2.0f * k3[1] + k4[1]) / 6.0f,
        state[2] + dt * (k1[2] + 2.0f * k2[2] + 2.0f * k3[2] + k4[2]) / 6.0f,
        state[3] + dt * (k1[3] + 2.0f * k2[3] + 2.0f * k3[3] + k4[3]) / 6.0f
    };

    pendulum.theta = UnpackTheta(nextState);
    pendulum.omega = UnpackOmega(nextState);
}

static Vec4State ComputeImplicitEulerResidual(
    const DoublePendulumState& pendulum,
    Vec2State theta0,
    Vec2State omega0,
    Vec4State x,
    float gravityAcceleration,
    float dt
)
{
    const Vec2State thetaNext = UnpackTheta(x);
    const Vec2State omegaNext = UnpackOmega(x);
    const Vec2State acceleration = Scale(Subtract(omegaNext, omega0), 1.0f / dt);

    float massMatrix[2][2];
    ComputeMassMatrix(pendulum, thetaNext, massMatrix);
    const Vec2State bias = ComputeBiasForces(
        pendulum,
        thetaNext,
        omegaNext,
        gravityAcceleration
    );

    return {
        thetaNext[0] - theta0[0] - dt * omegaNext[0],
        thetaNext[1] - theta0[1] - dt * omegaNext[1],
        massMatrix[0][0] * acceleration[0] + massMatrix[0][1] * acceleration[1] + bias[0],
        massMatrix[1][0] * acceleration[0] + massMatrix[1][1] * acceleration[1] + bias[1]
    };
}

static Vec4State ComputeImplicitMidpointResidual(
    const DoublePendulumState& pendulum,
    Vec2State theta0,
    Vec2State omega0,
    Vec4State x,
    float gravityAcceleration,
    float dt
)
{
    const Vec2State thetaNext = UnpackTheta(x);
    const Vec2State omegaNext = UnpackOmega(x);
    const Vec2State thetaMid = Scale(Add(theta0, thetaNext), 0.5f);
    const Vec2State omegaMid = Scale(Add(omega0, omegaNext), 0.5f);
    const Vec2State acceleration = Scale(Subtract(omegaNext, omega0), 1.0f / dt);

    float massMatrix[2][2];
    ComputeMassMatrix(pendulum, thetaMid, massMatrix);
    const Vec2State bias = ComputeBiasForces(
        pendulum,
        thetaMid,
        omegaMid,
        gravityAcceleration
    );

    return {
        thetaNext[0] - theta0[0] - dt * omegaMid[0],
        thetaNext[1] - theta0[1] - dt * omegaMid[1],
        massMatrix[0][0] * acceleration[0] + massMatrix[0][1] * acceleration[1] + bias[0],
        massMatrix[1][0] * acceleration[0] + massMatrix[1][1] * acceleration[1] + bias[1]
    };
}

static bool SolveLinearSystem4(float matrix[4][4], Vec4State rhs, Vec4State& solution)
{
    float augmented[4][5];
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            augmented[row][column] = matrix[row][column];
        }
        augmented[row][4] = rhs[row];
    }

    for (int pivotColumn = 0; pivotColumn < 4; ++pivotColumn) {
        int pivotRow = pivotColumn;
        for (int row = pivotColumn + 1; row < 4; ++row) {
            if (std::fabs(augmented[row][pivotColumn]) >
                std::fabs(augmented[pivotRow][pivotColumn])) {
                pivotRow = row;
            }
        }

        if (std::fabs(augmented[pivotRow][pivotColumn]) <= 0.000001f) {
            return false;
        }

        if (pivotRow != pivotColumn) {
            for (int column = pivotColumn; column < 5; ++column) {
                std::swap(augmented[pivotColumn][column], augmented[pivotRow][column]);
            }
        }

        const float pivot = augmented[pivotColumn][pivotColumn];
        for (int column = pivotColumn; column < 5; ++column) {
            augmented[pivotColumn][column] /= pivot;
        }

        for (int row = 0; row < 4; ++row) {
            if (row == pivotColumn) {
                continue;
            }

            const float factor = augmented[row][pivotColumn];
            for (int column = pivotColumn; column < 5; ++column) {
                augmented[row][column] -= factor * augmented[pivotColumn][column];
            }
        }
    }

    solution = { augmented[0][4], augmented[1][4], augmented[2][4], augmented[3][4] };
    return true;
}

template <typename ResidualFn>
static Vec4State SolveNewton(Vec4State initialGuess, int nonlinearIterations, ResidualFn computeResidual)
{
    Vec4State x = initialGuess;
    constexpr float finiteDifferenceStep = 0.0001f;

    for (int iteration = 0; iteration < nonlinearIterations; ++iteration) {
        const Vec4State residual = computeResidual(x);
        float jacobian[4][4] = {};

        for (int column = 0; column < 4; ++column) {
            Vec4State shifted = x;
            shifted[column] += finiteDifferenceStep;
            const Vec4State shiftedResidual = computeResidual(shifted);

            for (int row = 0; row < 4; ++row) {
                jacobian[row][column] =
                    (shiftedResidual[row] - residual[row]) / finiteDifferenceStep;
            }
        }

        Vec4State delta;
        if (!SolveLinearSystem4(
                jacobian,
                { -residual[0], -residual[1], -residual[2], -residual[3] },
                delta
            )) {
            break;
        }

        float maxDelta = 0.0f;
        for (int i = 0; i < 4; ++i) {
            x[i] += delta[i];
            maxDelta = std::max(maxDelta, std::fabs(delta[i]));
        }

        if (maxDelta <= 0.000001f) {
            break;
        }
    }

    return x;
}

static void StepImplicitEuler(
    DoublePendulumState& pendulum,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
)
{
    const Vec2State theta0 = pendulum.theta;
    const Vec2State omega0 = pendulum.omega;
    const Vec2State acceleration0 = ComputeDoublePendulumAcceleration(
        pendulum,
        theta0,
        omega0,
        gravityAcceleration
    );
    const Vec4State initialGuess = PackState(
        Add(theta0, Scale(omega0, dt)),
        Add(omega0, Scale(acceleration0, dt))
    );

    const Vec4State solution = SolveNewton(
        initialGuess,
        nonlinearIterations,
        [&](Vec4State x) {
            return ComputeImplicitEulerResidual(
                pendulum,
                theta0,
                omega0,
                x,
                gravityAcceleration,
                dt
            );
        }
    );

    pendulum.theta = UnpackTheta(solution);
    pendulum.omega = UnpackOmega(solution);
}

static void StepImplicitMidpoint(
    DoublePendulumState& pendulum,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
)
{
    const Vec2State theta0 = pendulum.theta;
    const Vec2State omega0 = pendulum.omega;
    const Vec2State acceleration0 = ComputeDoublePendulumAcceleration(
        pendulum,
        theta0,
        omega0,
        gravityAcceleration
    );
    const Vec4State initialGuess = PackState(
        Add(theta0, Scale(omega0, dt)),
        Add(omega0, Scale(acceleration0, dt))
    );

    const Vec4State solution = SolveNewton(
        initialGuess,
        nonlinearIterations,
        [&](Vec4State x) {
            return ComputeImplicitMidpointResidual(
                pendulum,
                theta0,
                omega0,
                x,
                gravityAcceleration,
                dt
            );
        }
    );

    pendulum.theta = UnpackTheta(solution);
    pendulum.omega = UnpackOmega(solution);
}

void StepDoublePendulum(
    DoublePendulumState& pendulum,
    IntegratorMode integratorMode,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
)
{
    switch (integratorMode) {
    case IntegratorMode::ExplicitEuler:
        StepExplicitEuler(pendulum, gravityAcceleration, dt);
        break;
    case IntegratorMode::SemiImplicitEuler:
        StepSemiImplicitEuler(pendulum, gravityAcceleration, dt);
        break;
    case IntegratorMode::Rk4:
        StepRk4(pendulum, gravityAcceleration, dt);
        break;
    case IntegratorMode::ImplicitEuler:
        StepImplicitEuler(pendulum, gravityAcceleration, dt, nonlinearIterations);
        break;
    case IntegratorMode::ImplicitMidpoint:
        StepImplicitMidpoint(pendulum, gravityAcceleration, dt, nonlinearIterations);
        break;
    }
}
