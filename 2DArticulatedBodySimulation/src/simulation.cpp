#include "simulation.h"

const char* GetBodyPresetLabel(BodyPreset preset)
{
    switch (preset) {
    case BodyPreset::SimplePendulum:
        return "Simple Pendulum";
    case BodyPreset::DoublePendulum:
        return "Double Pendulum";
    case BodyPreset::Chain10:
        return "10-Link Chain";
    case BodyPreset::YPendulum:
        return "Y Pendulum";
    }

    return "Unknown";
}

BodyPreset GetNextBodyPreset(BodyPreset preset)
{
    switch (preset) {
    case BodyPreset::SimplePendulum:
        return BodyPreset::DoublePendulum;
    case BodyPreset::DoublePendulum:
        return BodyPreset::Chain10;
    case BodyPreset::Chain10:
        return BodyPreset::YPendulum;
    case BodyPreset::YPendulum:
        return BodyPreset::SimplePendulum;
    }

    return BodyPreset::SimplePendulum;
}

void InitializeAppState(AppState& app)
{
    ResetTree(app);
}

static std::vector<float> ScaleAngles(
    const std::vector<float>& angles,
    float amplitudeScale
)
{
    std::vector<float> scaledAngles = angles;
    for (float& angle : scaledAngles) {
        angle *= amplitudeScale;
    }

    return scaledAngles;
}

static ArticulatedTreeState CreateYPendulumTree(Vector2 anchor)
{
    ArticulatedTreeState tree;
    tree.anchor = anchor;
    tree.links = {
        { -1, 120.0f, DEFAULT_LINK_MASS },
        { 0, 95.0f, DEFAULT_LINK_MASS },
        { 0, 95.0f, DEFAULT_LINK_MASS },
        { 1, 70.0f, DEFAULT_LINK_MASS },
        { 2, 70.0f, DEFAULT_LINK_MASS }
    };
    tree.q = { 0.35f, 0.95f, -0.95f, 0.45f, -0.45f };
    tree.qdot.assign(tree.q.size(), 0.0f);
    return tree;
}

static ArticulatedTreeState CreatePresetTree(BodyPreset preset, float amplitudeScale)
{
    const Vector2 defaultAnchor = { SCREEN_WIDTH * 0.5f, 90.0f };

    switch (preset) {
    case BodyPreset::SimplePendulum:
        return CreateChainTree(
            { SCREEN_WIDTH * 0.5f, 150.0f },
            1,
            280.0f,
            DEFAULT_LINK_MASS,
            ScaleAngles({ 1.10f }, amplitudeScale)
        );
    case BodyPreset::DoublePendulum:
        return CreateChainTree(
            { SCREEN_WIDTH * 0.5f, 130.0f },
            2,
            180.0f,
            DEFAULT_LINK_MASS,
            ScaleAngles({ 1.10f, 0.85f }, amplitudeScale)
        );
    case BodyPreset::Chain10:
        return CreateChainTree(
            defaultAnchor,
            DEFAULT_LINK_COUNT,
            DEFAULT_LINK_LENGTH,
            DEFAULT_LINK_MASS,
            ScaleAngles(
                { 0.95f, 0.50f, -0.35f, 0.30f, -0.22f, 0.16f, -0.12f, 0.09f, -0.07f, 0.05f },
                amplitudeScale
            )
        );
    case BodyPreset::YPendulum:
    {
        ArticulatedTreeState tree = CreateYPendulumTree({ SCREEN_WIDTH * 0.5f, 130.0f });
        tree.q = ScaleAngles(tree.q, amplitudeScale);
        return tree;
    }
    }

    return CreateChainTree(
        defaultAnchor,
        1,
        DEFAULT_LINK_LENGTH,
        DEFAULT_LINK_MASS,
        ScaleAngles({ 0.85f }, amplitudeScale)
    );
}

void ResetTree(AppState& app)
{
    app.endEffectorTrail.clear();
    app.tree = CreatePresetTree(app.bodyPreset, app.initialAmplitudeScale);

    InitializeMetrics(
        app.metrics,
        app.tree,
        GRAVITY_ACCELERATION
    );
}

static void UpdateTrail(AppState& app)
{
    const std::vector<Vector2> endpoints = GetTreeLinkEndPositions(app.tree, app.tree.q);
    if (!endpoints.empty()) {
        app.endEffectorTrail.push_back(endpoints.back());
    }

    if (app.endEffectorTrail.size() > TRAIL_CAPACITY) {
        app.endEffectorTrail.erase(app.endEffectorTrail.begin());
    }
}

void TickSimulation(AppState& app, float dt)
{
    StepArticulatedTree(
        app.tree,
        app.integratorMode,
        GRAVITY_ACCELERATION,
        dt,
        app.nonlinearIterations
    );
    UpdateTrail(app);

    UpdateMetrics(
        app.metrics,
        app.tree,
        GRAVITY_ACCELERATION
    );
}
