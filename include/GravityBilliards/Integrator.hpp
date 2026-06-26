#pragma once

#include "GravityBilliards/Vector2D.hpp"

namespace GravityBilliards {

/**
 * @class Integrator
 * @brief The pure virtual interface for numerical integration.
 * 
 * Defines the math for advancing position and velocity over time `dt`.
 */
class Integrator {
public:
    virtual ~Integrator() = default;

    /**
     * @brief Integrates physics using given acceleration and time step.
     * 
     * @param pos Current position (modified in place).
     * @param vel Current velocity (modified in place).
     * @param accel Current acceleration.
     * @param dt Time delta.
     */
    virtual void integrate(Vector2D& pos, Vector2D& vel, const Vector2D& accel, double dt) const = 0;
};

} // namespace GravityBilliards
