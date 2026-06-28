#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <cmath>

namespace GravityBilliards {

/// @brief Defines an analytic Keplerian orbit around a central body.
struct KeplerOrbit {
    double a;     ///< Semi-major axis
    double e;     ///< Eccentricity
    double omega; ///< Argument of periapsis (rotation of the ellipse)
    double M0;    ///< Mean anomaly at epoch (starting phase)
};

/// @brief Defines a purely kinematic trajectory (constant acceleration).
struct KinematicTrajectory {
    Vector2D initialPos;
    Vector2D initialVel;
    Vector2D acceleration;
    double startTime;
};

/// @brief Stateless utility functions for orbital mechanics and trajectories.
struct Trajectories {

    // ARCHITECTURE NOTE:
    // I was going to get ahead of myself and implement a generalised getTrace method right here, But I 
    // realised that if we want a trajectory (like an aiming line) to bounce off planets, it  
    // must stay in the PhysicsEngine, because that is where the Collision detector/resolver live! 
    // 
    // If we want a bullet or particle to behave like the player, we just feed its pos/vel into 
    // PhysicsEngine::step(). This toolkit is strictly for objects on perfect mathematical rails.
    // Eventually, we should overload PhysicsEngine::getTrace() to accept a specific list of 
    // attractors (so we can do partial traces), rather than building a trace generator here.
    
    /// @brief Calculates the exact 2D position of an object on a kinematic trajectory (constant acceleration).
    /// @param traj The kinematic parameters.
    /// @param time The current absolute simulation time.
    /// @return The calculated absolute position vector.
    static Vector2D solveKinematicTrajectory(const KinematicTrajectory& traj, double time) {
        double dt = time - traj.startTime;
        if (dt < 0.0) return traj.initialPos;
        return traj.initialPos + (traj.initialVel * dt) + (traj.acceleration * (0.5 * dt * dt));
    }
    
    /// @brief Calculates the exact 2D position of an object on a Keplerian orbit at a given time.
    /// 
    /// PHYSICAL CONSISTENCY NOTE: 
    /// This analytic solver is mathematically guaranteed to run at the exact same physical speed 
    /// as the N-body PhysicsEngine. This is because:
    /// 1. Both systems assume a Gravitational Constant of G = 1.0 (implied here by n = sqrt(M/a^3)).
    /// 2. The 'time' parameter is completely agnostic to frame rates or integration steps. As long 
    ///    as the accumulator driving this 'time' parameter and the accumulator driving the 
    ///    PhysicsEngine 'dt' steps are fed from the same master clock, a particle and a satellite 
    ///    on identical orbits will travel at identical speeds without desyncing.
    ///
    /// @param orbit The orbital parameters defining the path.
    /// @param parentPos The position of the central massive body.
    /// @param parentMass The mass of the central body (assuming G=1 as in the rest of the engine).
    /// @param time The current simulation time.
    /// @return The calculated absolute position vector.
    static Vector2D solveKeplerOrbit(const KeplerOrbit& orbit, const Vector2D& parentPos, double parentMass, double time) {
        // Mean motion
        double n = std::sqrt(parentMass / (orbit.a * orbit.a * orbit.a));
        // Mean anomaly
        double M = n * time + orbit.M0;
        
        // Solve Kepler's equation (M = E - e * sin(E)) for Eccentric anomaly (E) using Newton-Raphson
        double E = M; // Initial guess
        for (int i = 0; i < 5; ++i) {
            E = E - (E - orbit.e * std::sin(E) - M) / (1.0 - orbit.e * std::cos(E));
        }
        
        // True anomaly / Position in the orbital plane
        double x_plane = orbit.a * (std::cos(E) - orbit.e);
        double y_plane = orbit.a * std::sqrt(1.0 - orbit.e * orbit.e) * std::sin(E);
        
        // Rotate by argument of periapsis (omega)
        double cosOmega = std::cos(orbit.omega);
        double sinOmega = std::sin(orbit.omega);
        double x_rot = x_plane * cosOmega - y_plane * sinOmega;
        double y_rot = x_plane * sinOmega + y_plane * cosOmega;
        
        // Translate relative to parent position
        return parentPos + Vector2D(x_rot, y_rot);
    }

    /// @brief Calculates the exact 2D velocity vector of an object on a Keplerian orbit at a given time.
    /// Useful for seamlessly detaching a satellite and turning it into a free physics particle.
    /// @param orbit The orbital parameters defining the path.
    /// @param parentMass The mass of the central body.
    /// @param time The current simulation time.
    /// @return The calculated absolute velocity vector.
    static Vector2D getOrbitalVelocity(const KeplerOrbit& orbit, double parentMass, double time) {
        double n = std::sqrt(parentMass / (orbit.a * orbit.a * orbit.a));
        double M = n * time + orbit.M0;
        
        // Newton-Raphson for Eccentric anomaly (E)
        double E = M;
        for (int i = 0; i < 5; ++i) {
            E = E - (E - orbit.e * std::sin(E) - M) / (1.0 - orbit.e * std::cos(E));
        }

        // Derivative of E with respect to time
        double E_dot = n / (1.0 - orbit.e * std::cos(E));

        // Velocity in the orbital plane
        double vx_plane = -orbit.a * std::sin(E) * E_dot;
        double vy_plane = orbit.a * std::sqrt(1.0 - orbit.e * orbit.e) * std::cos(E) * E_dot;

        // Rotate by argument of periapsis (omega)
        double cosOmega = std::cos(orbit.omega);
        double sinOmega = std::sin(orbit.omega);
        double vx_rot = vx_plane * cosOmega - vy_plane * sinOmega;
        double vy_rot = vx_plane * sinOmega + vy_plane * cosOmega;

        return Vector2D(vx_rot, vy_rot);
    }

    /// @brief Calculates the time it takes for an object to complete one full orbit.
    /// @param orbit The orbital parameters.
    /// @param parentMass The mass of the central body.
    /// @return The period in seconds (simulation time).
    static double getOrbitalPeriod(const KeplerOrbit& orbit, double parentMass) {
        const double TWO_PI = 2.0 * 3.14159265358979323846;
        double n = std::sqrt(parentMass / (orbit.a * orbit.a * orbit.a));
        return TWO_PI / n;
    }

    /// @brief Calculates the velocity required to completely escape the gravitational pull of a body.
    /// @param parentMass The mass of the central body.
    /// @param distance The current distance from the center of the body.
    /// @return The escape velocity magnitude.
    static double getEscapeVelocity(double parentMass, double distance) {
        if (distance <= 0.0) return 0.0;
        // From Vis-viva equation where a -> infinity
        return std::sqrt(2.0 * parentMass / distance);
    }
};

} // namespace GravityBilliards
