#include "ui.h"

#include <algorithm>

UiLayout CreateUiLayout()
{
    return {
        { 20, 20, 120, 40 },
        { 155, 20, 120, 40 },
        { 20, 430, 300, 40 },
        { 250, 480, 36, 36 },
        { 290, 480, 36, 36 },
        { 20, 530, 150, 40 },
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

    if (IsKeyPressed(KEY_R) ||
        (leftClick && CheckCollisionPointRec(mouse, ui.resetButton))) {
        ResetPendulum(app);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.integratorButton)) {
        app.integratorMode = GetNextIntegratorMode(app.integratorMode);
        ResetPendulum(app);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.decreaseNewtonButton)) {
        app.nonlinearIterations = std::max(
            MIN_NONLINEAR_ITERATIONS,
            app.nonlinearIterations - 1
        );
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.increaseNewtonButton)) {
        app.nonlinearIterations = std::min(
            MAX_NONLINEAR_ITERATIONS,
            app.nonlinearIterations + 1
        );
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.trailButton)) {
        app.shouldDrawTrail = !app.shouldDrawTrail;
    }
}

static void DrawPendulum(const AppState& app)
{
    const Vector2 anchor = app.pendulum.anchor;
    const std::array<Vector2, 2> bobs = GetDoublePendulumBobPositions(app.pendulum);

    if (app.shouldDrawTrail && app.endEffectorTrail.size() > 1) {
        for (size_t i = 1; i < app.endEffectorTrail.size(); ++i) {
            const float alpha =
                static_cast<float>(i) / static_cast<float>(app.endEffectorTrail.size());
            DrawLineEx(
                app.endEffectorTrail[i - 1],
                app.endEffectorTrail[i],
                2.0f,
                ColorAlpha(SKYBLUE, 0.15f + 0.55f * alpha)
            );
        }
    }

    DrawLineEx(anchor, bobs[0], 4.0f, DARKGRAY);
    DrawLineEx(bobs[0], bobs[1], 4.0f, DARKGRAY);
    DrawCircleV(anchor, 8.0f, BLACK);
    DrawCircleLines(
        static_cast<int>(anchor.x),
        static_cast<int>(anchor.y),
        12.0f,
        GRAY
    );
    DrawCircleV(bobs[0], DEFAULT_BOB_RADIUS, DARKBLUE);
    DrawCircleLines(
        static_cast<int>(bobs[0].x),
        static_cast<int>(bobs[0].y),
        DEFAULT_BOB_RADIUS,
        BLACK
    );
    DrawCircleV(bobs[1], DEFAULT_BOB_RADIUS, MAROON);
    DrawCircleLines(
        static_cast<int>(bobs[1].x),
        static_cast<int>(bobs[1].y),
        DEFAULT_BOB_RADIUS,
        BLACK
    );
}

static void DrawMetrics(const AppState& app)
{
    DrawText(TextFormat("FPS: %d", GetFPS()), 20, 100, 20, BLACK);
    DrawText("Model: Lagrangian coordinates", 20, 130, 20, BLACK);
    DrawText(TextFormat("Integrator: %s", GetIntegratorModeLabel(app.integratorMode)), 20, 160, 20, BLACK);
    DrawText(TextFormat(
        "Theta: [%.3f, %.3f] rad",
        app.pendulum.theta[0],
        app.pendulum.theta[1]
    ), 20, 205, 20, BLACK);
    DrawText(TextFormat(
        "Omega: [%.3f, %.3f] rad/s",
        app.pendulum.omega[0],
        app.pendulum.omega[1]
    ), 20, 235, 20, BLACK);
    DrawText(TextFormat("Kinetic energy: %.2e", app.metrics.kineticEnergy), 20, 265, 20, BLACK);
    DrawText(TextFormat("Potential energy: %.2e", app.metrics.potentialEnergy), 20, 295, 20, BLACK);
    DrawText(TextFormat("Mechanical energy: %.2e", app.metrics.mechanicalEnergy), 20, 325, 20, BLACK);
    DrawText(TextFormat("Energy drift: %.2e", app.metrics.energyDrift), 20, 355, 20, BLACK);
    DrawText(TextFormat("Newton iterations: %d", app.nonlinearIterations), 20, 488, 20, BLACK);
}

void DrawApp(const AppState& app, const UiLayout& ui)
{
    const Vector2 mouse = GetMousePosition();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawPendulum(app);

    DrawButton(
        ui.pauseButton,
        app.isPaused ? "Resume" : "Pause",
        CheckCollisionPointRec(mouse, ui.pauseButton)
    );
    DrawButton(ui.resetButton, "Reset", CheckCollisionPointRec(mouse, ui.resetButton));
    DrawButton(
        ui.integratorButton,
        TextFormat("Mode: %s", GetIntegratorModeLabel(app.integratorMode)),
        CheckCollisionPointRec(mouse, ui.integratorButton)
    );
    DrawButton(ui.decreaseNewtonButton, "-", CheckCollisionPointRec(mouse, ui.decreaseNewtonButton));
    DrawButton(ui.increaseNewtonButton, "+", CheckCollisionPointRec(mouse, ui.increaseNewtonButton));
    DrawButton(
        ui.trailButton,
        app.shouldDrawTrail ? "Trail On" : "Trail Off",
        CheckCollisionPointRec(mouse, ui.trailButton)
    );

    DrawMetrics(app);

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
