#pragma once

#include "GravityBilliards/CollisionTypes.hpp"
#include "GravityBilliards/Vector2D.hpp"

namespace GravityBilliards {

/**
 * @class ICollisionResolver
 * @brief Pure virtual interface for collision resolution logic.
 */
class ICollisionResolver {
public:
    virtual ~ICollisionResolver() = default;

    /**
     * @brief Resolves a collision by modifying position and velocity.
     * @param info Details of the collision.
     * @param pos Particle position (modified in place).
     * @param vel Particle velocity (modified in place).
     * @param bounceDamping Material property dictating energy loss.
     * @param dt Time step delta.
     */
    virtual void resolve(const CollisionInfo& info, Vector2D& pos, Vector2D& vel, double bounceDamping, double dt) const = 0;
};

} // namespace GravityBilliards
