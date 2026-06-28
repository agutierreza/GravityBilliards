#pragma once

#include <vector>
#include "Attractor.hpp"

namespace GravityBilliards {

/**
 * @class World
 * @brief Manages the collection of Attractors that make up a physics problem or environment.
 * 
 * The World is responsible for holding the world state (the attractors) and provides
 * robust utilities for procedurally generating valid, non-overlapping layouts.
 */
class World {
public:
    /**
     * @brief The array of attractors present in this world.
     * The simulation engine iterates over this array to calculate gravity.
     */
    std::vector<Attractor> attractors;

    /**
     * @brief Default constructor. Creates an empty world with no attractors.
     */
    World() = default;
    
    /**
     * @brief Constructs a world using an explicitly defined list of attractors.
     * @param initialAttractors A vector of predefined attractors.
     */
    World(const std::vector<Attractor>& initialAttractors);
    
    /**
     * @brief Procedurally generates a random world layout.
     * 
     * Randomly assigns mass and position to `numAttractors` attractors. Guarantees that
     * no attractors overlap each other and all attractors fit within the provided bounds.
     * 
     * @param numAttractors The exact number of attractors to spawn.
     * @param width The maximum width of the play area.
     * @param height The maximum height of the play area.
     */
    World(int numAttractors, double width, double height);

    /**
     * @brief Appends procedurally generated random attractors to the existing world.
     * 
     * Operates identically to the procedural constructor, but preserves existing
     * attractors and ensures new ones don't overlap with them.
     * 
     * @param numAttractors The exact number of attractors to add.
     * @param width The maximum width of the play area.
     * @param height The maximum height of the play area.
     */
    void addRandomAttractors(int numAttractors, double width, double height);

    /**
     * @brief Validates if a proposed attractor intersects with any existing attractors.
     * 
     * This function is used during procedural generation to ensure a minimum 
     * safe distance between celestial bodies.
     * 
     * @param newAttractor The attractor to validate.
     * @return True if the position is safe and non-overlapping, False otherwise.
     */
    bool isValidAttractorPosition(const Attractor& newAttractor) const;

    /**
     * @brief Calculates the total gravity force (acceleration) vector at a given position.
     * 
     * @param pos The position to calculate the force at.
     * @return A Vector2D representing the net gravitational pull.
     */
    Vector2D getGravityForceAt(const Vector2D& pos) const;

    /**
     * @brief Calculates the gravitational potential at a given position.
     * 
     * This can be visualized as the depth of the "gravity well" on a 2D surface,
     * where attractors carve holes proportional to their mass and distance.
     * 
     * @param pos The position to calculate the potential at.
     * @return The scalar gravitational potential.
     */
    double getGravityPotentialAt(const Vector2D& pos) const;

    /**
     * @brief Advances the state of the world, updating positions of any mobile attractors.
     * @param time The absolute simulation time.
     */
    void advance(double time);
};

}
