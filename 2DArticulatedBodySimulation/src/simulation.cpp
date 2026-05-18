#include "simulation.h"

void InitializeAppState(AppState& app)
{
    ResetPendulum(app);
}

void ResetPendulum(AppState& app)
{
    app.endEffectorTrail.clear();
    app.pendulum = {
        { SCREEN_WIDTH * 0.5f, 150.0f },
        { DEFAULT_LINK_LENGTH, DEFAULT_LINK_LENGTH },
        { DEFAULT_LINK_MASS, DEFAULT_LINK_MASS },
        DEFAULT_INITIAL_THETA,
        { 0.0f, 0.0f }
    };

    InitializeMetrics(
        app.metrics,
        app.pendulum,
        GRAVITY_ACCELERATION
    );
}

static void UpdateTrail(AppState& app)
{
    app.endEffectorTrail.push_back(GetDoublePendulumBobPositions(app.pendulum)[1]);
    if (app.endEffectorTrail.size() > TRAIL_CAPACITY) {
        app.endEffectorTrail.erase(app.endEffectorTrail.begin());
    }
}

void TickSimulation(AppState& app, float dt)
{
    StepDoublePendulum(
        app.pendulum,
        app.integratorMode,
        GRAVITY_ACCELERATION,
        dt,
        app.nonlinearIterations
    );
    UpdateTrail(app);

    UpdateMetrics(
        app.metrics,
        app.pendulum,
        GRAVITY_ACCELERATION
    );
}
