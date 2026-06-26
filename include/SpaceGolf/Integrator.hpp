#pragma once

#include "World.hpp"
#include "Vector2D.hpp"
#include <vector>

namespace SpaceGolf {

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
 * @brief The core physics engine.
 * 
 * Handles all Euler integration, inverse-square gravity calculation, and inelastic collision logic.
 * It is completely stateless; you pass the world state into its functions, allowing you to 
 * run multiple integrators simultaneously without side effects.
 */
class Integrator {
public:
    /**
     * @brief The time step delta applied per frame of physics integration.
     * Larger values calculate faster but reduce physics accuracy. (Default: 1.0)
     */
    double dt = 1.0;
    
    /**
     * @brief The velocity threshold squared below which the particle is considered completely "stopped".
     */
    double stopVelocityThreshold = 0.05;

    /**
     * @brief The energy loss damping applied to velocity on every bounce.
     */
    double bounceDamping = 1.2;

    /**
     * @brief The physical radius of the moving particle. 
     * Used exclusively to pad the collision boundary around attractors.
     */
    double particleRadius = 6.0;

    Integrator() = default;
    
    /**
     * @brief Performs a single integration step, handling gravity and collision.
     * 
     * Includes a restitution threshold to prevent micro-bouncing on low-velocity impacts.
     * 
     * @param world The attractor layout.
     * @param pos Current position (modified in place).
     * @param vel Current velocity (modified in place).
     */
    void step(const World& world, Vector2D& pos, Vector2D& vel) const;

    /**
     * @brief Simulates exactly `time` frames into the future and returns the end position.
     * Useful for predicting where the ball will be at a specific moment without storing a trace.
     * 
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param time The exact number of frames to integrate forward.
     * @return The predicted final Vector2D position.
     */
    Vector2D predictPosition(const World& world, Vector2D startPos, Vector2D startVelocity, double time) const;
    
    /**
     * @brief Integrates physics continuously until the particle's velocity remains below `stopVelocityThreshold` for 15 consecutive frames.
     * 
     * The 15-frame requirement prevents premature stops during bounce apexes where velocity momentarily drops near zero.
     * 
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param timeoutSeconds A fail-safe timeout in frames to prevent infinite loops if the particle escapes gravity.
     * @return An IntegratorResult struct containing the stopping data.
     */
    IntegratorResult simulateUntilStop(const World& world, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const;
    
    /**
     * @brief Extracts a dense timeline array of the particle's movement between two points in time.
     * 
     * This function pre-allocates memory and returns a fully populated vector of `TracePoint`s.
     * It is heavily optimized for real-time visualization and playback.
     * 
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param startTime The frame number to start recording.
     * @param endTime The frame number to stop recording.
     * @return A chronological array of state snapshots.
     */
    std::vector<TracePoint> getTrace(const World& world, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const;
};

}
