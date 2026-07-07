#include <gtest/gtest.h>
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include "GravityBilliards/Topology.hpp"

using namespace GravityBilliards;

class CollisionDetectorTest : public ::testing::Test {
protected:
    CircleCollisionDetector detector;
    // Set up a basic world with zero gravity so we isolate collision logic
    World world{0, 1000, 1000}; 
    EuclideanTopology topology;

    void SetUp() override {
        // Clear randomly generated attractors
        world.attractors.clear();
        
        // Add specific attractors for testing
        // Attractor 1: center (0,0), radius 10
        // Attractor 1: center (0,0), radius 10, mass 100.0
        world.attractors.push_back(Attractor(100.0, 10.0, Vector2D(0.0, 0.0)));
        // Attractor 2: center (100,100), radius 20, mass 100.0
        world.attractors.push_back(Attractor(100.0, 20.0, Vector2D(100.0, 100.0)));
    }
};

TEST_F(CollisionDetectorTest, DefiniteMiss) {
    Vector2D pos(50.0, 50.0);
    double radius = 5.0;

    auto result = detector.checkCollision(world, pos, radius, topology);
    EXPECT_FALSE(result.has_value());
}

TEST_F(CollisionDetectorTest, DefiniteHit) {
    Vector2D pos(5.0, 0.0);
    double radius = 6.0;

    auto result = detector.checkCollision(world, pos, radius, topology);
    ASSERT_TRUE(result.has_value());
    
    // Normal should point from attractor center to particle center
    // Attractor (0,0) -> Particle (5,0) => Normal (1,0)
    EXPECT_DOUBLE_EQ(result->normal.x, 1.0);
    EXPECT_DOUBLE_EQ(result->normal.y, 0.0);
}

TEST_F(CollisionDetectorTest, ExactEdgeTouch) {
    Vector2D pos(15.0, 0.0);
    double radius = 5.0;

    auto result = detector.checkCollision(world, pos, radius, topology);
    EXPECT_FALSE(result.has_value()); // Current baseline uses strictly <
}

TEST_F(CollisionDetectorTest, AngledHit) {
    Vector2D pos(100.0, 125.0);
    double radius = 10.0;

    auto result = detector.checkCollision(world, pos, radius, topology);
    ASSERT_TRUE(result.has_value());
    
    EXPECT_DOUBLE_EQ(result->normal.x, 0.0);
    EXPECT_DOUBLE_EQ(result->normal.y, 1.0);
}
