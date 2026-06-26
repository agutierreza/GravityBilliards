#pragma once

#include "Vector2D.hpp"

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

    /**
     * @brief Default constructor. Creates an attractor with mass 1 at (0,0).
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
     * @brief Constructs an attractor with a precise double mass and position.
     * @param m The mass of the attractor.
     * @param pos The center position.
     */
    Attractor(double m, const Vector2D& pos);
    
    /**
     * @brief Recalculates the radius of the attractor.
     * 
     * This is called automatically by the constructor. It assumes a constant 2D density
     * such that the area of the attractor is proportional to its mass (Area = Mass / Density).
     */
    void calculateRadius();
};

}
