#pragma once

#include "GravityBilliards/ICollisionResolver.hpp"

namespace GravityBilliards {

/**
 * @class InelasticCollisionResolver
 * @brief Concrete implementation for resolving inelastic bounces.
 */
class InelasticCollisionResolver : public ICollisionResolver {
public:
    InelasticCollisionResolver() = default;
    virtual ~InelasticCollisionResolver() = default;

    /**
     * @brief Resolves collision by pushing the particle out and damping its velocity.
     * @param info Details of the collision.
     * @param pos Particle position (modified in place).
     * @param vel Particle velocity (modified in place).
     * @param bounceDamping Material property dictating energy loss.
     * @param dt Time step delta.
     */
    void resolve(const CollisionInfo& info, Vector2D& pos, Vector2D& vel, double bounceDamping, double dt) const override;
};

} // namespace GravityBilliards
