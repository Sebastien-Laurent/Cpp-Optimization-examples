// Build and run:
//   mkdir -p build
//   c++ -O2 raylibtest.cpp -o build/raylibtest $(pkg-config --cflags --libs raylib)
//   ./build/raylibtest

#include "raylib.h"
#include <cmath>
#include <random>

struct Particle{
    Vector2 position;
    Vector2 direction;
    float radius;
};

Vector2 Random_direction();
Vector2 Random_position();
void UpdateParticles(std::vector<Particle> &particles, float &speed);

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

int main()
{
    //UI

    const Rectangle pauseButton = { 20, 20, 120, 40 };
    const Rectangle decreaseSpeedButton = { 20, 70, 40, 40 };
    const Rectangle increaseSpeedButton = { 100, 70, 40, 40 };
    const Rectangle decreaseParticleButton = { 20, 170, 40, 40 };
    const Rectangle increaseParticleButton = { 100, 170, 40, 40 };


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib test");
    SetTargetFPS(60);

    //Particles

    const float radius = 10.0f;

    std::vector<Particle> particles;
    particles.push_back({Random_position(),Random_direction(),radius});

    //Vector2 particlePosition = {10, (float)screenHeight / 2.0f};
    //Vector2 particleSpeedDirection = Random_direction();

    float speedReference = 200.0f;
    bool isPaused = false;

    while (!WindowShouldClose()) {
        
        if (!isPaused)
        {
            UpdateParticles(particles,speedReference);
        }

        Vector2 mouse = GetMousePosition();
        bool mouseOverPauseButton = CheckCollisionPointRec(mouse, pauseButton);
        bool mouseOverDecreaseSpeedButton = CheckCollisionPointRec(mouse, decreaseSpeedButton);
        bool mouseOverIncreaseSpeedButton = CheckCollisionPointRec(mouse, increaseSpeedButton);
        bool mouseOverDecreaseParticleButton = CheckCollisionPointRec(mouse, decreaseParticleButton);
        bool mouseOverIncreaseParticleButton =CheckCollisionPointRec(mouse, increaseParticleButton);

        if (mouseOverPauseButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isPaused = !isPaused;
        }

        if (mouseOverDecreaseSpeedButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            speedReference -= 5.0f;
            if (speedReference < 0.0f) speedReference = 0.0f;
        }

        if (mouseOverIncreaseSpeedButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            speedReference += 5.0f;
            if (speedReference > 1000.0f) speedReference = 1000.0f;
        }

        if (IsKeyPressed(KEY_SPACE)) {
            isPaused = !isPaused;
        }

        if (mouseOverIncreaseParticleButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            particles.push_back({Random_position(),
                                 Random_direction(),
                                 radius});
        }

        if (mouseOverDecreaseParticleButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && particles.size() > 1) {
            particles.pop_back();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (const Particle& particle : particles) {
            DrawCircle(
                particle.position.x,
                particle.position.y,
                particle.radius,
                RED
            );
        } 
        DrawRectangleRec(pauseButton, mouseOverPauseButton ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(pauseButton, 2, DARKGRAY);
        DrawText(isPaused ? "Resume" : "Pause", 45, 32, 20, BLACK);

        DrawRectangleRec(decreaseSpeedButton, mouseOverDecreaseSpeedButton ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(decreaseSpeedButton, 2, DARKGRAY);
        DrawText("-", 34, 79, 24, BLACK);

        DrawRectangleRec(increaseSpeedButton, mouseOverIncreaseSpeedButton ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(increaseSpeedButton, 2, DARKGRAY);
        DrawText("+", 113, 79, 24, BLACK);

        DrawText(TextFormat("Speed: %.0f", speedReference), 20, 120, 20, BLACK);
        DrawText(TextFormat("FPS: %d", GetFPS()), 20, 145, 20, BLACK);

        DrawRectangleRec(decreaseParticleButton, mouseOverDecreaseParticleButton ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(decreaseParticleButton, 2, DARKGRAY);
        DrawText("-", 34, 179, 24, BLACK);

        DrawRectangleRec(increaseParticleButton, mouseOverIncreaseParticleButton ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(increaseParticleButton, 2, DARKGRAY);
        DrawText("+", 113, 179, 24, BLACK);

        DrawText(TextFormat("Particles: %zu", particles.size()), 20, 220, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}


void UpdateParticles(std::vector<Particle> &particles, float &speed)
{
    for(Particle& particle : particles)
    {
        particle.position.x += GetFrameTime() * speed * particle.direction.x;
        particle.position.y += GetFrameTime() * speed * particle.direction.y;
    

    //should modify this to prevent particle to go outside and modify position to include the bouncing 
    if (particle.position.x > SCREEN_WIDTH - particle.radius || particle.position.x < particle.radius) {
        particle.direction.x = -particle.direction.x;
    }

    if (particle.position.y > SCREEN_HEIGHT - particle.radius || particle.position.y < particle.radius) {
        particle.direction.y = -particle.direction.y;
    }
}

}

Vector2 Random_direction()
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * PI);

    float angle = angle_dist(rng);
    return {
        std::cos(angle),
        std::sin(angle),
    };
}

Vector2 Random_position()
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> pos_height_dist(0, SCREEN_HEIGHT);
    static std::uniform_real_distribution<float> pos_width_dist(0, SCREEN_WIDTH);
    float height = pos_height_dist(rng);
    float width = pos_width_dist(rng);

    return {
        width,
        height,
    };
}
