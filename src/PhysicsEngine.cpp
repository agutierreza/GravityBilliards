#include "GravityBilliards/PhysicsEngine.hpp"
#include <optional>
#include <cmath>

namespace GravityBilliards {

PhysicsEngine::PhysicsEngine(std::shared_ptr<Integrator> integrator,
                             std::shared_ptr<ICollisionDetector> detector,
                             std::shared_ptr<ICollisionResolver> resolver) 
    : integrator(integrator), collisionDetector(detector), collisionResolver(resolver) {}

void PhysicsEngine::step(const World& world, Vector2D& pos, Vector2D& vel) const {
    // 1. Check for collisions
    auto collision = collisionDetector ? collisionDetector->checkCollision(world, pos, particleRadius) : std::nullopt;
    
    if (collision.has_value()) {
        // 2. Resolve collision
        if (collisionResolver) {
            collisionResolver->resolve(*collision, pos, vel, bounceDamping, dt);
        }
    } else {
        // 3. Normal update (gravity + integration)
        Vector2D accel{0.0, 0.0};
        
        for (const auto& attractor : world.attractors) {
            double dist = pos.distanceTo(attractor.position);
            // Add gravity (Inverse square law)
            double cubeDistance = dist * dist * dist;
            if (cubeDistance > 0.0001) { // avoid division by zero
                accel += (attractor.position - pos) * (static_cast<double>(attractor.mass) / cubeDistance);
            }
        }
        
        if (integrator) {
            integrator->integrate(pos, vel, accel, dt);
        }
    }
}

Vector2D PhysicsEngine::predictPosition(const World& world, Vector2D startPos, Vector2D startVelocity, double time) const {
    Vector2D pos = startPos;
    Vector2D vel = startVelocity;
    
    int numSteps = static_cast<int>(time / dt);
    for (int i = 0; i < numSteps; ++i) {
        step(world, pos, vel);
    }
    
    return pos;
}

IntegratorResult PhysicsEngine::simulateUntilStop(const World& world, Vector2D startPos, Vector2D startVelocity, double timeoutSeconds) const {
    Vector2D pos = startPos;
    Vector2D vel = startVelocity;
    
    double elapsed = 0.0;
    bool stopped = false;
    double timeBelowThreshold = 0.0;
    
    while (elapsed < timeoutSeconds) {
        step(world, pos, vel);
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

std::vector<TracePoint> PhysicsEngine::getTrace(const World& world, Vector2D startPos, Vector2D startVelocity, double startTime, double endTime) const {
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
        step(world, pos, vel);
        currentSimTime += dt;
    }
    
    return trace;
}

} // namespace GravityBilliards
