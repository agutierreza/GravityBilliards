#pragma once

#include "GravityBilliards/World.hpp"
#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GameEntity.hpp"
#include <vector>

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
class GameOrchestrator {
public:
    using DefaultEngine = GravityBilliards::PhysicsEngine<
        GravityBilliards::EulerIntegrator, 
        GravityBilliards::CircleCollisionDetector, 
        GameCollisionResolver>;

    GravityBilliards::World world;
    DefaultEngine physicsEngine;
    
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

    GameOrchestrator(const GravityBilliards::World& initialWorld, const GravityBilliards::Vector2D& playerStartPos);

    /**
     * @brief Updates the entire game state by advancing time by physicsEngine.dt.
     */
    void update();
};

} // namespace GravityGame
