#pragma once

#include "articulated_tree.h"

enum class IntegratorMode {
    ExplicitEuler,
    SemiImplicitEuler,
    Rk4,
    ImplicitEuler,
    ImplicitMidpoint
};

const char* GetIntegratorModeLabel(IntegratorMode mode);
IntegratorMode GetNextIntegratorMode(IntegratorMode mode);
void StepArticulatedTree(
    ArticulatedTreeState& tree,
    IntegratorMode integratorMode,
    float gravityAcceleration,
    float dt,
    int nonlinearIterations
);
