#include "ui.h"

#include "math_utils.h"

#include <algorithm>

UiLayout CreateUiLayout()
{
    return {
        { 20, 20, 120, 40 },
        { 155, 20, 120, 40 },
        { 20, 430, 300, 40 },
        { 20, 480, 300, 40 },
        { 250, 530, 36, 36 },
        { 290, 530, 36, 36 },
        { 250, 580, 36, 36 },
        { 290, 580, 36, 36 },
        { 20, 630, 150, 40 },
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
        ResetTree(app);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.bodyPresetButton)) {
        app.bodyPreset = GetNextBodyPreset(app.bodyPreset);
        ResetTree(app);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.integratorButton)) {
        app.integratorMode = GetNextIntegratorMode(app.integratorMode);
        ResetTree(app);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.decreaseAmplitudeButton)) {
        app.initialAmplitudeScale = ClampFloat(
            app.initialAmplitudeScale - INITIAL_AMPLITUDE_SCALE_STEP,
            MIN_INITIAL_AMPLITUDE_SCALE,
            MAX_INITIAL_AMPLITUDE_SCALE
        );
        ResetTree(app);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.increaseAmplitudeButton)) {
        app.initialAmplitudeScale = ClampFloat(
            app.initialAmplitudeScale + INITIAL_AMPLITUDE_SCALE_STEP,
            MIN_INITIAL_AMPLITUDE_SCALE,
            MAX_INITIAL_AMPLITUDE_SCALE
        );
        ResetTree(app);
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
    const Vector2 anchor = app.tree.anchor;
    const std::vector<Vector2> joints = GetTreeJointPositions(app.tree, app.tree.q);
    const std::vector<Vector2> endpoints = GetTreeLinkEndPositions(app.tree, app.tree.q);

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

    for (size_t link = 0; link < app.tree.links.size(); ++link) {
        DrawLineEx(joints[link], endpoints[link], 4.0f, DARKGRAY);
    }

    DrawCircleV(anchor, 8.0f, BLACK);
    DrawCircleLines(
        static_cast<int>(anchor.x),
        static_cast<int>(anchor.y),
        12.0f,
        GRAY
    );

    for (size_t link = 0; link < endpoints.size(); ++link) {
        DrawCircleV(endpoints[link], DEFAULT_BOB_RADIUS, link + 1 == endpoints.size() ? MAROON : DARKBLUE);
        DrawCircleLines(
            static_cast<int>(endpoints[link].x),
            static_cast<int>(endpoints[link].y),
            DEFAULT_BOB_RADIUS,
            BLACK
        );
    }
}

static void DrawMetrics(const AppState& app)
{
    DrawText(TextFormat("FPS: %d", GetFPS()), 20, 100, 20, BLACK);
    DrawText("Model: Lagrangian tree", 20, 130, 20, BLACK);
    DrawText(TextFormat("Body: %s", GetBodyPresetLabel(app.bodyPreset)), 20, 160, 20, BLACK);
    DrawText(TextFormat("Integrator: %s", GetIntegratorModeLabel(app.integratorMode)), 20, 190, 20, BLACK);
    DrawText(TextFormat(
        "DOF: %zu, links: %zu",
        app.tree.q.size(),
        app.tree.links.size()
    ), 20, 220, 20, BLACK);
    DrawText(TextFormat("Kinetic energy: %.2e", app.metrics.kineticEnergy), 20, 260, 20, BLACK);
    DrawText(TextFormat("Potential energy: %.2e", app.metrics.potentialEnergy), 20, 290, 20, BLACK);
    DrawText(TextFormat("Mechanical energy: %.2e", app.metrics.mechanicalEnergy), 20, 320, 20, BLACK);
    DrawText(TextFormat("Energy drift: %.2e", app.metrics.energyDrift), 20, 350, 20, BLACK);
    DrawText(TextFormat("Initial amplitude: %.1fx", app.initialAmplitudeScale), 20, 538, 20, BLACK);
    DrawText(TextFormat("Newton iterations: %d", app.nonlinearIterations), 20, 588, 20, BLACK);
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
        ui.bodyPresetButton,
        TextFormat("Body: %s", GetBodyPresetLabel(app.bodyPreset)),
        CheckCollisionPointRec(mouse, ui.bodyPresetButton)
    );
    DrawButton(
        ui.integratorButton,
        TextFormat("Mode: %s", GetIntegratorModeLabel(app.integratorMode)),
        CheckCollisionPointRec(mouse, ui.integratorButton)
    );
    DrawButton(ui.decreaseAmplitudeButton, "-", CheckCollisionPointRec(mouse, ui.decreaseAmplitudeButton));
    DrawButton(ui.increaseAmplitudeButton, "+", CheckCollisionPointRec(mouse, ui.increaseAmplitudeButton));
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
