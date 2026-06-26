#pragma once

#include "GravityBilliards/Integrator.hpp"

namespace GravityBilliards {

/**
 * @class EulerIntegrator
 * @brief A concrete numerical integrator using Euler integration.
 */
class EulerIntegrator : public Integrator {
public:
    EulerIntegrator() = default;
    virtual ~EulerIntegrator() = default;
    
    /**
     * @brief Performs a single numerical integration step using Euler's method.
     * 
     * @param pos Current position (modified in place).
     * @param vel Current velocity (modified in place).
     * @param accel Current acceleration.
     * @param dt Time delta.
     */
    void integrate(Vector2D& pos, Vector2D& vel, const Vector2D& accel, double dt) const override;
};

} // namespace GravityBilliards
