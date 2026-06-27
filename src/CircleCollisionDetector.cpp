#include "GravityBilliards/CircleCollisionDetector.hpp"

namespace GravityBilliards {

std::optional<CollisionInfo> CircleCollisionDetector::checkCollision(const World& world, const Vector2D& pos, double particleRadius) const {
    for (const auto& attractor : world.attractors) {
        double dist = pos.distanceTo(attractor.position);
        double minSafeDistance = attractor.radius + particleRadius;
        
        if (dist < minSafeDistance) {
            CollisionInfo info;
            info.normal = (pos - attractor.position).normalized();
            info.penetration = minSafeDistance - dist;
            info.surfaceBounciness = attractor.surfaceBounciness;
            return info;
        }
    }
    return std::nullopt;
}

} // namespace GravityBilliards
