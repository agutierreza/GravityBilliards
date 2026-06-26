#pragma once

#include <nlohmann/json.hpp>
#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Integrator.hpp"
#include "GravityBilliards/Vector2D.hpp"

namespace GravityBilliards {

/**
 * @brief Represents a full set of initial conditions including the world layout, 
 * integrator parameters, and initial particle state.
 */
struct InitialConditions {
    /**
     * @brief The initial world layout and attractors.
     */
    World world;

    /**
     * @brief The starting position of the particle.
     */
    Vector2D particleStartPos;

    /**
     * @brief The starting velocity vector of the particle.
     */
    Vector2D particleStartVel;

    /**
     * @brief The physical radius of the moving particle.
     */
    double particleRadius = 6.0;

    /**
     * @brief The velocity threshold squared below which the particle is considered completely "stopped".
     */
    double stopVelocityThreshold = 0.01;

    /**
     * @brief Parses an InitialConditions struct from a JSON object.
     * @param j The nlohmann::json object containing the state data.
     * @return A fully populated InitialConditions instance.
     */
    static InitialConditions fromJson(const nlohmann::json& j);

    /**
     * @brief Serializes the InitialConditions into a JSON object.
     * @return An nlohmann::json object representing these conditions.
     */
    nlohmann::json toJson() const;
};

} // namespace GravityBilliards
