#include "collision.h"
#include "collision_common.h"
#include "simulation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace {

constexpr int NEIGHBOR_RANGE = 1;
constexpr int NEIGHBOR_WIDTH = 2 * NEIGHBOR_RANGE + 1;
constexpr int NEIGHBOR_CELL_COUNT = NEIGHBOR_WIDTH * NEIGHBOR_WIDTH;

struct CellOffset {
    int x;
    int y;
};

struct SpatialGrid {
    float cellSize;
    int columns;//X
    int rows;//Y
    std::vector<std::vector<size_t>> cells;
};

int GetCellX(const SpatialGrid& grid, float x)
{
    return static_cast<int> (x / grid.cellSize);
}

int GetCellY(const SpatialGrid& grid, float y){
    return static_cast<int>(y / grid.cellSize);
}

int GetCellIndex(const SpatialGrid& grid, int cellX, int cellY){
    return cellX + cellY * grid.columns;
}

std::vector<size_t> GetNeighboringCellIndices(
    const SpatialGrid& grid,
    int cellX,
    int cellY,
    const std::array<CellOffset, NEIGHBOR_CELL_COUNT>& neighborOffsets
)
{
    std::vector<size_t> neighboringCells;
    neighboringCells.reserve(neighborOffsets.size());

    for (const CellOffset offset : neighborOffsets) {
        const int neighborCellX = cellX + offset.x;
        const int neighborCellY = cellY + offset.y;

        if (neighborCellX < 0 ||
            neighborCellX >= grid.columns ||
            neighborCellY < 0 ||
            neighborCellY >= grid.rows) {
            continue;
        }

        neighboringCells.push_back(
            static_cast<size_t>(GetCellIndex(grid, neighborCellX, neighborCellY))
        );
    }

    return neighboringCells;
}

std::array<CellOffset, NEIGHBOR_CELL_COUNT> CreateNeighborOffsets()
{
    std::array<CellOffset, NEIGHBOR_CELL_COUNT> offsets;
    size_t offsetIndex = 0;

    for (int y = -NEIGHBOR_RANGE; y <= NEIGHBOR_RANGE; ++y) {
        for (int x = -NEIGHBOR_RANGE; x <= NEIGHBOR_RANGE; ++x) {
            offsets[offsetIndex] = { x, y };
            ++offsetIndex;
        }
    }

    return offsets;
}

SpatialGrid CreateSpatialGrid(float cellSize)
{
    SpatialGrid grid;

    grid.cellSize = cellSize;
    grid.columns = static_cast<int>(std::ceil(SCREEN_WIDTH / cellSize));
    grid.rows = static_cast<int>(std::ceil(SCREEN_HEIGHT / cellSize));
    grid.cells.resize(grid.columns * grid.rows);
    return grid;
}

void InsertParticles(SpatialGrid& grid, const std::vector<Particle>& particles){

    for (size_t i = 0; i < particles.size();i++){
        int cellX = GetCellX(grid, particles[i].position.x);
        int cellY = GetCellY(grid, particles[i].position.y);

        cellX = std::clamp(cellX, 0, grid.columns - 1);
        cellY = std::clamp(cellY, 0, grid.rows - 1);

        grid.cells[GetCellIndex(grid, cellX, cellY)].push_back(i);
    }
}

} // namespace

CollisionStats CheckParticleCollisionsSpatialGrid(
    std::vector<Particle>& particles,
    float restitution
)
{
    static std::mt19937 rng(std::random_device{}());

    ResetParticleCollisionFlags(particles);
    CollisionStats stats;

    SpatialGrid grid = CreateSpatialGrid(2 * PARTICLE_RADIUS);
    std::array<CellOffset, NEIGHBOR_CELL_COUNT> neighborOffsets = CreateNeighborOffsets();

    // Shuffling the cell offsets once per pass to break the fixed scan direction which introduces artifacts
    std::shuffle(neighborOffsets.begin(), neighborOffsets.end(), rng);
    
    InsertParticles(grid, particles);

    for (size_t i = 0; i < particles.size(); ++i)
    {
        std::vector<size_t> particleCandidates;
        std::vector<size_t> neighboringCells = GetNeighboringCellIndices(
            grid,
            GetCellX(grid, particles[i].position.x),
            GetCellY(grid, particles[i].position.y),
            neighborOffsets
        );

        for (size_t j = 0; j < neighboringCells.size(); j++)
        {
            const size_t cellIndex = neighboringCells[j];

            for (size_t k = 0; k < grid.cells[cellIndex].size(); k++)
            {
                particleCandidates.push_back(grid.cells[cellIndex][k]);
            }
        }

        for (size_t j = 0; j < particleCandidates.size(); j++)
        {
            size_t k = particleCandidates[j];
            if(k <= i){
                continue;
            }

            ++stats.candidateChecks;

            if (ResolveParticleCollision(particles[i], particles[k], restitution)) {
                ++stats.actualCollisions;
            }
        }
    
    }

    return stats;
}
