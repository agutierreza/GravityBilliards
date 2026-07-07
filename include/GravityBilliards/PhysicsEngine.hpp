#pragma once

#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include "GravityBilliards/CollisionTypes.hpp"
#include "GravityBilliards/Topology.hpp"
#include <vector>
#include <optional>
#include <cmath>

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
 * @brief The result returned when simulating until the particle comes to a rest.
 */
struct IntegratorResult {
    Vector2D finalPosition; ///< The location of the particle when the integrator ended.
    Vector2D finalVelocity; ///< The velocity of the particle when the integrator ended.
    double timeElapsed;     ///< The total amount of time simulated in frames.
    bool stopped;           ///< True if the particle successfully stopped, False if it hit the timeout.
};

/**
 * @class PhysicsEngine
 * @brief Orchestrates the integration, collision detection, and collision resolution into a cohesive simulation step.
 * 
 * Uses Static Polymorphism (Templates) for maximum performance.
 */
template <typename TIntegrator, typename TDetector, typename TResolver, typename TTopology = EuclideanTopology>
class PhysicsEngine {
public:
    /**
     * @brief The time step delta applied per frame of physics integration.
     */
    double dt = 1.0;
    
    /**
     * @brief The velocity threshold squared below which the particle is considered completely "stopped".
     */
    double stopVelocityThreshold = 0.05;
    
    /**
     * @brief The energy loss damping applied to velocity on every bounce.
     */
    double bounceDamping = 0.8;
    
    /**
     * @brief The physical radius of the moving particle. 
     */
    double particleRadius = 6.0;

    /**
     * @brief The mathematical integration strategy used to advance state.
     */
    TIntegrator integrator;
    
    /**
     * @brief The geometry checker to detect collisions.
     */
    TDetector collisionDetector;
    
    /**
     * @brief The physics handler for resolving collisions.
     */
    TResolver collisionResolver;

    /**
     * @brief The topology defining space and distance.
     */
    TTopology topology;

    /**
     * @brief Constructs a PhysicsEngine with specific strategies.
     * @param integrator The underlying mathematical integrator to use.
     * @param detector The collision detection strategy.
     * @param resolver The collision resolution strategy.
     * @param topology The topology of the space.
     */
    PhysicsEngine(TIntegrator integrator, TDetector detector, TResolver resolver, TTopology topology = TTopology{})
        : integrator(std::move(integrator)), collisionDetector(std::move(detector)), collisionResolver(std::move(resolver)), topology(std::move(topology)) {}

    /**
     * @brief Performs a single integration step, handling gravity and collision.
     * 
     * Gravity forces are computed and unconditionally integrated first to update position and velocity.
     * Then, collisions are checked. If detected, the collision resolver will zero out normal velocity 
     * and push the particle to the surface, dynamically using the frame's acceleration for resting calculations.
     * 
     * @param world The attractor layout.
     * @param pos Current position (modified in place).
     * @param vel Current velocity (modified in place).
     */
    void step(const World& world, Vector2D& pos, Vector2D& vel) const {
        Vector2D accel{0.0, 0.0};
        for (const auto& attractor : world.attractors) {
            Vector2D dir = topology.getShortestDirection(pos, attractor.position);
            double distSq = dir.x * dir.x + dir.y * dir.y;
            double dist = std::sqrt(distSq);
            double cubeDistance = dist * distSq;
            if (cubeDistance > 0.0001) { 
                accel += dir * (static_cast<double>(attractor.mass) / cubeDistance);
            }
        }
        integrator.integrate(pos, vel, accel, dt);
        topology.wrapPosition(pos);

        auto collision = collisionDetector.checkCollision(world, pos, particleRadius, topology);
        if (collision.has_value()) {
            collisionResolver.resolve(*collision, pos, vel, accel, bounceDamping, dt);
        }
    }

    /**
     * @brief Simulates exactly `time` frames into the future and returns the end position.
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param time The exact number of frames to integrate forward.
     * @return The predicted final Vector2D position.
     */
    Vector2D predictPosition(const World& world, Vector2D startPos, Vector2D startVelocity, double time) const {
        Vector2D pos = startPos;
        Vector2D vel = startVelocity;
        int numSteps = static_cast<int>(time / dt);
        for (int i = 0; i < numSteps; ++i) {
            step(world, pos, vel);
        }
        return pos;
    }

    /**
     * @brief Integrates physics continuously until the particle's velocity remains below `stopVelocityThreshold` for 15 consecutive frames.
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param timeoutSeconds A fail-safe timeout in frames to prevent infinite loops.
     * @return An IntegratorResult struct containing the stopping data.
     */
    IntegratorResult simulateUntilStop(const World& world, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const {
        Vector2D pos = startPos;
        Vector2D vel = startVelocity;
        double elapsed = 0.0;
        bool stopped = false;
        double timeBelowThreshold = 0.0;
        
        while (elapsed < timeoutSeconds) {
            step(world, pos, vel);
            elapsed += dt;
            
            if (vel.magnitudeSquared() < (stopVelocityThreshold * stopVelocityThreshold)) {
                timeBelowThreshold += dt;
                if (timeBelowThreshold >= 15.0) {
                    stopped = true;
                    break;
                }
            } else {
                timeBelowThreshold = 0.0;
            }
        }
        return {pos, vel, elapsed, stopped};
    }

    /**
     * @brief Extracts a dense timeline array of the particle's movement between two points in time.
     * @param world The attractor layout.
     * @param startPos Initial particle position.
     * @param startVelocity Initial particle velocity.
     * @param startTime The frame number to start recording.
     * @param endTime The frame number to stop recording.
     * @return A chronological array of state snapshots.
     */
    std::vector<TracePoint> getTrace(const World& world, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const {
        std::vector<TracePoint> trace;
        if (endTime < startTime || startTime < 0.0) return trace;
        
        int stepsToSimulate = static_cast<int>(endTime / dt);
        int stepsToRecord = static_cast<int>((endTime - startTime) / dt);
        trace.reserve(stepsToRecord + 1);
        
        Vector2D pos = startPos;
        Vector2D vel = startVelocity;
        double currentSimTime = 0.0;
        
        for (int i = 0; i <= stepsToSimulate; ++i) {
            if (currentSimTime >= startTime && currentSimTime <= endTime) {
                trace.push_back({pos, vel, currentSimTime});
            }
            step(world, pos, vel);
            currentSimTime += dt;
        }
        return trace;
    }
};

} // namespace GravityBilliards
