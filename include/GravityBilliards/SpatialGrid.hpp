#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cstdint>

namespace GravityBilliards {

/// @brief A Broadphase spatial hashing grid for efficient O(1) neighborhood queries.
/// @tparam T The type of entity ID to store (e.g. int, size_t, or a struct).
template <typename T>
class SpatialGrid {
public:
    /// @brief Construct a new Spatial Grid
    /// @param cellSize The size of each grid bucket. Should roughly match the maximum expected object diameter.
    explicit SpatialGrid(double cellSize) : m_cellSize(cellSize) {
        if (m_cellSize <= 0.0) {
            m_cellSize = 100.0; // Fallback safe value
        }
    }

    /// @brief Clears all entities from the grid.
    void clear() {
        m_grid.clear();
    }

    /// @brief Inserts an entity into all cells its bounding box overlaps.
    /// @param entity The entity identifier.
    /// @param pos The center position of the entity.
    /// @param radius The radius (or half-extent) of the entity.
    void insert(T entity, const Vector2D& pos, double radius) {
        int minX = static_cast<int>(std::floor((pos.x - radius) / m_cellSize));
        int maxX = static_cast<int>(std::floor((pos.x + radius) / m_cellSize));
        int minY = static_cast<int>(std::floor((pos.y - radius) / m_cellSize));
        int maxY = static_cast<int>(std::floor((pos.y + radius) / m_cellSize));

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                m_grid[hash(x, y)].push_back(entity);
            }
        }
    }

    /// @brief Retrieves all potential entities that could intersect with the given query area.
    /// @param pos The center of the query area.
    /// @param radius The radius of the query area.
    /// @return A list of potential entity matches. Note: This may contain duplicates and false positives (due to AABB vs Circle logic).
    std::vector<T> getPotentialCollisions(const Vector2D& pos, double radius) const {
        std::vector<T> results;
        
        int minX = static_cast<int>(std::floor((pos.x - radius) / m_cellSize));
        int maxX = static_cast<int>(std::floor((pos.x + radius) / m_cellSize));
        int minY = static_cast<int>(std::floor((pos.y - radius) / m_cellSize));
        int maxY = static_cast<int>(std::floor((pos.y + radius) / m_cellSize));

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                uint64_t cellHash = hash(x, y);
                auto it = m_grid.find(cellHash);
                if (it != m_grid.end()) {
                    for (const auto& item : it->second) {
                        results.push_back(item);
                    }
                }
            }
        }
        
        return results;
    }

private:
    double m_cellSize;
    std::unordered_map<uint64_t, std::vector<T>> m_grid;

    /// @brief Computes a unique hash for a 2D grid coordinate.
    uint64_t hash(int x, int y) const {
        uint64_t ux = static_cast<uint64_t>(static_cast<uint32_t>(x));
        uint64_t uy = static_cast<uint64_t>(static_cast<uint32_t>(y));
        return (ux << 32) | uy;
    }
};

} // namespace GravityBilliards
