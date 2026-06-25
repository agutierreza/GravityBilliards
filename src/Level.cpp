#include "SpaceGolf/Level.hpp"
#include <random>

namespace SpaceGolf {

Level::Level(const std::vector<Planet>& initialPlanets) : planets(initialPlanets) {}

Level::Level(int numPlanets, double width, double height) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Mass distribution (e.g. 50 to 500)
    std::uniform_int_distribution<> massDist(50, 500);
    // Position distributions
    std::uniform_real_distribution<> xDist(0.0, width);
    std::uniform_real_distribution<> yDist(0.0, height);

    int maxAttempts = 1000;
    
    for (int i = 0; i < numPlanets; ++i) {
        bool placed = false;
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            Planet p(massDist(gen), {xDist(gen), yDist(gen)});
            
            // Need to make sure they are fully inside the bounds as well (optional but good)
            if (p.position.x - p.radius < 0 || p.position.x + p.radius > width ||
                p.position.y - p.radius < 0 || p.position.y + p.radius > height) {
                continue;
            }

            if (isValidPlanetPosition(p)) {
                planets.push_back(p);
                placed = true;
                break;
            }
        }
        
        // If we fail after maxAttempts, it's likely too crowded. 
        // For this assignment we just stop adding or continue with fewer planets.
        if (!placed) {
            break;
        }
    }
}

bool Level::isValidPlanetPosition(const Planet& newPlanet) const {
    for (const auto& p : planets) {
        double dist = p.position.distanceTo(newPlanet.position);
        // Add a tiny buffer to avoid edge cases where they touch perfectly
        if (dist <= (p.radius + newPlanet.radius + 1.0)) {
            return false;
        }
    }
    return true;
}

} // namespace SpaceGolf
