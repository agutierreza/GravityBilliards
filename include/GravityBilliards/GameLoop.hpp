#pragma once

#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/InitialConditions.hpp"
#include "GravityBilliards/GameCommand.hpp"
#include <vector>
#include <memory>

namespace GravityBilliards {

/**
 * @class GameLoop
 * @brief Manages the tick-by-tick simulation state and command queuing.
 * 
 * Provides a decoupled architectural layer, allowing external UI/Peripheral modules
 * to submit agnostic mathematical commands (like ApplyThrust) without handling
 * physical integration details.
 */
template <typename TIntegrator, typename TDetector, typename TResolver>
class GameLoop {
private:
    PhysicsEngine<TIntegrator, TDetector, TResolver> m_engine;
    World m_world;
    Vector2D m_position;
    Vector2D m_velocity;
    
    double m_currentTime = 0.0;
    double m_timeBelowThreshold = 0.0;
    bool m_stopped = false;
    
    std::vector<std::unique_ptr<GameCommand>> m_pendingCommands;

public:
    /**
     * @brief Constructs a new game loop.
     * @param ic The initial conditions for the simulation.
     * @param integrator The underlying mathematical integrator to use.
     * @param detector The collision detection strategy.
     * @param resolver The collision resolution strategy.
     */
    GameLoop(const InitialConditions& ic, 
             TIntegrator integrator = TIntegrator(), 
             TDetector detector = TDetector(), 
             TResolver resolver = TResolver())
        : m_engine(std::move(integrator), std::move(detector), std::move(resolver)),
          m_world(ic.world),
          m_position(ic.particleStartPos),
          m_velocity(ic.particleStartVel)
    {
        m_engine.particleRadius = ic.particleRadius;
        m_engine.stopVelocityThreshold = ic.stopVelocityThreshold;
    }

    /**
     * @brief Steps the simulation forward by one tick (dt).
     * 
     * Processes any queued commands first, then integrates the physics step,
     * and finally updates the stopped detection state.
     */
    void step() {
        // 1. Process all pending commands
        for (auto& cmd : m_pendingCommands) {
            if (cmd) {
                cmd->execute(m_position, m_velocity, m_engine.dt);
            }
        }
        m_pendingCommands.clear();

        // 2. Perform physics step
        m_engine.step(m_world, m_position, m_velocity);
        m_currentTime += m_engine.dt;

        // 3. Update stop detection state
        double stopThreshold = m_engine.stopVelocityThreshold;
        if (m_velocity.magnitudeSquared() < (stopThreshold * stopThreshold)) {
            m_timeBelowThreshold += m_engine.dt;
            if (m_timeBelowThreshold >= 15.0) {
                m_stopped = true;
            }
        } else {
            m_timeBelowThreshold = 0.0;
            m_stopped = false;
        }
    }

    /**
     * @brief Queues a command to be executed on the next tick.
     * @param command The command to execute.
     */
    void queueCommand(std::unique_ptr<GameCommand> command) {
        m_pendingCommands.push_back(std::move(command));
    }

    /**
     * @brief Retrieves the current position of the particle.
     * @return The current position vector.
     */
    Vector2D getPosition() const { return m_position; }

    /**
     * @brief Retrieves the current velocity of the particle.
     * @return The current velocity vector.
     */
    Vector2D getVelocity() const { return m_velocity; }

    /**
     * @brief Retrieves the current simulation time.
     * @return The simulation time in frames.
     */
    double getCurrentTime() const { return m_currentTime; }

    /**
     * @brief Checks if the particle has come to a stop.
     * @return True if the particle's velocity has remained below the threshold for 15 frames, false otherwise.
     */
    bool isStopped() const { return m_stopped; }

    /**
     * @brief Retrieves the physics engine instance.
     * @return A reference to the underlying PhysicsEngine.
     */
    PhysicsEngine<TIntegrator, TDetector, TResolver>& getEngine() { return m_engine; }

    /**
     * @brief Retrieves the physics engine instance.
     * @return A const reference to the underlying PhysicsEngine.
     */
    const PhysicsEngine<TIntegrator, TDetector, TResolver>& getEngine() const { return m_engine; }
    
    /**
     * @brief Retrieves the current world layout.
     * @return A const reference to the world.
     */
    const World& getWorld() const { return m_world; }
};

} // namespace GravityBilliards
