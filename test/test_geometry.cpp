#include <gtest/gtest.h>
#include "GravityBilliards/Geometry.hpp"
#include "GravityBilliards/Vector2D.hpp"

using namespace GravityBilliards;

TEST(GeometryTest, CircleCircleIntersection) {
    Vector2D posA(0, 0);
    double radiusA = 5.0;

    // Touching
    Vector2D posB1(10, 0);
    double radiusB1 = 5.0;
    EXPECT_TRUE(Geometry::checkCircleIntersection(posA, radiusA, posB1, radiusB1));

    // Overlapping
    Vector2D posB2(8, 0);
    double radiusB2 = 5.0;
    EXPECT_TRUE(Geometry::checkCircleIntersection(posA, radiusA, posB2, radiusB2));

    // Disjoint
    Vector2D posB3(11, 0);
    double radiusB3 = 5.0;
    EXPECT_FALSE(Geometry::checkCircleIntersection(posA, radiusA, posB3, radiusB3));
}

TEST(GeometryTest, PointInCircle) {
    Vector2D center(10, 10);
    double radius = 5.0;

    EXPECT_TRUE(Geometry::isPointInCircle(Vector2D(10, 10), center, radius)); // center
    EXPECT_TRUE(Geometry::isPointInCircle(Vector2D(15, 10), center, radius)); // edge
    EXPECT_TRUE(Geometry::isPointInCircle(Vector2D(13, 14), center, radius)); // inside (dist = 5)
    EXPECT_FALSE(Geometry::isPointInCircle(Vector2D(15, 15), center, radius)); // outside
}

TEST(GeometryTest, LineSegmentCircleIntersection) {
    Vector2D circleCenter(10, 10);
    double radius = 5.0;

    // Line intersects through the circle
    EXPECT_TRUE(Geometry::checkLineSegmentCircleIntersection(Vector2D(0, 10), Vector2D(20, 10), circleCenter, radius));
    
    // Line stops just short of the circle
    EXPECT_FALSE(Geometry::checkLineSegmentCircleIntersection(Vector2D(0, 10), Vector2D(4, 10), circleCenter, radius));

    // Line starts and ends inside the circle
    EXPECT_TRUE(Geometry::checkLineSegmentCircleIntersection(Vector2D(9, 9), Vector2D(11, 11), circleCenter, radius));

    // Tangent line
    EXPECT_TRUE(Geometry::checkLineSegmentCircleIntersection(Vector2D(5, 0), Vector2D(5, 20), circleCenter, radius));

    // Line completely misses
    EXPECT_FALSE(Geometry::checkLineSegmentCircleIntersection(Vector2D(0, 0), Vector2D(20, 0), circleCenter, radius));
}

TEST(GeometryTest, AABBCircleIntersection) {
    Vector2D circleCenter(10, 10);
    double radius = 5.0;

    Vector2D boxMin(0, 0);
    Vector2D boxMax(6, 6); // Corner is at (6,6). Dist to (10,10) is sqrt(16+16)=sqrt(32)=5.65 > 5. Should not intersect.
    EXPECT_FALSE(Geometry::checkAABBCircleIntersection(boxMin, boxMax, circleCenter, radius));

    Vector2D boxMax2(7, 7); // Corner is at (7,7). Dist to (10,10) is sqrt(9+9)=sqrt(18)=4.24 <= 5. Should intersect.
    EXPECT_TRUE(Geometry::checkAABBCircleIntersection(boxMin, boxMax2, circleCenter, radius));

    // Circle inside box
    Vector2D boxMin3(0, 0);
    Vector2D boxMax3(20, 20);
    EXPECT_TRUE(Geometry::checkAABBCircleIntersection(boxMin3, boxMax3, circleCenter, radius));
}

TEST(GeometryTest, PointLineSegmentDistanceSquared) {
    Vector2D lineStart(10.0, 10.0);
    Vector2D lineEnd(20.0, 10.0);

    // 1. Point exactly on the line
    Vector2D p1(15.0, 10.0);
    EXPECT_DOUBLE_EQ(Geometry::pointLineSegmentDistanceSquared(p1, lineStart, lineEnd), 0.0);

    // 2. Point perpendicularly above the line (distance 5.0 -> squared 25.0)
    Vector2D p2(15.0, 15.0);
    EXPECT_DOUBLE_EQ(Geometry::pointLineSegmentDistanceSquared(p2, lineStart, lineEnd), 25.0);

    // 3. Point past the end of the line segment
    // Closest point on segment is lineEnd (20.0, 10.0). Point is (25.0, 10.0). Dist = 5.0 -> sq 25.0
    Vector2D p3(25.0, 10.0);
    EXPECT_DOUBLE_EQ(Geometry::pointLineSegmentDistanceSquared(p3, lineStart, lineEnd), 25.0);

    // 4. Point before the start of the segment
    // Closest point is lineStart (10.0, 10.0). Point is (5.0, 10.0). Dist = 5.0 -> sq 25.0
    Vector2D p4(5.0, 10.0);
    EXPECT_DOUBLE_EQ(Geometry::pointLineSegmentDistanceSquared(p4, lineStart, lineEnd), 25.0);
    
    // 5. Zero-length line segment (point)
    // Closest point is lineStart (10.0, 10.0). Point is (10.0, 15.0). Dist = 5.0 -> sq 25.0
    Vector2D p5(10.0, 15.0);
    EXPECT_DOUBLE_EQ(Geometry::pointLineSegmentDistanceSquared(p5, lineStart, lineStart), 25.0);
}
