#include "SpaceGolf/World.hpp"
#include <random>

namespace SpaceGolf {

World::World(const std::vector<Attractor>& initialAttractors) : attractors(initialAttractors) {}

World::World(int numAttractors, double width, double height) {
    addRandomAttractors(numAttractors, width, height);
}

void World::addRandomAttractors(int numAttractors, double width, double height) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Mass distribution (e.g. 50 to 500)
    std::uniform_int_distribution<> massDist(50, 500);
    // Position distributions
    std::uniform_real_distribution<> xDist(0.0, width);
    std::uniform_real_distribution<> yDist(0.0, height);

    int maxAttempts = 1000;
    
    for (int i = 0; i < numAttractors; ++i) {
        bool placed = false;
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            Attractor p(massDist(gen), {xDist(gen), yDist(gen)});
            
            // Need to make sure they are fully inside the bounds as well (optional but good)
            if (p.position.x - p.radius < 0 || p.position.x + p.radius > width ||
                p.position.y - p.radius < 0 || p.position.y + p.radius > height) {
                continue;
            }

            if (isValidAttractorPosition(p)) {
                attractors.push_back(p);
                placed = true;
                break;
            }
        }
        
        // If we fail after maxAttempts, it's likely too crowded. 
        // For this assignment we just stop adding or continue with fewer attractors.
        if (!placed) {
            break;
        }
    }
}

bool World::isValidAttractorPosition(const Attractor& newAttractor) const {
    for (const auto& p : attractors) {
        double dist = p.position.distanceTo(newAttractor.position);
        // Add a tiny buffer to avoid edge cases where they touch perfectly
        if (dist <= (p.radius + newAttractor.radius + 1.0)) {
            return false;
        }
    }
    return true;
}

Vector2D World::getGravityForceAt(const Vector2D& pos) const {
    Vector2D force{0.0, 0.0};
    for (const auto& attractor : attractors) {
        double dist = pos.distanceTo(attractor.position);
        if (dist > 0.0001) { // Prevent division by zero
            double cubeDistance = dist * dist * dist;
            force += (attractor.position - pos) * (static_cast<double>(attractor.mass) / cubeDistance);
        }
    }
    return force;
}

double World::getGravityPotentialAt(const Vector2D& pos) const {
    double potential = 0.0;
    for (const auto& attractor : attractors) {
        double dist = pos.distanceTo(attractor.position);
        if (dist > 0.0001) {
            potential -= static_cast<double>(attractor.mass) / dist;
        }
    }
    return potential;
}

} // namespace SpaceGolf
