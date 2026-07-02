#pragma once

#include <cmath>

namespace GravityBilliards {

/**
 * @struct Vector2D
 * @brief Represents a mathematical 2D vector or coordinate in the physics engine.
 * 
 * Provides standard vector math operations necessary for gravity calculations
 * and velocity integration.
 */
struct Vector2D {
    double x; ///< The X coordinate of the vector.
    double y; ///< The Y coordinate of the vector.

    /**
     * @brief Default constructor. Initializes vector to (0, 0).
     */
    Vector2D() : x(0), y(0) {}

    /**
     * @brief Parameterized constructor.
     * @param x Initial X coordinate.
     * @param y Initial Y coordinate.
     */
    Vector2D(double x, double y) : x(x), y(y) {}

    // Operator Overloads
    /**
     * @brief Adds another vector to this one.
     * @param other Vector to add.
     * @return Resulting vector.
     */
    Vector2D operator+(const Vector2D& other) const { return Vector2D(x + other.x, y + other.y); }

    /**
     * @brief Subtracts another vector from this one.
     * @param other Vector to subtract.
     * @return Resulting vector.
     */
    Vector2D operator-(const Vector2D& other) const { return Vector2D(x - other.x, y - other.y); }

    /**
     * @brief Multiplies the vector by a scalar.
     * @param scalar The multiplier.
     * @return Resulting vector.
     */
    Vector2D operator*(double scalar) const { return Vector2D(x * scalar, y * scalar); }

    /**
     * @brief Divides the vector by a scalar.
     * @param scalar The divisor.
     * @return Resulting vector.
     */
    Vector2D operator/(double scalar) const { return Vector2D(x / scalar, y / scalar); }

    /**
     * @brief Adds another vector to this one in place.
     * @param other Vector to add.
     * @return Reference to this vector.
     */
    Vector2D& operator+=(const Vector2D& other) { x += other.x; y += other.y; return *this; }

    /**
     * @brief Subtracts another vector from this one in place.
     * @param other Vector to subtract.
     * @return Reference to this vector.
     */
    Vector2D& operator-=(const Vector2D& other) { x -= other.x; y -= other.y; return *this; }

    /**
     * @brief Calculates the exact length (magnitude) of the vector.
     * @return The magnitude as a double.
     */
    double magnitude() const { return std::sqrt(x * x + y * y); }

    /**
     * @brief Calculates the squared length of the vector.
     * 
     * This avoids the computational overhead of `std::sqrt`. Use this when 
     * comparing lengths/distances for performance optimization.
     * @return The squared magnitude.
     */
    double magnitudeSquared() const { return x * x + y * y; }

    /**
     * @brief Returns a vector with the same direction but a magnitude of 1.0.
     * @return The normalized unit vector. Returns (0,0) if magnitude is 0.
     */
    Vector2D normalized() const {
        double mag = magnitude();
        if (mag == 0) return Vector2D();
        return *this / mag;
    }

    /**
     * @brief Calculates the Euclidean distance between this vector and another.
     * @param other The target vector/position.
     * @return The exact distance.
     */
    double distanceTo(const Vector2D& other) const {
        return (*this - other).magnitude();
    }

    /**
     * @brief Calculates the squared Euclidean distance between this vector and another.
     * @param other The target vector/position.
     * @return The squared distance.
     */
    double distanceSquaredTo(const Vector2D& other) const {
        return (*this - other).magnitudeSquared();
    }

    /**
     * @brief Calculates the dot product of this vector and another.
     * @param other The target vector.
     * @return The scalar dot product.
     */
    double dot(const Vector2D& other) const {
        return x * other.x + y * other.y;
    }
};

}
