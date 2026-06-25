#pragma once

#include "Level.hpp"
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
 * @struct SimulationResult
 * @brief Holds the final results of a `simulateUntilStop` execution.
 */
struct SimulationResult {
    Vector2D finalPosition; ///< The location of the particle when the simulation ended.
    Vector2D finalVelocity; ///< The velocity of the particle when the simulation ended.
    double timeElapsed;     ///< The total amount of time simulated in frames.
    bool stopped;           ///< True if the particle successfully stopped, False if it hit the timeout.
};

/**
 * @class Simulation
 * @brief The core physics engine.
 * 
 * Handles all Euler integration, inverse-square gravity calculation, and inelastic collision logic.
 * It is completely stateless; you pass the world state into its functions, allowing you to 
 * run multiple simulations simultaneously without side effects.
 */
class Simulation {
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
     * Used exclusively to pad the collision boundary around planets.
     */
    double particleRadius = 6.0;

    Simulation() = default;
    
    /**
     * @brief Performs a single integration step.
     * @param level The planet layout.
     * @param pos Current position (modified in place).
     * @param vel Current velocity (modified in place).
     */
    void step(const Level& level, Vector2D& pos, Vector2D& vel) const;

    /**
     * @brief Simulates exactly `time` frames into the future and returns the end position.
     * Useful for predicting where the ball will be at a specific moment without storing a trace.
     * 
     * @param level The planet layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param time The exact number of frames to integrate forward.
     * @return The predicted final Vector2D position.
     */
    Vector2D predictPosition(const Level& level, Vector2D startPos, Vector2D startVelocity, double time) const;
    
    /**
     * @brief Integrates physics continuously until the particle's velocity falls below `stopVelocityThreshold`.
     * 
     * @param level The planet layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param timeoutSeconds A fail-safe timeout in frames to prevent infinite loops if the particle escapes gravity.
     * @return A SimulationResult struct containing the stopping data.
     */
    SimulationResult simulateUntilStop(const Level& level, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const;
    
    /**
     * @brief Extracts a dense timeline array of the particle's movement between two points in time.
     * 
     * This function pre-allocates memory and returns a fully populated vector of `TracePoint`s.
     * It is heavily optimized for real-time visualization and playback.
     * 
     * @param level The planet layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param startTime The frame number to start recording.
     * @param endTime The frame number to stop recording.
     * @return A chronological array of state snapshots.
     */
    std::vector<TracePoint> getTrace(const Level& level, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const;
};

}
