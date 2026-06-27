#pragma once

#include "GravityBilliards/CollisionTypes.hpp"
#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Vector2D.hpp"

namespace GravityBilliards {

/**
 * @class ICollisionDetector
 * @brief Pure virtual interface for collision detection logic.
 */
class ICollisionDetector {
public:
    virtual ~ICollisionDetector() = default;

    /**
     * @brief Checks if a particle is colliding with the world's attractors.
     * @param world The world containing the attractors.
     * @param pos Current position of the particle.
     * @param particleRadius Radius of the particle.
     * @return A valid CollisionInfo if a collision occurred, otherwise std::nullopt.
     */
    virtual std::optional<CollisionInfo> checkCollision(const World& world, const Vector2D& pos, double particleRadius) const = 0;
};

} // namespace GravityBilliards
