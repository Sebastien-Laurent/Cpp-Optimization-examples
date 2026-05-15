#pragma once

#include "collision.h"
#include "metrics.h"
#include "particle.h"

#include <vector>

constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 900;
constexpr int INITIAL_PARTICLE_COUNT = 100;
constexpr int HIGHLIGHTED_PARTICLE_COUNT = 100;
constexpr int PARTICLE_COUNT_STEP = 100;
constexpr float PARTICLE_RADIUS = 2.0f;
constexpr float PARTICLE_INITIAL_SPEED = 200.0f;
constexpr float SPEED_STEP = 5.0f;
constexpr float MIN_SPEED = 0.0f;
constexpr float MAX_SPEED = 1000.0f;
constexpr float FIXED_SIMULATION_TIMESTEP = 1.0f / 60.0f;
constexpr float GRAVITY_ACCELERATION = 500.0f;

struct AppState {
    std::vector<Particle> particles;
    float speed = PARTICLE_INITIAL_SPEED;
    bool isPaused = false;
    bool isGravityEnabled = false;
    bool isParticleCollisionEnabled = true;
    CollisionMode collisionMode = CollisionMode::BruteForce;
    MetricsState metrics;
};

void InitializeAppState(AppState& app);
void TickSimulation(AppState& app, float dt);

float ClampFloat(float value, float minValue, float maxValue);

void ApplyGravity(std::vector<Particle>& particles, float dt);
void IntegrateParticles(std::vector<Particle>& particles, float dt, bool isGravityEnabled);
void ResolveWallCollisions(std::vector<Particle>& particles, bool gravityOn);
