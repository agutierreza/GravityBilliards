#pragma once

#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include "GravityBilliards/Integrator.hpp"
#include <vector>
#include <memory>

namespace GravityBilliards {

/**
 * @struct TracePoint
 * @brief Represents a single snapshot of the particle's state at a specific time.
 */
struct TracePoint {
    Vector2D position; ///< The position of the particle.
    Vector2D velocity; ///< The velocity vector of the particle.
    double time;       ///< The exact time this snapshot was taken.
};

/**
 * @struct IntegratorResult
 * @brief Holds the final results of a `simulateUntilStop` execution.
 */
struct IntegratorResult {
    Vector2D finalPosition; ///< The location of the particle when the integrator ended.
    Vector2D finalVelocity; ///< The velocity of the particle when the integrator ended.
    double timeElapsed;     ///< The total amount of time simulated in frames.
    bool stopped;           ///< True if the particle successfully stopped, False if it hit the timeout.
};

/**
 * @class PhysicsEngine
 * @brief Orchestrator for the physics simulation.
 * 
 * Handles the simulation loop, gravity fields, and collision detection/resolution.
 * Uses a given Integrator strategy for advancing state through time.
 */
class PhysicsEngine {
public:
    double dt = 1.0;
    double stopVelocityThreshold = 0.05;
    double bounceDamping = 1.2;
    double particleRadius = 6.0;

    std::shared_ptr<Integrator> integrator;

    PhysicsEngine(std::shared_ptr<Integrator> integrator);

    void step(const World& world, Vector2D& pos, Vector2D& vel) const;
    Vector2D predictPosition(const World& world, Vector2D startPos, Vector2D startVelocity, double time) const;
    IntegratorResult simulateUntilStop(const World& world, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const;
    std::vector<TracePoint> getTrace(const World& world, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const;
};

} // namespace GravityBilliards
