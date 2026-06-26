#pragma once

#include "World.hpp"
#include "Vector2D.hpp"
#include <vector>

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
 * @class Integrator
 * @brief The pure virtual interface for the core physics engine.
 * 
 * Defines the contract for integration logic. Implementations can use
 * different algorithms (e.g., Euler, Runge-Kutta) to solve the motion.
 */
class Integrator {
public:
    virtual ~Integrator() = default;

    /**
     * @brief Performs a single integration step, handling gravity and collision.
     * 
     * @param world The attractor layout.
     * @param pos Current position (modified in place).
     * @param vel Current velocity (modified in place).
     */
    virtual void step(const World& world, Vector2D& pos, Vector2D& vel) const = 0;

    /**
     * @brief Simulates exactly `time` frames into the future and returns the end position.
     * 
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param time The exact number of frames to integrate forward.
     * @return The predicted final Vector2D position.
     */
    virtual Vector2D predictPosition(const World& world, Vector2D startPos, Vector2D startVelocity, double time) const = 0;
    
    /**
     * @brief Integrates physics continuously until the particle's velocity remains below a threshold.
     * 
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param timeoutSeconds A fail-safe timeout in frames to prevent infinite loops.
     * @return An IntegratorResult struct containing the stopping data.
     */
    virtual IntegratorResult simulateUntilStop(const World& world, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const = 0;
    
    /**
     * @brief Extracts a dense timeline array of the particle's movement between two points in time.
     * 
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param startTime The frame number to start recording.
     * @param endTime The frame number to stop recording.
     * @return A chronological array of state snapshots.
     */
    virtual std::vector<TracePoint> getTrace(const World& world, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const = 0;
};

} // namespace GravityBilliards
