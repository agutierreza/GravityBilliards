#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <algorithm>

namespace GravityBilliards {

/// @brief Stateless utility functions for geometric calculations and intersection tests.
struct Geometry {
    
    /// @brief Checks if two circles intersect.
    /// @param posA The center position of the first circle.
    /// @param radiusA The radius of the first circle.
    /// @param posB The center position of the second circle.
    /// @param radiusB The radius of the second circle.
    /// @return True if the circles intersect or touch, false otherwise.
    static bool checkCircleIntersection(const Vector2D& posA, double radiusA, const Vector2D& posB, double radiusB) {
        double distSq = posA.distanceSquaredTo(posB);
        double radSum = radiusA + radiusB;
        return distSq <= (radSum * radSum);
    }
    
    /// @brief Checks if a point lies within a circle.
    /// @param point The point to check.
    /// @param center The center position of the circle.
    /// @param radius The radius of the circle.
    /// @return True if the point is inside or on the edge of the circle, false otherwise.
    static bool isPointInCircle(const Vector2D& point, const Vector2D& center, double radius) {
        double distSq = point.distanceSquaredTo(center);
        return distSq <= (radius * radius);
    }

    /// @brief Calculates the squared distance from a point to a finite line segment.
    /// @param point The point to measure from.
    /// @param lineStart The start of the line segment.
    /// @param lineEnd The end of the line segment.
    /// @return The squared shortest distance.
    static double pointLineSegmentDistanceSquared(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd) {
        double l2 = lineStart.distanceSquaredTo(lineEnd);
        if (l2 == 0.0) return point.distanceSquaredTo(lineStart); // Line is just a point

        // Consider the line extending the segment, parameterized as lineStart + t (lineEnd - lineStart).
        // We find projection of point p onto the line. 
        // It falls where t = [(p-v) . (w-v)] / |w-v|^2
        double t = std::max(0.0, std::min(1.0, (point - lineStart).dot(lineEnd - lineStart) / l2));
        Vector2D projection = lineStart + (lineEnd - lineStart) * t;
        return point.distanceSquaredTo(projection);
    }

    /// @brief Checks if a finite line segment intersects with a circle.
    /// Useful for fast-moving objects (continuous collision) or laser raycasting.
    /// @param lineStart Start point of the ray/segment.
    /// @param lineEnd End point of the ray/segment.
    /// @param circleCenter The center of the circle.
    /// @param circleRadius The radius of the circle.
    /// @return True if the line segment touches or passes through the circle.
    static bool checkLineSegmentCircleIntersection(const Vector2D& lineStart, const Vector2D& lineEnd, const Vector2D& circleCenter, double circleRadius) {
        double distSq = pointLineSegmentDistanceSquared(circleCenter, lineStart, lineEnd);
        return distSq <= (circleRadius * circleRadius);
    }

    /// @brief Checks if an Axis-Aligned Bounding Box (AABB) intersects with a circle.
    /// @param minBound The top-left (minimum X,Y) corner of the box.
    /// @param maxBound The bottom-right (maximum X,Y) corner of the box.
    /// @param circleCenter The center of the circle.
    /// @param circleRadius The radius of the circle.
    /// @return True if they intersect.
    static bool checkAABBCircleIntersection(const Vector2D& minBound, const Vector2D& maxBound, const Vector2D& circleCenter, double circleRadius) {
        // Find the closest point on the AABB to the circle center
        double closestX = std::max(minBound.x, std::min(circleCenter.x, maxBound.x));
        double closestY = std::max(minBound.y, std::min(circleCenter.y, maxBound.y));

        Vector2D closestPoint(closestX, closestY);
        return circleCenter.distanceSquaredTo(closestPoint) <= (circleRadius * circleRadius);
    }
};

} // namespace GravityBilliards
