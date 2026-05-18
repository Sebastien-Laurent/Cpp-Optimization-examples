#pragma once

#include "metrics.h"
#include "pendulum.h"

#include <vector>

constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 900;
constexpr float FIXED_SIMULATION_TIMESTEP = 1.0f / 120.0f;
constexpr float GRAVITY_ACCELERATION = 700.0f;
constexpr float DEFAULT_LINK_LENGTH = 180.0f;
constexpr float DEFAULT_LINK_MASS = 1.0f;
constexpr float DEFAULT_BOB_RADIUS = 16.0f;
constexpr Vec2State DEFAULT_INITIAL_THETA = { 0.85f, 1.45f };
constexpr int DEFAULT_NONLINEAR_ITERATIONS = 8;
constexpr int MIN_NONLINEAR_ITERATIONS = 1;
constexpr int MAX_NONLINEAR_ITERATIONS = 32;
constexpr int TRAIL_CAPACITY = 420;

struct AppState {
    DoublePendulumState pendulum;
    std::vector<Vector2> endEffectorTrail;
    IntegratorMode integratorMode = IntegratorMode::ImplicitMidpoint;
    MetricsState metrics;
    int nonlinearIterations = DEFAULT_NONLINEAR_ITERATIONS;
    bool isPaused = false;
    bool shouldDrawTrail = true;
};

void InitializeAppState(AppState& app);
void ResetPendulum(AppState& app);
void TickSimulation(AppState& app, float dt);
