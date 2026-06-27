#pragma once

#include "GravityBilliards/Vector2D.hpp"

namespace GravityBilliards {

/**
 * @class GameCommand
 * @brief Abstract interface for mathematical commands applied to the game state.
 * 
 * Provides a decoupled architectural layer, allowing UI/Peripheral modules to 
 * submit agnostic simulation operations.
 */
class GameCommand {
public:
    virtual ~GameCommand() = default;

    /**
     * @brief Executes the command, modifying the state.
     * @param position The current position of the particle.
     * @param velocity The current velocity of the particle.
     * @param dt The simulation time step delta.
     */
    virtual void execute(Vector2D& position, Vector2D& velocity, double dt) = 0;
};

/**
 * @class ApplyThrustCommand
 * @brief Applies an instantaneous Delta-V thrust (impulse) to the particle.
 */
class ApplyThrustCommand : public GameCommand {
public:
    /**
     * @brief The Delta-V vector to add to the velocity.
     */
    Vector2D deltaV;

    /**
     * @brief Constructs a new thrust command.
     * @param dv The Delta-V vector.
     */
    explicit ApplyThrustCommand(const Vector2D& dv) : deltaV(dv) {}

    /**
     * @brief Executes the thrust, adding Delta-V to the velocity.
     * @param position The current position of the particle.
     * @param velocity The current velocity of the particle.
     * @param dt The simulation time step delta.
     */
    void execute(Vector2D& position, Vector2D& velocity, double dt) override {
        (void)position;
        (void)dt;
        velocity += deltaV;
    }
};

/**
 * @class ApplyForceCommand
 * @brief Applies a continuous force over the duration of a tick.
 * 
 * Assumes a unit mass for the particle, resulting in a velocity change of F * dt.
 */
class ApplyForceCommand : public GameCommand {
public:
    /**
     * @brief The force vector to apply.
     */
    Vector2D force;

    /**
     * @brief Constructs a new force command.
     * @param f The force vector.
     */
    explicit ApplyForceCommand(const Vector2D& f) : force(f) {}

    /**
     * @brief Executes the force command, altering velocity based on the force and dt.
     * @param position The current position of the particle.
     * @param velocity The current velocity of the particle.
     * @param dt The simulation time step delta.
     */
    void execute(Vector2D& position, Vector2D& velocity, double dt) override {
        (void)position;
        velocity += force * dt;
    }
};

} // namespace GravityBilliards
