// Build and run:
//   mkdir -p build
//   c++ -O2 raylibtest.cpp -o build/raylibtest $(pkg-config --cflags --libs raylib)
//   ./build/raylibtest

#include "raylib.h"

#include <cmath>
#include <random>
#include <vector>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
constexpr float PARTICLE_RADIUS = 10.0f;
constexpr float PARTICLE_INITIAL_SPEED = 200.0f;
constexpr float SPEED_STEP = 5.0f;
constexpr float MIN_SPEED = 0.0f;
constexpr float MAX_SPEED = 1000.0f;
constexpr float FIXED_SIMULATION_TIMESTEP = 1.0f / 60.0f;

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool isColliding = false;
};

struct AppState {
    std::vector<Particle> particles;
    float speed = PARTICLE_INITIAL_SPEED;
    bool isPaused = false;
};

struct UiLayout {
    Rectangle pauseButton;
    Rectangle decreaseSpeedButton;
    Rectangle increaseSpeedButton;
    Rectangle decreaseParticleButton;
    Rectangle increaseParticleButton;
};

UiLayout CreateUiLayout();
Particle CreateRandomParticle(float speed);
Vector2 RandomVelocity(float speed);
Vector2 RandomPosition(float radius);
float ClampFloat(float value, float minValue, float maxValue);
float ComputeTotalKineticEnergy(const std::vector<Particle>& particles);
void HandleInput(AppState& app, const UiLayout& ui);
void DrawApp(const AppState& app, const UiLayout& ui);
void DrawButton(Rectangle button, const char* label, bool isHovered);

void UpdateParticles(std::vector<Particle>& particles, float dt);
void CheckParticleCollisions(std::vector<Particle> &particles);
void ResolveCollision(Particle &pA, Particle &pB);

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib test");
    SetTargetFPS(60);

    const UiLayout ui = CreateUiLayout();
    AppState app;
    app.particles.push_back(CreateRandomParticle(app.speed));

    float accumulator = 0.0f;

    while (!WindowShouldClose()) {

        accumulator += GetFrameTime();

        HandleInput(app, ui);

        while(accumulator >= FIXED_SIMULATION_TIMESTEP){

        if (!app.isPaused) {
            UpdateParticles(app.particles, FIXED_SIMULATION_TIMESTEP);
            CheckParticleCollisions(app.particles);
        }
        accumulator -= FIXED_SIMULATION_TIMESTEP;
        }

        DrawApp(app, ui);
    }

    CloseWindow();
    return 0;
}

UiLayout CreateUiLayout()
{
    return {
        { 20, 20, 120, 40 },
        { 20, 70, 40, 40 },
        { 100, 70, 40, 40 },
        { 20, 170, 40, 40 },
        { 100, 170, 40, 40 },
    };
}

Particle CreateRandomParticle(float speed)
{
    return {
        RandomPosition(PARTICLE_RADIUS),
        RandomVelocity(speed),
        PARTICLE_RADIUS,
    };
}

void HandleInput(AppState& app, const UiLayout& ui)
{
    const Vector2 mouse = GetMousePosition();
    const bool leftClick = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (IsKeyPressed(KEY_SPACE) ||
        (leftClick && CheckCollisionPointRec(mouse, ui.pauseButton))) {
        app.isPaused = !app.isPaused;
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.decreaseSpeedButton)) {
        app.speed = ClampFloat(app.speed - SPEED_STEP, MIN_SPEED, MAX_SPEED);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.increaseSpeedButton)) {
        app.speed = ClampFloat(app.speed + SPEED_STEP, MIN_SPEED, MAX_SPEED);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.increaseParticleButton)) {
        app.particles.push_back(CreateRandomParticle(app.speed));
    }

    if (leftClick &&
        CheckCollisionPointRec(mouse, ui.decreaseParticleButton) &&
        app.particles.size() > 1) {
        app.particles.pop_back();
    }
}

void UpdateParticles(std::vector<Particle>& particles, float dt)
{
    for (Particle& particle : particles) {
        particle.position.x += dt * particle.velocity.x;
        particle.position.y += dt * particle.velocity.y;

        if (particle.position.x > SCREEN_WIDTH - particle.radius ||
            particle.position.x < particle.radius) {
            particle.velocity.x = -particle.velocity.x;
            particle.position.x = ClampFloat(
                particle.position.x,
                particle.radius,
                SCREEN_WIDTH - particle.radius
            );
        }

        if (particle.position.y > SCREEN_HEIGHT - particle.radius ||
            particle.position.y < particle.radius) {
            particle.velocity.y = -particle.velocity.y;
            particle.position.y = ClampFloat(
                particle.position.y,
                particle.radius,
                SCREEN_HEIGHT - particle.radius
            );
        }
    }
}

