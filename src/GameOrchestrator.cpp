#include "Game/GameOrchestrator.hpp"
#include "GravityBilliards/Geometry.hpp"

namespace GravityGame {

GameOrchestrator::GameOrchestrator(const GravityBilliards::World& initialWorld, const GravityBilliards::Vector2D& playerStartPos)
    : world(initialWorld),
      physicsEngine(
          GravityBilliards::EulerIntegrator(),
          GravityBilliards::CircleCollisionDetector(),
          GameCollisionResolver()
      )
{
    physicsEngine.collisionResolver.collisionFlag = &hasCollidedThisFrame;
    physicsEngine.collisionResolver.collisionPos = &collisionPos;
    
    player.position = playerStartPos;
    player.velocity = {0.0, 0.0};
    player.isPhysicsBound = true;
}

void GameOrchestrator::update() {
    if (isGameOver) return;

    // Reset frame flags
    hasCollidedThisFrame = false;

    double dt = physicsEngine.dt;

    // Apply thrust to player velocity before physics step
    player.velocity += playerThrust * dt;

    // 1. Advance mobile world attractors (moving planets)
    world.advance(currentTime);

    // 2. Update rail-bound trajectories for entities
    for (auto& entity : entities) {
        if (entity.active && entity.trajectory.has_value() && entity.parentAttractorIndex.has_value()) {
            int parentIdx = entity.parentAttractorIndex.value();
            if (parentIdx >= 0 && parentIdx < world.attractors.size()) {
                const auto& parent = world.attractors[parentIdx];
                std::visit([&](auto&& traj) {
                    using T = std::decay_t<decltype(traj)>;
                    if constexpr (std::is_same_v<T, GravityBilliards::KeplerOrbit>) {
                        entity.position = GravityBilliards::Trajectories::solveKeplerOrbit(
                            traj, parent.position, parent.mass, currentTime
                        );
                    } else if constexpr (std::is_same_v<T, GravityBilliards::KinematicTrajectory>) {
                        entity.position = GravityBilliards::Trajectories::solveKinematicTrajectory(traj, currentTime);
                    }
                }, entity.trajectory.value());
            }
        }
    }

    // 3. Step physics-bound entities
    if (player.active && player.isPhysicsBound) {
        physicsEngine.step(world, player.position, player.velocity);
    }
    for (auto& entity : entities) {
        if (entity.active && entity.isPhysicsBound) {
            physicsEngine.step(world, entity.position, entity.velocity);
        }
    }

    // 4. Resolve game-specific interactions (collecting items, taking damage from enemies)
    for (auto& entity : entities) {
        if (entity.active && player.active) {
            bool intersect = GravityBilliards::Geometry::checkCircleIntersection(
                player.position, player.radius,
                entity.position, entity.radius
            );
            if (intersect) {
                if (entity.scoreValue > 0) {
                    entity.active = false; // Collected
                    score += entity.scoreValue;
                }
                if (entity.damageValue > 0) {
                    // Trigger damage via the collision flag
                    hasCollidedThisFrame = true;
                    collisionPos = entity.position;
                    // Optional: deactivate enemy if it's a one-time obstacle
                    // entity.active = false; 
                }
            }
        }
    }

    // Update global clock
    currentTime += dt;
}

} // namespace GravityGame
