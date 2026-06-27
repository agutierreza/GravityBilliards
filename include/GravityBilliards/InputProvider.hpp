#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <optional>

namespace GravityBilliards {

/**
 * @class InputProvider
 * @brief Interface for polling peripheral inputs and returning abstract mathematical operations.
 * 
 * Provides a decoupled mechanism for the simulation to retrieve input state
 * without depending on UI frameworks or input libraries.
 */
class InputProvider {
public:
    virtual ~InputProvider() = default;
    
    /**
     * @brief Polls the input provider for a user-applied force during the current tick.
     * @return An optional Vector2D representing the force applied by the user. 
     *         If std::nullopt is returned, no force is applied.
     */
    virtual std::optional<Vector2D> getForceInput() = 0;
};

} // namespace GravityBilliards
