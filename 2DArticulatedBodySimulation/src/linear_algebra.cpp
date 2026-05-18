#include "linear_algebra.h"

#include <algorithm>
#include <cmath>

constexpr float LINEAR_SOLVE_EPSILON = 0.000001f;

std::vector<float> AddVectors(const std::vector<float>& a, const std::vector<float>& b)
{
    std::vector<float> result(a.size(), 0.0f);
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = a[i] + b[i];
    }

    return result;
}

std::vector<float> SubtractVectors(const std::vector<float>& a, const std::vector<float>& b)
{
    std::vector<float> result(a.size(), 0.0f);
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = a[i] - b[i];
    }

    return result;
}

std::vector<float> ScaleVector(const std::vector<float>& value, float scale)
{
    std::vector<float> result(value.size(), 0.0f);
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = value[i] * scale;
    }

    return result;
}

std::vector<float> PackState(
    const std::vector<float>& q,
    const std::vector<float>& qdot
)
{
    std::vector<float> state;
    state.reserve(q.size() + qdot.size());
    state.insert(state.end(), q.begin(), q.end());
    state.insert(state.end(), qdot.begin(), qdot.end());
    return state;
}

std::vector<float> UnpackQ(const std::vector<float>& state)
{
    const size_t dof = state.size() / 2;
    return std::vector<float>(state.begin(), state.begin() + dof);
}

std::vector<float> UnpackQdot(const std::vector<float>& state)
{
    const size_t dof = state.size() / 2;
    return std::vector<float>(state.begin() + dof, state.end());
}

bool SolveLinearSystem(DenseMatrix matrix, std::vector<float> rhs, std::vector<float>& solution)
{
    const size_t n = rhs.size();

    for (size_t pivotColumn = 0; pivotColumn < n; ++pivotColumn) {
        size_t pivotRow = pivotColumn;
        for (size_t row = pivotColumn + 1; row < n; ++row) {
            if (std::fabs(matrix[row][pivotColumn]) >
                std::fabs(matrix[pivotRow][pivotColumn])) {
                pivotRow = row;
            }
        }

        if (std::fabs(matrix[pivotRow][pivotColumn]) <= LINEAR_SOLVE_EPSILON) {
            return false;
        }

        if (pivotRow != pivotColumn) {
            std::swap(matrix[pivotColumn], matrix[pivotRow]);
            std::swap(rhs[pivotColumn], rhs[pivotRow]);
        }

        const float pivot = matrix[pivotColumn][pivotColumn];
        for (size_t column = pivotColumn; column < n; ++column) {
            matrix[pivotColumn][column] /= pivot;
        }
        rhs[pivotColumn] /= pivot;

        for (size_t row = 0; row < n; ++row) {
            if (row == pivotColumn) {
                continue;
            }

            const float factor = matrix[row][pivotColumn];
            for (size_t column = pivotColumn; column < n; ++column) {
                matrix[row][column] -= factor * matrix[pivotColumn][column];
            }
            rhs[row] -= factor * rhs[pivotColumn];
        }
    }

    solution = rhs;
    return true;
}
