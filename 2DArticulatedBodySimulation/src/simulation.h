#pragma once

#include "integrators.h"
#include "metrics.h"

#include <vector>

constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 900;
constexpr float FIXED_SIMULATION_TIMESTEP = 1.0f / 240.0f;
constexpr float GRAVITY_ACCELERATION = 700.0f;
constexpr int DEFAULT_LINK_COUNT = 10;
constexpr float DEFAULT_LINK_LENGTH = 55.0f;
constexpr float DEFAULT_LINK_MASS = 1.0f;
constexpr float DEFAULT_BOB_RADIUS = 8.0f;
constexpr int DEFAULT_NONLINEAR_ITERATIONS = 4;
constexpr int MIN_NONLINEAR_ITERATIONS = 1;
constexpr int MAX_NONLINEAR_ITERATIONS = 32;
constexpr float DEFAULT_INITIAL_AMPLITUDE_SCALE = 1.0f;
constexpr float MIN_INITIAL_AMPLITUDE_SCALE = 0.1f;
constexpr float MAX_INITIAL_AMPLITUDE_SCALE = 2.0f;
constexpr float INITIAL_AMPLITUDE_SCALE_STEP = 0.1f;
constexpr int TRAIL_CAPACITY = 420;

enum class BodyPreset {
    SimplePendulum,
    DoublePendulum,
    Chain10,
    YPendulum
};

struct AppState {
    ArticulatedTreeState tree;
    std::vector<Vector2> endEffectorTrail;
    BodyPreset bodyPreset = BodyPreset::Chain10;
    IntegratorMode integratorMode = IntegratorMode::Rk4;
    MetricsState metrics;
    int nonlinearIterations = DEFAULT_NONLINEAR_ITERATIONS;
    float initialAmplitudeScale = DEFAULT_INITIAL_AMPLITUDE_SCALE;
    bool isPaused = false;
    bool shouldDrawTrail = true;
};

const char* GetBodyPresetLabel(BodyPreset preset);
BodyPreset GetNextBodyPreset(BodyPreset preset);
void InitializeAppState(AppState& app);
void ResetTree(AppState& app);
void TickSimulation(AppState& app, float dt);
