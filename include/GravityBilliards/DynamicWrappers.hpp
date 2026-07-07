#pragma once

#include "GravityBilliards/ICollisionDetector.hpp"
#include "GravityBilliards/ICollisionResolver.hpp"
#include "GravityBilliards/Integrator.hpp"
#include <memory>
#include <optional>

namespace GravityBilliards {

/**
 * @struct DynamicDetector
 * @brief Wrapper for dynamic dispatch collision detection.
 */
struct DynamicDetector {
    std::shared_ptr<ICollisionDetector> ptr;
    std::optional<CollisionInfo> checkCollision(const World& world, const Vector2D& pos, double radius, const ITopology& topology) const {
        return ptr ? ptr->checkCollision(world, pos, radius, topology) : std::nullopt;
    }
};

/**
 * @struct DynamicResolver
 * @brief Wrapper for dynamic dispatch collision resolution.
 */
struct DynamicResolver {
    std::shared_ptr<ICollisionResolver> ptr;
    void resolve(const CollisionInfo& info, Vector2D& pos, Vector2D& vel, const Vector2D& accel, double damping, double dt) const {
        if (ptr) ptr->resolve(info, pos, vel, accel, damping, dt);
    }
};

/**
 * @struct DynamicIntegrator
 * @brief Wrapper for dynamic dispatch integration.
 */
struct DynamicIntegrator {
    std::shared_ptr<Integrator> ptr;
    void integrate(Vector2D& pos, Vector2D& vel, const Vector2D& accel, double dt) const {
        if (ptr) ptr->integrate(pos, vel, accel, dt);
    }
};

} // namespace GravityBilliards
