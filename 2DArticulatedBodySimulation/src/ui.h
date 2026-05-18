#pragma once

#include "raylib.h"
#include "simulation.h"

struct UiLayout {
    Rectangle pauseButton;
    Rectangle resetButton;
    Rectangle bodyPresetButton;
    Rectangle integratorButton;
    Rectangle decreaseAmplitudeButton;
    Rectangle increaseAmplitudeButton;
    Rectangle decreaseNewtonButton;
    Rectangle increaseNewtonButton;
    Rectangle trailButton;
};

UiLayout CreateUiLayout();
void HandleInput(AppState& app, const UiLayout& ui);
void DrawApp(const AppState& app, const UiLayout& ui);
void DrawButton(Rectangle button, const char* label, bool isHovered);
