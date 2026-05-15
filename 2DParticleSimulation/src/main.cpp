#include "raylib.h"
#include "simulation.h"
#include "ui.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Particle Simulation");
    SetTargetFPS(60);

    Texture2D particleSprite = CreateParticleSpriteTexture();
    const UiLayout ui = CreateUiLayout();
    AppState app;
    InitializeAppState(app);

    float accumulator = 0.0f;

    while (!WindowShouldClose()) {
        accumulator += GetFrameTime();

        HandleInput(app, ui);

        while (accumulator >= FIXED_SIMULATION_TIMESTEP) {
            if (!app.isPaused) {
                TickSimulation(app, FIXED_SIMULATION_TIMESTEP);
            }

            accumulator -= FIXED_SIMULATION_TIMESTEP;
        }

        DrawApp(app, ui, particleSprite);
    }

    UnloadTexture(particleSprite);
    CloseWindow();
    return 0;
}
