#pragma once

#include <vector>

using DenseMatrix = std::vector<std::vector<float>>;

std::vector<float> AddVectors(const std::vector<float>& a, const std::vector<float>& b);
std::vector<float> SubtractVectors(const std::vector<float>& a, const std::vector<float>& b);
std::vector<float> ScaleVector(const std::vector<float>& value, float scale);
std::vector<float> PackState(
    const std::vector<float>& q,
    const std::vector<float>& qdot
);
std::vector<float> UnpackQ(const std::vector<float>& state);
std::vector<float> UnpackQdot(const std::vector<float>& state);
bool SolveLinearSystem(
    DenseMatrix matrix,
    std::vector<float> rhs,
    std::vector<float>& solution
);
