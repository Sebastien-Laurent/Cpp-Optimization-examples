#include "ui.h"

#include "collision.h"
#include "particle.h"

#include <algorithm>

constexpr int PARTICLE_SPRITE_SIZE = 32;

UiLayout CreateUiLayout()
{
    return {
        { 20, 20, 120, 40 },
        { 20, 70, 40, 40 },
        { 100, 70, 40, 40 },
        { 20, 170, 40, 40 },
        { 100, 170, 40, 40 },
        { 20, 350, 150, 40 },
        { 20, 400, 150, 40 },
        { 20, 450, 150, 40 },
        { 20, 500, 260, 40 },
    };
}

Texture2D CreateParticleSpriteTexture()
{
    Image image = GenImageColor(PARTICLE_SPRITE_SIZE, PARTICLE_SPRITE_SIZE, BLANK);
    ImageDrawCircle(
        &image,
        PARTICLE_SPRITE_SIZE / 2,
        PARTICLE_SPRITE_SIZE / 2,
        PARTICLE_SPRITE_SIZE / 2,
        WHITE
    );

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
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
        for (int i = 0; i < PARTICLE_COUNT_STEP; ++i) {
            app.particles.push_back(CreateRandomParticle(app.speed));
        }
    }

    if (leftClick &&
        CheckCollisionPointRec(mouse, ui.decreaseParticleButton) &&
        app.particles.size() > 1) {
        const size_t removableParticleCount = app.particles.size() - 1;
        const size_t particlesToRemove = std::min(
            removableParticleCount,
            static_cast<size_t>(PARTICLE_COUNT_STEP)
        );

        app.particles.resize(app.particles.size() - particlesToRemove);
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.gravityButton)) {
        app.isGravityEnabled = !app.isGravityEnabled;
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.collisionButton)) {
        app.isParticleCollisionEnabled = !app.isParticleCollisionEnabled;
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.drawParticlesButton)) {
        app.shouldDrawParticles = !app.shouldDrawParticles;
    }

    if (leftClick && CheckCollisionPointRec(mouse, ui.collisionModeButton)) {
        app.collisionMode = GetNextCollisionMode(app.collisionMode);
    }
}

void DrawApp(const AppState& app, const UiLayout& ui, Texture2D particleSprite)
{
    const Vector2 mouse = GetMousePosition();

    const bool mouseOverPauseButton = CheckCollisionPointRec(mouse, ui.pauseButton);
    const bool mouseOverDecreaseSpeedButton = CheckCollisionPointRec(mouse, ui.decreaseSpeedButton);
    const bool mouseOverIncreaseSpeedButton = CheckCollisionPointRec(mouse, ui.increaseSpeedButton);
    const bool mouseOverDecreaseParticleButton = CheckCollisionPointRec(mouse, ui.decreaseParticleButton);
    const bool mouseOverIncreaseParticleButton = CheckCollisionPointRec(mouse, ui.increaseParticleButton);
    const bool mouseOverGravityButton = CheckCollisionPointRec(mouse, ui.gravityButton);
    const bool mouseOverCollisionButton = CheckCollisionPointRec(mouse, ui.collisionButton);
    const bool mouseOverDrawParticlesButton = CheckCollisionPointRec(mouse, ui.drawParticlesButton);
    const bool mouseOverCollisionModeButton = CheckCollisionPointRec(mouse, ui.collisionModeButton);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (app.shouldDrawParticles) {
        for (size_t i = 0; i < app.particles.size(); ++i) {
            const Particle& particle = app.particles[i];
            const Color particleColor =
                (app.isParticleCollisionEnabled && particle.isColliding) ?
                    BLUE :
                    (i < HIGHLIGHTED_PARTICLE_COUNT ? GREEN : RED);

            DrawTexturePro(
                particleSprite,
                {
                    0.0f,
                    0.0f,
                    static_cast<float>(particleSprite.width),
                    static_cast<float>(particleSprite.height)
                },
                {
                    particle.position.x,
                    particle.position.y,
                    particle.radius * 2.0f,
                    particle.radius * 2.0f
                },
                {
                    particle.radius,
                    particle.radius
                },
                0.0f,
                particleColor
            );
        }
    }

    DrawButton(ui.pauseButton, app.isPaused ? "Resume" : "Pause", mouseOverPauseButton);
    DrawButton(ui.decreaseSpeedButton, "-", mouseOverDecreaseSpeedButton);
    DrawButton(ui.increaseSpeedButton, "+", mouseOverIncreaseSpeedButton);
    DrawText(TextFormat("Speed: %.0f", app.speed), 20, 120, 20, BLACK);
    DrawText(TextFormat("FPS: %d", GetFPS()), 20, 145, 20, BLACK);

    DrawButton(ui.decreaseParticleButton, "-", mouseOverDecreaseParticleButton);
    DrawButton(ui.increaseParticleButton, "+", mouseOverIncreaseParticleButton);
    DrawText(TextFormat("Particles: %zu", app.particles.size()), 20, 220, 20, BLACK);
    DrawText(TextFormat("Kinetic energy: %.2e", app.metrics.kineticEnergy), 20, 245, 20, BLACK);
    DrawText(TextFormat("Mechanical energy: %.2e", app.metrics.mechanicalEnergy), 20, 270, 20, BLACK);
    DrawText(TextFormat("Collisions/s: %.2e", app.metrics.collisionRate), 20, 295, 20, BLACK);
    DrawText(TextFormat("Collision checks/s: %.2e", app.metrics.collisionCandidateCheckRate), 20, 320, 20, BLACK);
    DrawButton(ui.gravityButton, app.isGravityEnabled ? "Gravity On" : "Gravity Off", mouseOverGravityButton);
    DrawButton(
        ui.collisionButton,
        app.isParticleCollisionEnabled ? "Collisions On" : "Collisions Off",
        mouseOverCollisionButton
    );
    DrawButton(
        ui.drawParticlesButton,
        app.shouldDrawParticles ? "Draw On" : "Draw Off",
        mouseOverDrawParticlesButton
    );
    DrawButton(
        ui.collisionModeButton,
        TextFormat("Mode: %s", GetCollisionModeLabel(app.collisionMode)),
        mouseOverCollisionModeButton
    );

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
