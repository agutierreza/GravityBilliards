#pragma once

#include "GravityBilliards/ICollisionDetector.hpp"

namespace GravityBilliards {

/**
 * @class CircleCollisionDetector
 * @brief Concrete implementation of circle-vs-circle collision detection.
 */
class CircleCollisionDetector : public ICollisionDetector {
public:
    CircleCollisionDetector() = default;
    virtual ~CircleCollisionDetector() = default;

    /**
     * @brief Evaluates distance to all attractors to find intersections.
     * @param world The world containing the attractors.
     * @param pos Current position of the particle.
     * @param particleRadius Radius of the particle.
     * @return A valid CollisionInfo if an intersection is found, otherwise std::nullopt.
     */
    std::optional<CollisionInfo> checkCollision(const World& world, const Vector2D& pos, double particleRadius) const override;
};

} // namespace GravityBilliards
