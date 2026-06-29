#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <vector>
#include <cmath>

namespace Graphics {

/// @brief A standalone utility for translating bounded wrapped physics coordinates 
/// into continuous infinite rendering coordinates, and providing a continuous tracking camera.
class ToroidalCamera {
public:
    GravityBilliards::Vector2D position{0.0, 0.0};
    double viewportWidth = 1000.0;
    double viewportHeight = 800.0;
    double worldWidth = 1000.0;
    double worldHeight = 1000.0;

    ToroidalCamera() = default;

    /// @brief Initializes the camera.
    /// @param w The width of the torus world.
    /// @param h The height of the torus world.
    /// @param vw The width of the viewport.
    /// @param vh The height of the viewport.
    ToroidalCamera(double w, double h, double vw, double vh)
        : viewportWidth(vw), viewportHeight(vh), worldWidth(w), worldHeight(h) {}

    /// @brief Smoothly tracks a wrapped physics target position.
    /// @param targetPhysicsPos The target's position in wrapped physics space [0, W), [0, H).
    /// @param smoothingFactor The lerp factor (0.0 to 1.0).
    void trackTarget(const GravityBilliards::Vector2D& targetPhysicsPos, double smoothingFactor) {
        // 1. Safely wrap the continuous camera position into the physics space using double fmod
        double cWrapX = std::fmod(std::fmod(position.x, worldWidth) + worldWidth, worldWidth);
        double cWrapY = std::fmod(std::fmod(position.y, worldHeight) + worldHeight, worldHeight);

        // 2. Calculate the raw delta
        double dx = targetPhysicsPos.x - cWrapX;
        double dy = targetPhysicsPos.y - cWrapY;

        // 3. Adjust for toroidal shortest path
        if (dx > worldWidth / 2.0) dx -= worldWidth;
        else if (dx < -worldWidth / 2.0) dx += worldWidth;

        if (dy > worldHeight / 2.0) dy -= worldHeight;
        else if (dy < -worldHeight / 2.0) dy += worldHeight;

        // 4. Calculate continuous target position
        double tx = position.x + dx;
        double ty = position.y + dy;

        // 5. Lerp towards the target
        position.x += (tx - position.x) * smoothingFactor;
        position.y += (ty - position.y) * smoothingFactor;
    }

    /// @brief Calculates all visible continuous rendering coordinates for an object, including phantom edge-copies.
    /// @param objectPhysicsPos The object's position in wrapped physics space [0, W), [0, H).
    /// @param objectRadius The collision/render radius of the object.
    /// @return A list of continuous positions where the sprite should be rendered.
    std::vector<GravityBilliards::Vector2D> getVisibleRenderPositions(const GravityBilliards::Vector2D& objectPhysicsPos, double objectRadius) const {
        std::vector<GravityBilliards::Vector2D> visiblePositions;
        
        // 1. Find the base continuous render coordinate using the exact same shortest-path logic
        double cWrapX = std::fmod(std::fmod(position.x, worldWidth) + worldWidth, worldWidth);
        double cWrapY = std::fmod(std::fmod(position.y, worldHeight) + worldHeight, worldHeight);

        double dx = objectPhysicsPos.x - cWrapX;
        double dy = objectPhysicsPos.y - cWrapY;

        if (dx > worldWidth / 2.0) dx -= worldWidth;
        else if (dx < -worldWidth / 2.0) dx += worldWidth;

        if (dy > worldHeight / 2.0) dy -= worldHeight;
        else if (dy < -worldHeight / 2.0) dy += worldHeight;

        double rx = position.x + dx;
        double ry = position.y + dy;

        // Precompute viewport boundaries for fast culling
        double viewMinX = position.x - viewportWidth / 2.0;
        double viewMaxX = position.x + viewportWidth / 2.0;
        double viewMinY = position.y - viewportHeight / 2.0;
        double viewMaxY = position.y + viewportHeight / 2.0;

        // 2 & 3. Generate 3x3 grid and cull
        for (int ix = -1; ix <= 1; ++ix) {
            for (int iy = -1; iy <= 1; ++iy) {
                double candidateX = rx + (ix * worldWidth);
                double candidateY = ry + (iy * worldHeight);

                // AABB check against the viewport (with object radius buffer)
                if ((candidateX + objectRadius > viewMinX) && (candidateX - objectRadius < viewMaxX) &&
                    (candidateY + objectRadius > viewMinY) && (candidateY - objectRadius < viewMaxY)) {
                    
                    visiblePositions.push_back({candidateX, candidateY});
                }
            }
        }

        return visiblePositions;
    }
};

} // namespace Graphics
