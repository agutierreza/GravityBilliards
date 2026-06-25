#pragma once

#include "Vector2D.hpp"

namespace SpaceGolf {

/**
 * @struct Planet
 * @brief Represents a celestial body in the Space Golf universe.
 * 
 * Planets exert gravitational pull on particles and act as solid collision bodies.
 * The radius of a planet is dynamically calculated based on its mass to ensure
 * a consistent 2D surface density across all objects.
 */
struct Planet {
    double mass;       ///< The gravitational mass of the planet. Determines pull strength.
    double radius;     ///< The physical radius of the planet. Determines collision boundaries.
    Vector2D position; ///< The center coordinates of the planet in 2D space.

    /**
     * @brief Default constructor. Creates a planet with mass 1 at (0,0).
     */
    Planet();

    /**
     * @brief Constructs a planet with a given mass at (0,0).
     * @param mass The mass of the planet.
     */
    Planet(int mass);

    /**
     * @brief Constructs a planet with a given integer mass and position.
     * @param mass The mass of the planet.
     * @param position The center position.
     */
    Planet(int mass, Vector2D position);

    /**
     * @brief Constructs a planet with a precise double mass and position.
     * @param m The mass of the planet.
     * @param pos The center position.
     */
    Planet(double m, const Vector2D& pos);
    
    /**
     * @brief Recalculates the radius of the planet.
     * 
     * This is called automatically by the constructor. It assumes a constant 2D density
     * such that the area of the planet is proportional to its mass (Area = Mass / Density).
     */
    void calculateRadius();
};

}
