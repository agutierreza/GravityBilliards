#include <gtest/gtest.h>
#include "GravityBilliards/Topology.hpp"

using namespace GravityBilliards;

TEST(TopologyTest, ToroidalShortestDirection) {
    ToroidalTopology top(100.0, 100.0);

    Vector2D p1(10.0, 10.0);
    Vector2D p2(90.0, 10.0);
    
    // Manual Math:
    // W = 100, H = 100.
    // p1 = (10, 10), p2 = (90, 10)
    // Distance from p1 to p2 going right: 90 - 10 = 80.
    // Distance from p1 to p2 going left (wrapping): from 10 to 0 is 10, from 100 to 90 is 10. Total 20 left.
    // Shortest path is going left by 20.
    // Direction vector dx = -20, dy = 0.
    
    Vector2D dir = top.getShortestDirection(p1, p2);
    EXPECT_DOUBLE_EQ(dir.x, -20.0);
    EXPECT_DOUBLE_EQ(dir.y, 0.0);
    
    // Test Y wrapping
    Vector2D p3(50.0, 5.0);
    Vector2D p4(50.0, 85.0);
    
    // Manual Math:
    // Distance down: 85 - 5 = 80.
    // Distance up (wrapping): 5 to 0 is 5, 100 to 85 is 15. Total 20 up.
    // Shortest path is going up by 20.
    // Direction vector dx = 0, dy = -20.
    dir = top.getShortestDirection(p3, p4);
    EXPECT_DOUBLE_EQ(dir.x, 0.0);
    EXPECT_DOUBLE_EQ(dir.y, -20.0);
}

TEST(TopologyTest, ToroidalWrapping) {
    ToroidalTopology top(100.0, 100.0);

    Vector2D p1(110.0, -30.0);
    
    // Manual Math:
    // Wrapping x=110.0: 110 % 100 = 10.
    // Wrapping y=-30.0: -30 % 100 = -30. Since < 0, add 100 => 70.
    // Expected wrapped pos: (10, 70).
    
    top.wrapPosition(p1);
    EXPECT_DOUBLE_EQ(p1.x, 10.0);
    EXPECT_DOUBLE_EQ(p1.y, 70.0);
}
