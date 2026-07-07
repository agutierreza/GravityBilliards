#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <cmath>

namespace GravityBilliards {

/**
 * @brief Base interface for a world topology defining distances and directions.
 */
class ITopology {
public:
    virtual ~ITopology() = default;

    /// @brief Gets the shortest direction vector from one point to another.
    virtual Vector2D getShortestDirection(const Vector2D& from, const Vector2D& to) const = 0;

    /// @brief Gets the shortest wrapped distance between two points.
    virtual double getDistance(const Vector2D& from, const Vector2D& to) const = 0;

    /// @brief Wraps the position according to the topology.
    virtual void wrapPosition(Vector2D& pos) const = 0;
};

/**
 * @brief Represents a standard, infinite Euclidean plane.
 */
struct EuclideanTopology : public ITopology {
    /// @brief Gets the shortest direction vector from one point to another.
    /// @param from The starting position.
    /// @param to The target position.
    /// @return The directional vector.
    Vector2D getShortestDirection(const Vector2D& from, const Vector2D& to) const override {
        return to - from;
    }

    /// @brief Gets the shortest wrapped distance between two points.
    /// @param from The starting position.
    /// @param to The target position.
    /// @return The distance.
    double getDistance(const Vector2D& from, const Vector2D& to) const override {
        return from.distanceTo(to);
    }

    /// @brief Wraps the position according to the topology. Does nothing for Euclidean.
    /// @param pos The position to wrap (modified in place).
    void wrapPosition(Vector2D& pos) const override {
        // Infinite plane, no wrapping
    }
};

/**
 * @brief Represents a Toroidal (wrapping) world surface.
 */
class ToroidalTopology : public ITopology {
public:
    double width;
    double height;

    ToroidalTopology() : width(10000.0), height(10000.0) {}

    /// @brief Constructs a ToroidalTopology.
    /// @param w The width of the world.
    /// @param h The height of the world.
    ToroidalTopology(double w, double h) : width(w), height(h) {}

    /// @brief Gets the shortest wrapped direction vector from one point to another.
    /// @param from The starting position.
    /// @param to The target position.
    /// @return The directional vector.
    Vector2D getShortestDirection(const Vector2D& from, const Vector2D& to) const override {
        double dx = to.x - from.x;
        if (dx > width / 2.0) dx -= width;
        else if (dx < -width / 2.0) dx += width;
        
        double dy = to.y - from.y;
        if (dy > height / 2.0) dy -= height;
        else if (dy < -height / 2.0) dy += height;
        
        return {dx, dy};
    }

    /// @brief Gets the shortest wrapped distance between two points.
    /// @param from The starting position.
    /// @param to The target position.
    /// @return The distance.
    double getDistance(const Vector2D& from, const Vector2D& to) const override {
        Vector2D dir = getShortestDirection(from, to);
        return std::sqrt(dir.x * dir.x + dir.y * dir.y);
    }

    /// @brief Wraps the position to stay within the torus bounds.
    /// @param pos The position to wrap (modified in place).
    void wrapPosition(Vector2D& pos) const override {
        pos.x = std::fmod(pos.x, width);
        if (pos.x < 0.0) pos.x += width;
        
        pos.y = std::fmod(pos.y, height);
        if (pos.y < 0.0) pos.y += height;
    }
};

} // namespace GravityBilliards
