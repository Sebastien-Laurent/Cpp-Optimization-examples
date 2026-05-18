#include "raylib.h"
#include "simulation.h"
#include "ui.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Articulated Body Simulation");
    SetTargetFPS(60);

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

        DrawApp(app, ui);
    }

    CloseWindow();
    return 0;
}
