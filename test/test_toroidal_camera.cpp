#include <gtest/gtest.h>
#include "Graphics/ToroidalCamera.hpp"

using namespace Graphics;
using namespace GravityBilliards;

class ToroidalCameraTest : public ::testing::Test {
protected:
    double W = 1000.0;
    double H = 1000.0;
    double VW = 800.0; // Viewport width
    double VH = 600.0; // Viewport height
    ToroidalCamera camera{W, H, VW, VH};
};

TEST_F(ToroidalCameraTest, BasicTrackingNoWrapping) {
    camera.position = {500.0, 500.0};
    
    // Target is slightly to the right
    Vector2D target{550.0, 500.0};
    camera.trackTarget(target, 1.0); // 1.0 means instant snap
    
    EXPECT_DOUBLE_EQ(camera.position.x, 550.0);
    EXPECT_DOUBLE_EQ(camera.position.y, 500.0);
}

TEST_F(ToroidalCameraTest, TrackingAcrossWrapBoundaryRight) {
    camera.position = {990.0, 500.0};
    
    // Target has wrapped around the right edge and is now at x=10
    Vector2D target{10.0, 500.0};
    
    // If the camera tracks this target properly, it should move rightwards,
    // conceptually crossing 1000 and arriving at 1010 in continuous space.
    camera.trackTarget(target, 1.0);
    
    EXPECT_DOUBLE_EQ(camera.position.x, 1010.0);
    EXPECT_DOUBLE_EQ(camera.position.y, 500.0);
}

TEST_F(ToroidalCameraTest, TrackingAcrossWrapBoundaryLeft) {
    camera.position = {10.0, 500.0};
    
    // Target has wrapped around the left edge and is now at x=990
    Vector2D target{990.0, 500.0};
    
    // The camera should move leftwards into negative continuous space.
    camera.trackTarget(target, 1.0);
    
    EXPECT_DOUBLE_EQ(camera.position.x, -10.0);
    EXPECT_DOUBLE_EQ(camera.position.y, 500.0);
}

TEST_F(ToroidalCameraTest, TrackingWithExtremeContinuousCoordinates) {
    // Camera has tracked the player through multiple wraps and is far away in continuous space
    camera.position = {5010.0, -3010.0}; // equivalent to wrapped (10, 990)
    
    // Player moves to wrapped (990, 10)
    Vector2D target{990.0, 10.0};
    
    camera.trackTarget(target, 1.0);
    
    // Camera should move left in X and up in Y, taking the shortest Toroidal path
    EXPECT_DOUBLE_EQ(camera.position.x, 4990.0);
    EXPECT_DOUBLE_EQ(camera.position.y, -2990.0);
}

TEST_F(ToroidalCameraTest, GetVisibleRenderPositions_CenteredObject) {
    camera.position = {500.0, 500.0};
    
    Vector2D obj{500.0, 500.0};
    auto positions = camera.getVisibleRenderPositions(obj, 10.0);
    
    ASSERT_EQ(positions.size(), 1);
    EXPECT_DOUBLE_EQ(positions[0].x, 500.0);
    EXPECT_DOUBLE_EQ(positions[0].y, 500.0);
}

TEST_F(ToroidalCameraTest, GetVisibleRenderPositions_EdgeOverlapping) {
    camera.position = {500.0, 500.0}; 
    // Wait, if camera is at 500, viewport is 800 wide (from 100 to 900). 
    // An object at 990 won't be seen by this camera normally! 
    // Let's place the camera at x=990.
    camera.position = {990.0, 500.0};
    
    // Object is at x=10 (wrapped around)
    Vector2D obj{10.0, 500.0};
    double radius = 30.0; 
    
    // Camera covers X: 990 - 400 = 590 to 990 + 400 = 1390.
    // The object at 10 should render at continuous x=1010.
    auto positions = camera.getVisibleRenderPositions(obj, radius);
    
    ASSERT_EQ(positions.size(), 1);
    EXPECT_DOUBLE_EQ(positions[0].x, 1010.0);
}

TEST_F(ToroidalCameraTest, GetVisibleRenderPositions_PhantomCopies_Corner) {
    // Camera is precisely at the corner (0,0), looking at a large viewport
    camera.position = {0.0, 0.0};
    
    // A huge object at (0,0) with radius 100
    Vector2D obj{0.0, 0.0};
    double radius = 100.0;
    
    auto positions = camera.getVisibleRenderPositions(obj, radius);
    
    // Since it's exactly on the corner and overlaps all 4 continuous quadrants
    // relative to the viewport (which covers -400 to 400), we should see exactly 4 copies!
    // Center at (0,0)
    // Left phantom at (-1000, 0) -> rejected because outside X viewport (-400, 400)
    // Wait, the viewport is [-400, 400]. 
    // The phantoms are at (ix*1000, iy*1000). 
    // So none of the phantoms (-1000, 1000) will be in the viewport [-400, 400]!
    
    ASSERT_EQ(positions.size(), 1);
    EXPECT_DOUBLE_EQ(positions[0].x, 0.0);
    EXPECT_DOUBLE_EQ(positions[0].y, 0.0);
}

TEST_F(ToroidalCameraTest, GetVisibleRenderPositions_PhantomCopies_RealOverlap) {
    // Make viewport bigger than world to see all phantoms
    camera.viewportWidth = 2500.0;
    camera.viewportHeight = 2500.0;
    camera.position = {500.0, 500.0};
    
    Vector2D obj{500.0, 500.0};
    double radius = 10.0;
    
    auto positions = camera.getVisibleRenderPositions(obj, radius);
    
    // We should see the 3x3 grid because viewport covers -750 to 1750
    // The object at 500 will have phantoms at -500, 500, 1500!
    // Since -500 and 1500 are within [-750, 1750], all 9 should be visible.
    EXPECT_EQ(positions.size(), 9);
}

TEST_F(ToroidalCameraTest, GetVisibleRenderPositions_Culled) {
    camera.position = {500.0, 500.0};
    
    // Object at x=10 (way offscreen, not overlapping any edge of the viewport [-400, 400] around camera)
    // Camera is at 500. Viewport is [100, 900].
    // Object is at 10.
    Vector2D obj{10.0, 500.0};
    double radius = 50.0;
    
    auto positions = camera.getVisibleRenderPositions(obj, radius);
    
    // Should be culled completely
    EXPECT_TRUE(positions.empty());
}
