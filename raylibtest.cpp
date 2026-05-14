// Build and run:
//   mkdir -p build
//   c++ -O2 raylibtest.cpp -o build/raylibtest $(pkg-config --cflags --libs raylib)
//   ./build/raylibtest

#include "raylib.h"
#include <cmath>
#include <random>

Vector2 Random_direction();

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "raylib test");
    SetTargetFPS(60);

    Vector2 particlePosition = {10, (float)screenHeight / 2.0f};
    Vector2 particleSpeedDirection = Random_direction();

    const float radius = 10.0f;
    const float speedReference = 50.0f;
    bool isPaused = false;

    while (!WindowShouldClose()) {

        if(!isPaused)
        {
        particlePosition.x += GetFrameTime() * speedReference * particleSpeedDirection.x;
        particlePosition.y += GetFrameTime() * speedReference * particleSpeedDirection.y;
    }

        if(particlePosition.x > screenWidth-radius || particlePosition.x < radius)
        {
            particleSpeedDirection.x = -particleSpeedDirection.x;
        }

        if(particlePosition.y > screenHeight-radius || particlePosition.y < radius)
        {
            particleSpeedDirection.y = -particleSpeedDirection.y;
        }

        if(IsKeyPressed(KEY_SPACE))
        {
            isPaused = !isPaused;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircle(particlePosition.x, particlePosition.y, radius, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
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
