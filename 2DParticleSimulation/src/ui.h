#pragma once

#include "raylib.h"
#include "simulation.h"

struct UiLayout {
    Rectangle pauseButton;
    Rectangle decreaseSpeedButton;
    Rectangle increaseSpeedButton;
    Rectangle decreaseParticleButton;
    Rectangle increaseParticleButton;
    Rectangle gravityButton;
    Rectangle collisionButton;
    Rectangle drawParticlesButton;
    Rectangle collisionModeButton;
};

UiLayout CreateUiLayout();
Texture2D CreateParticleSpriteTexture();
void HandleInput(AppState& app, const UiLayout& ui);
void DrawApp(const AppState& app, const UiLayout& ui, Texture2D particleSprite);
void DrawButton(Rectangle button, const char* label, bool isHovered);