void CheckParticleCollisions(std::vector<Particle> &particles)
{
    for(Particle& particle: particles){
        particle.isColliding = false;
    }

    for (size_t i = 0; i < particles.size();i++){
        for (size_t j = i+1; j < particles.size(); j++){
            ResolveCollision(particles[i], particles[j]);
        }
    }
}
void ResolveCollision(Particle &pA, Particle &pB){

    Vector2 delta = {
        pB.position.x - pA.position.x,
        pB.position.y - pA.position.y};

    float distanceSquared = delta.x * delta.x + delta.y * delta.y;
    float radiusSum = pA.radius + pB.radius;

    if(distanceSquared > radiusSum *radiusSum){
        return;
    }

    pA.isColliding = true;
    pB.isColliding = true;

    float distance = std::sqrt(distanceSquared);

    //To avoid dividing by zero, but not very realistic (Maybe improve later)
    if(distance == 0.0f){
        return;
    }

    Vector2 normal = {delta.x / distance, delta.y / distance};

    // Push particles apart so they do not remain overlapping.
    float overlap = radiusSum - distance;
    pA.position.x -= normal.x * overlap * 0.5f;
    pA.position.y -= normal.y * overlap * 0.5f;
    pB.position.x += normal.x * overlap * 0.5f;
    pB.position.y += normal.y * overlap * 0.5f;

    Vector2 relativeVelocity = {
        pA.velocity.x - pB.velocity.x,
        pA.velocity.y - pB.velocity.y};

    float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

    //case where particles are already moving apart (but overlaping), can happen at high velocity -> tunneling
    if(velocityAlongNormal <= 0.0f){
        return;
    }

    Vector2 impulse = {
        velocityAlongNormal * normal.x,
        velocityAlongNormal * normal.y};

    pA.velocity.x -= impulse.x;
    pA.velocity.y -= impulse.y;

    pB.velocity.x += impulse.x;
    pB.velocity.y += impulse.y;

}

float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) {
        return minValue;
    }

    if (value > maxValue) {
        return maxValue;
    }

    return value;
}

float ComputeTotalKineticEnergy(const std::vector<Particle>& particles)
{
    constexpr float particleMass = 1.0f;
    float totalKineticEnergy = 0.0f;

    for (const Particle& particle : particles) {
        const float speedSquared =
            particle.velocity.x * particle.velocity.x +
            particle.velocity.y * particle.velocity.y;

        totalKineticEnergy += 0.5f * particleMass * speedSquared;
    }

    return totalKineticEnergy;
}

Vector2 RandomVelocity(float speed)
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * PI);

    float angle = angle_dist(rng);
    return {
        speed * std::cos(angle),
        speed * std::sin(angle),
    };
}

Vector2 RandomPosition(float radius)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> pos_height_dist(radius, SCREEN_HEIGHT - radius);
    std::uniform_real_distribution<float> pos_width_dist(radius, SCREEN_WIDTH - radius);
    float height = pos_height_dist(rng);
    float width = pos_width_dist(rng);

    return {
        width,
        height,
    };
}

void DrawApp(const AppState& app, const UiLayout& ui)
{
    const Vector2 mouse = GetMousePosition();

    const bool mouseOverPauseButton = CheckCollisionPointRec(mouse, ui.pauseButton);
    const bool mouseOverDecreaseSpeedButton = CheckCollisionPointRec(mouse, ui.decreaseSpeedButton);
    const bool mouseOverIncreaseSpeedButton = CheckCollisionPointRec(mouse, ui.increaseSpeedButton);
    const bool mouseOverDecreaseParticleButton = CheckCollisionPointRec(mouse, ui.decreaseParticleButton);
    const bool mouseOverIncreaseParticleButton = CheckCollisionPointRec(mouse, ui.increaseParticleButton);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (const Particle& particle : app.particles) {
        DrawCircle(
            particle.position.x,
            particle.position.y,
            particle.radius,
            particle.isColliding ? BLUE : RED
        );
    }

    DrawButton(ui.pauseButton, app.isPaused ? "Resume" : "Pause", mouseOverPauseButton);
    DrawButton(ui.decreaseSpeedButton, "-", mouseOverDecreaseSpeedButton);
    DrawButton(ui.increaseSpeedButton, "+", mouseOverIncreaseSpeedButton);
    DrawText(TextFormat("Speed: %.0f", app.speed), 20, 120, 20, BLACK);
    DrawText(TextFormat("FPS: %d", GetFPS()), 20, 145, 20, BLACK);

    DrawButton(ui.decreaseParticleButton, "-", mouseOverDecreaseParticleButton);
    DrawButton(ui.increaseParticleButton, "+", mouseOverIncreaseParticleButton);
    DrawText(TextFormat("Particles: %zu", app.particles.size()), 20, 220, 20, BLACK);
    DrawText(TextFormat("Kinetic energy: %.0f", ComputeTotalKineticEnergy(app.particles)), 20, 245, 20, BLACK);

    EndDrawing();
}

void DrawButton(Rectangle button, const char* label, bool isHovered)
{
    const int fontSize = 20;
    const int textWidth = MeasureText(label, fontSize);
    const int textX = static_cast<int>(button.x + (button.width - textWidth) / 2.0f);
    const int textY = static_cast<int>(button.y + (button.height - fontSize) / 2.0f);

    DrawRectangleRec(button, isHovered ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(button, 2, DARKGRAY);
    DrawText(label, textX, textY, fontSize, BLACK);
}
