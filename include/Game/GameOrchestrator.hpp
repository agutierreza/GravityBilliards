#pragma once

#include "GravityBilliards/World.hpp"
#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GravityBilliards/Geometry.hpp"
#include "GravityBilliards/Topology.hpp"
#include "GameEntity.hpp"
#include <vector>
#include <variant>

namespace GravityGame {

/**
 * @brief Resolves collisions and sets a flag if a hard impact occurs (used for player damage).
 */
struct GameCollisionResolver : public GravityBilliards::InelasticCollisionResolver {
    bool* collisionFlag = nullptr;
    GravityBilliards::Vector2D* collisionPos = nullptr;

    void resolve(const GravityBilliards::CollisionInfo& info, GravityBilliards::Vector2D& pos, GravityBilliards::Vector2D& vel, const GravityBilliards::Vector2D& accel, double damping, double dt) const {
        double impactSpeed = -vel.dot(info.normal);
        if (impactSpeed > 1.0) {
            if (collisionFlag) {
                *collisionFlag = true;
                if (collisionPos) *collisionPos = pos;
            }
        }
        GravityBilliards::InelasticCollisionResolver::resolve(info, pos, vel, accel, damping, dt);
    }
};

/**
 * @class GameOrchestrator
 * @brief Manages the overarching simulation state for a complete game.
 * 
 * Owns the physics engine, the world, and all generic entities.
 * Orchestrates each frame:
 * 1. Advances mobile world attractors.
 * 2. Updates rail-bound entity trajectories.
 * 3. Steps physics-bound entities.
 * 4. Resolves game-specific interactions.
 */
template <typename TopologyType = GravityBilliards::EuclideanTopology>
class GameOrchestrator {
public:
    using EngineType = GravityBilliards::PhysicsEngine<
        GravityBilliards::EulerIntegrator, 
        GravityBilliards::CircleCollisionDetector, 
        GameCollisionResolver,
        TopologyType>;

    GravityBilliards::World world;
    EngineType physicsEngine;
    
    // The player entity is tracked separately for easy camera/input binding
    GameEntity player;
    
    // All other generic entities (enemies, collectibles, bullets, obstacles)
    std::vector<GameEntity> entities;
    
    int score = 0;
    double currentTime = 0.0;
    bool isGameOver = false;

    // Player inputs
    GravityBilliards::Vector2D playerThrust;

    // Collision tracking for damage
    bool hasCollidedThisFrame = false;
    GravityBilliards::Vector2D collisionPos;

    GameOrchestrator(const GravityBilliards::World& initialWorld, const GravityBilliards::Vector2D& playerStartPos, TopologyType top = TopologyType{})
        : world(initialWorld),
          physicsEngine(
              GravityBilliards::EulerIntegrator(),
              GravityBilliards::CircleCollisionDetector(),
              GameCollisionResolver(),
              top
          )
    {
        physicsEngine.collisionResolver.collisionFlag = &hasCollidedThisFrame;
        physicsEngine.collisionResolver.collisionPos = &collisionPos;
        
        player.position = playerStartPos;
        player.velocity = {0.0, 0.0};
        player.isPhysicsBound = true;
    }

    /**
     * @brief Updates the entire game state by advancing time by physicsEngine.dt.
     */
    void update() {
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
                    }
                }
            }
        }

        // Update global clock
        currentTime += dt;
    }
};

} // namespace GravityGame
