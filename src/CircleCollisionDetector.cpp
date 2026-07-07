#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/Topology.hpp"

namespace GravityBilliards {

std::optional<CollisionInfo> CircleCollisionDetector::checkCollision(const World& world, const Vector2D& pos, double particleRadius, const ITopology& topology) const {
    for (size_t i = 0; i < world.attractors.size(); ++i) {
        const auto& attractor = world.attractors[i];
        
        // Use the topology to accurately calculate distance and direction for the collision
        double dist = topology.getDistance(pos, attractor.position);
        double minSafeDistance = attractor.radius + particleRadius;
        
        if (dist < minSafeDistance) {
            CollisionInfo info;
            // The collision normal should push the particle AWAY from the attractor.
            // getShortestDirection(from, to) goes from 'from' to 'to'.
            // So from attractor.position to pos gives the outward normal!
            info.normal = topology.getShortestDirection(attractor.position, pos).normalized();
            info.penetration = minSafeDistance - dist;
            info.surfaceBounciness = attractor.surfaceBounciness;
            info.hitAttractorIndex = static_cast<int>(i);
            return info;
        }
    }
    return std::nullopt;
}

} // namespace GravityBilliards
