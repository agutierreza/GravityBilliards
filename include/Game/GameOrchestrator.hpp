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
 * @brief Resolves collisions and tracks impacts.
 */
struct GameCollisionResolver : public GravityBilliards::InelasticCollisionResolver {
    int* lastHitAttractorIndex = nullptr;
    double* lastImpactSpeed = nullptr;

    void resolve(const GravityBilliards::CollisionInfo& info, GravityBilliards::Vector2D& pos, GravityBilliards::Vector2D& vel, const GravityBilliards::Vector2D& accel, double damping, double dt) const {
        double impactSpeed = -vel.dot(info.normal);
        
        if (lastHitAttractorIndex) *lastHitAttractorIndex = info.hitAttractorIndex;
        if (lastImpactSpeed) *lastImpactSpeed = impactSpeed;

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

enum class GameStatus { PLAYING, WON, LOST };

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
    int totalCollectibles = 0;
    int health = 5;
    double damageCooldown = 0.0;
    double currentTime = 0.0;
    GameStatus status = GameStatus::PLAYING;

    // Player inputs
    GravityBilliards::Vector2D playerThrust;

    // Physics hit reporting
    int lastHitAttractorIndex = -1;
    double lastImpactSpeed = 0.0;

    GameOrchestrator(const GravityBilliards::World& initialWorld, const GravityBilliards::Vector2D& playerStartPos, TopologyType top = TopologyType{})
        : world(initialWorld),
          physicsEngine(
              GravityBilliards::EulerIntegrator(),
              GravityBilliards::CircleCollisionDetector(),
              GameCollisionResolver(),
              top
          )
    {
        physicsEngine.collisionResolver.lastHitAttractorIndex = &lastHitAttractorIndex;
        physicsEngine.collisionResolver.lastImpactSpeed = &lastImpactSpeed;
        
        player.position = playerStartPos;
        player.velocity = {0.0, 0.0};
        player.isPhysicsBound = true;
    }

    /**
     * @brief Updates the entire game state by advancing time by physicsEngine.dt.
     */
    void update() {
        if (status != GameStatus::PLAYING) return;

        // Reset frame flags
        lastHitAttractorIndex = -1;
        lastImpactSpeed = 0.0;
        
        double dt = physicsEngine.dt;
        if (damageCooldown > 0.0) damageCooldown -= dt;

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
            
            // Process Player's Planet Impacts immediately
            if (lastHitAttractorIndex != -1) {
                if (lastImpactSpeed > 1.0) { // Hard crash
                    if (damageCooldown <= 0.0) {
                        health--;
                        damageCooldown = 1.0;
                        if (health <= 0) status = GameStatus::LOST;
                    }
                } else if (lastHitAttractorIndex == 0) { // Gentle landing on home
                    if (score >= totalCollectibles * 10) {
                        status = GameStatus::WON;
                    }
                }
            }
        }

        // Reset so entities don't trigger player impacts
        lastHitAttractorIndex = -1;
        lastImpactSpeed = 0.0;

        for (auto& entity : entities) {
            if (entity.active && entity.isPhysicsBound) {
                physicsEngine.step(world, entity.position, entity.velocity);
                // We ignore entity-vs-planet impacts for now
                lastHitAttractorIndex = -1;
                lastImpactSpeed = 0.0;
            }
        }

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
                        if (damageCooldown <= 0.0) {
                            health--;
                            damageCooldown = 1.0;
                            if (health <= 0) status = GameStatus::LOST;
                        }

                        // Detach from rails and make physical
                        if (!entity.isPhysicsBound) {
                            entity.trajectory.reset();
                            entity.isPhysicsBound = true;
                        }

                        // Calculate partially inelastic collision
                        GravityBilliards::Vector2D delta = player.position - entity.position;
                        double dist = delta.magnitude();
                        if (dist > 0.0001) {
                            GravityBilliards::Vector2D normal = delta / dist;
                            
                            // Resolve physical overlap so they don't get stuck inside each other
                            double overlap = (player.radius + entity.radius) - dist;
                            if (overlap > 0) {
                                double invMassP = 1.0 / player.mass;
                                double invMassE = 1.0 / entity.mass;
                                double sumInvMass = invMassP + invMassE;
                                
                                player.position += normal * (overlap * (invMassP / sumInvMass));
                                entity.position -= normal * (overlap * (invMassE / sumInvMass));
                            }

                            // Exchange momentum
                            GravityBilliards::Vector2D relativeVel = player.velocity - entity.velocity;
                            double velocityAlongNormal = relativeVel.dot(normal);
                            
                            // Only resolve if they are moving towards each other
                            if (velocityAlongNormal < 0) {
                                double e = 0.2; // Partially inelastic (bounces slightly)
                                double j = -(1.0 + e) * velocityAlongNormal;
                                double invMassP = 1.0 / player.mass;
                                double invMassE = 1.0 / entity.mass;
                                j /= (invMassP + invMassE);
                                
                                GravityBilliards::Vector2D impulse = normal * j;
                                player.velocity += impulse * invMassP;
                                entity.velocity -= impulse * invMassE;
                            }
                        }
                    }
                }
            }
        }

        // (Planet impacts are now processed immediately after player step)

        // Update global clock
        currentTime += dt;
    }
};

} // namespace GravityGame
