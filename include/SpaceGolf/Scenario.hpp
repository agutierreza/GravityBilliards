#pragma once

#include <nlohmann/json.hpp>
#include "SpaceGolf/Level.hpp"
#include "SpaceGolf/Simulation.hpp"
#include "SpaceGolf/Vector2D.hpp"

namespace SpaceGolf {

/**
 * @brief Represents a full physics scenario including the level layout, 
 * simulation parameters, and initial particle state.
 */
struct Scenario {
    Level level;
    Vector2D particleStartPos;
    Vector2D particleStartVel;
    double particleRadius = 6.0;
    double stopVelocityThreshold = 0.01;

    /**
     * @brief Parses a Scenario from a JSON object.
     * @param j The nlohmann::json object containing the scenario data.
     * @return A fully populated Scenario instance.
     */
    static Scenario fromJson(const nlohmann::json& j);

    /**
     * @brief Serializes the Scenario into a JSON object.
     * @return An nlohmann::json object representing the scenario.
     */
    nlohmann::json toJson() const;
};

} // namespace SpaceGolf
