#pragma once

#include <vector>
#include "Planet.hpp"

namespace SpaceGolf {

/**
 * @class Level
 * @brief Manages the collection of Planets that make up a physics scenario.
 * 
 * The Level is responsible for holding the world state (the planets) and provides
 * robust utilities for procedurally generating valid, non-overlapping layouts.
 */
class Level {
public:
    /**
     * @brief The array of planets present in this level.
     * The simulation engine iterates over this array to calculate gravity.
     */
    std::vector<Planet> planets;

    /**
     * @brief Default constructor. Creates an empty level with no planets.
     */
    Level() = default;
    
    /**
     * @brief Constructs a level using an explicitly defined list of planets.
     * @param initialPlanets A vector of predefined planets.
     */
    Level(const std::vector<Planet>& initialPlanets);
    
    /**
     * @brief Procedurally generates a random level layout.
     * 
     * Randomly assigns mass and position to `numPlanets` planets. Guarantees that
     * no planets overlap each other and all planets fit within the provided bounds.
     * 
     * @param numPlanets The exact number of planets to spawn.
     * @param width The maximum width of the play area.
     * @param height The maximum height of the play area.
     */
    Level(int numPlanets, double width, double height);

    /**
     * @brief Appends procedurally generated random planets to the existing level.
     * 
     * Operates identically to the procedural constructor, but preserves existing
     * planets and ensures new ones don't overlap with them.
     * 
     * @param numPlanets The exact number of planets to add.
     * @param width The maximum width of the play area.
     * @param height The maximum height of the play area.
     */
    void addRandomPlanets(int numPlanets, double width, double height);

    /**
     * @brief Validates if a proposed planet intersects with any existing planets.
     * 
     * This function is used during procedural generation to ensure a minimum 
     * safe distance between celestial bodies.
     * 
     * @param newPlanet The planet to validate.
     * @return True if the position is safe and non-overlapping, False otherwise.
     */
    bool isValidPlanetPosition(const Planet& newPlanet) const;
};

}
