#pragma once

#include <nlohmann/json.hpp>
#include "SpaceGolf/World.hpp"
#include "SpaceGolf/Simulation.hpp"
#include "SpaceGolf/Vector2D.hpp"

namespace SpaceGolf {

/**
 * @brief Represents a full physics initialConditions including the world layout, 
 * simulation parameters, and initial particle state.
 */
struct InitialConditions {
    World world;
    Vector2D particleStartPos;
    Vector2D particleStartVel;
    double particleRadius = 6.0;
    double stopVelocityThreshold = 0.01;

    /**
     * @brief Parses a InitialConditions from a JSON object.
     * @param j The nlohmann::json object containing the initialConditions data.
     * @return A fully populated InitialConditions instance.
     */
    static InitialConditions fromJson(const nlohmann::json& j);

    /**
     * @brief Serializes the InitialConditions into a JSON object.
     * @return An nlohmann::json object representing the initialConditions.
     */
    nlohmann::json toJson() const;
};

} // namespace SpaceGolf
