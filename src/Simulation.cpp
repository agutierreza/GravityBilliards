#include "SpaceGolf/Simulation.hpp"
#include <cmath>

namespace SpaceGolf {

void Simulation::step(const Level& level, Vector2D& pos, Vector2D& vel) const {
    Vector2D accel{0.0, 0.0};
    bool collided = false;
    Vector2D collisionNormal{0.0, 0.0};
    
    // First pass: Calculate gravity and check for collisions
    for (const auto& planet : level.planets) {
        double dist = pos.distanceTo(planet.position);
        
        if (dist < planet.radius + particleRadius) {
            collided = true;
            // The normal is pointing outward from the planet center to the particle
            collisionNormal = (pos - planet.position).normalized();
            // Move particle exactly to the surface to avoid sticking
            pos = planet.position + (collisionNormal * (planet.radius + particleRadius));
            break; // Handle one collision per step
        } else {
            // Add gravity (Inverse square law)
            // accel += (planet.pos - pos) * mass / dist^3
            // Which is equivalent to (dir / dist^2) * mass
            double cubeDistance = dist * dist * dist;
            accel += (planet.position - pos) * (static_cast<double>(planet.mass) / cubeDistance);
        }
    }
    
    if (collided) {
        // Inelastic collision projection
        // Project current velocity onto the normal and tangential components
        Vector2D p = collisionNormal; // already normalized
        Vector2D pOrtog{-p.y, p.x};
        
        double proyX = vel.dot(p);
        double proyY = vel.dot(pOrtog);
        
        // Restitution threshold: if impact is very slow, do not bounce to allow sleeping
        if (proyX < 0.0 && proyX > -0.5) {
            proyX = 0.0;
        } else {
            proyX = -proyX / bounceDamping;
        }
        
        Vector2D Ux = p * proyX;
        Vector2D Uy = pOrtog * (proyY / bounceDamping);
        
        vel = Ux + Uy;
        
        pos += vel * dt;
    } else {
        // Normal update
        vel += accel * dt;
        pos += vel * dt;
    }
}

Vector2D Simulation::predictPosition(const Level& level, Vector2D startPos, Vector2D startVelocity, double time) const {
    Vector2D pos = startPos;
    Vector2D vel = startVelocity;
    
    int numSteps = static_cast<int>(time / dt);
    for (int i = 0; i < numSteps; ++i) {
        step(level, pos, vel);
    }
    
    return pos;
}

SimulationResult Simulation::simulateUntilStop(const Level& level, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const {
    Vector2D pos = startPos;
    Vector2D vel = startVelocity;
    
    double elapsed = 0.0;
    bool stopped = false;
    double timeBelowThreshold = 0.0;
    
    while (elapsed < timeoutSeconds) {
        step(level, pos, vel);
        elapsed += dt;
        
        if (vel.magnitudeSquared() < (stopVelocityThreshold * stopVelocityThreshold)) {
            timeBelowThreshold += dt;
            // Require velocity to remain below threshold for 15.0 time units continuously
            if (timeBelowThreshold >= 15.0) {
                stopped = true;
                break;
            }
        } else {
            timeBelowThreshold = 0.0;
        }
    }
    
    return {pos, vel, elapsed, stopped};
}

std::vector<TracePoint> Simulation::getTrace(const Level& level, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const {
    std::vector<TracePoint> trace;
    
    if (endTime < startTime || startTime < 0.0) {
        return trace;
    }
    
    // Pre-allocate to prevent heap reallocations during trace
    int stepsToSimulate = static_cast<int>(endTime / dt);
    int stepsToRecord = static_cast<int>((endTime - startTime) / dt);
    trace.reserve(stepsToRecord + 1);
    
    Vector2D pos = startPos;
    Vector2D vel = startVelocity;
    double currentSimTime = 0.0;
    
    for (int i = 0; i <= stepsToSimulate; ++i) {
        if (currentSimTime >= startTime && currentSimTime <= endTime) {
            trace.push_back({pos, vel, currentSimTime});
        }
        step(level, pos, vel);
        currentSimTime += dt;
    }
    
    return trace;
}

} // namespace SpaceGolf
