#pragma once

#include "Vector2D.hpp"
#include "Trajectories.hpp"
#include <optional>

namespace GravityBilliards {

/**
 * @struct Attractor
 * @brief Represents a celestial body in the Space Golf universe.
 * 
 * Attractors exert gravitational pull on particles and act as solid collision bodies.
 * The radius of an attractor is dynamically calculated based on its mass to ensure
 * a consistent 2D surface density across all objects.
 */
struct Attractor {
    double mass;       ///< The gravitational mass of the attractor. Determines pull strength.
    double radius;     ///< The physical radius of the attractor. Determines collision boundaries.
    Vector2D position; ///< The center coordinates of the attractor in 2D space.
    
    // Future-proofing for advanced physics resolvers
    double spinVelocity = 0.0;       ///< Angular velocity (radians per frame)
    double surfaceFriction = 0.0;    ///< Coefficient of friction for spinning tangentials
    double surfaceBounciness = 1.0;  ///< Multiplier for the bounce restitution

    // Optional parameters for moving planets on rails
    std::optional<TrajectoryVariant> trajectory;    ///< If present, this attractor moves along a trajectory.
    std::optional<int> parentAttractorIndex;        ///< The index of the attractor this one orbits (if applicable).

    /**
     * @brief Default constructor. Creates an attractor with mass 1, radius 4 at (0,0).
     */
    Attractor();

    /**
     * @brief Constructs an attractor with a given mass at (0,0).
     * @param mass The mass of the attractor.
     */
    Attractor(int mass);

    /**
     * @brief Constructs an attractor with a given integer mass and position.
     * @param mass The mass of the attractor.
     * @param position The center position.
     */
    Attractor(int mass, Vector2D position);

    /**
     * @brief Constructs an attractor with explicit mass, radius, and position.
     * @param m The mass of the attractor.
     * @param r The physical collision radius.
     * @param pos The center position.
     */
    Attractor(double m, double r, const Vector2D& pos);
};

}
