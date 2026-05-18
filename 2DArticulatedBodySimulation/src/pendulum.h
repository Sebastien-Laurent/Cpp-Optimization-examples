#pragma once

#include "raylib.h"

#include <array>

enum class IntegratorMode {
    ExplicitEuler,
    SemiImplicitEuler,
    Rk4,
    ImplicitEuler,
    ImplicitMidpoint
};

using Vec2State = std::array<float, 2>;
using Vec4State = std::array<float, 4>;

struct DoublePendulumState {
    Vector2 anchor = { 0.0f, 0.0f };
    Vec2State length = { 1.0f, 1.0f };
    Vec2State mass = { 1.0f, 1.0f };
    Vec2State theta = { 0.0f, 0.0f };
    Vec2State omega = { 0.0f, 0.0f };
};

const char* GetIntegratorModeLabel(IntegratorMode mode);
IntegratorMode GetNextIntegratorMode(IntegratorMode mode);

Vec2State ComputeDoublePendulumAcceleration(
    const DoublePendulumState& pendulum,
    const Vec2State& theta,
    const Vec2State& omega,
    float gravityAcceleration
);
std::array<Vector2, 2> GetDoublePendulumBobPositions(const DoublePendulumState& pendulum);
std::array<Vector2, 2> GetDoublePendulumBobVelocities(const DoublePendulumState& pendulum);
void StepDoublePendulum(
    DoublePendulumState& pendulum,
    IntegratorMode integratorMode,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
);
