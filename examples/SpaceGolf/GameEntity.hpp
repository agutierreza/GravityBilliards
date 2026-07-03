#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include "GravityBilliards/Trajectories.hpp"
#include <optional>

namespace GravityGame {

/**
 * @brief Represents a generic interactive element in the game.
 * Uses simple behavior flags to determine if it is a collectible, enemy, player, or projectile.
 */
struct GameEntity {
    GravityBilliards::Vector2D position;
    GravityBilliards::Vector2D velocity;
    double radius = 5.0;
    double mass = 1.0;
    bool active = true;

    // Behavior Flags
    bool isPhysicsBound = false; // Does it bounce/use gravity?
    int scoreValue = 0;          // > 0 means collectible
    int damageValue = 0;         // > 0 means enemy/obstacle
    
    // Rail logic
    std::optional<GravityBilliards::TrajectoryVariant> trajectory;
    std::optional<int> parentAttractorIndex;
};

} // namespace GravityGame
