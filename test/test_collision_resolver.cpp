#include <gtest/gtest.h>
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GravityBilliards/CollisionTypes.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include <cmath>

using namespace GravityBilliards;

class CollisionResolverTest : public ::testing::Test {
protected:
    InelasticCollisionResolver resolver;
};

TEST_F(CollisionResolverTest, NormalReflection) {
    Vector2D pos(0.0, 0.0);
    Vector2D vel(0.0, -10.0);
    CollisionInfo info{Vector2D(0.0, 1.0), 0.0, 1.0};
    
    Vector2D accel(0.0, 0.0);
    resolver.resolve(info, pos, vel, accel, 0.8, 1.0);

    EXPECT_DOUBLE_EQ(vel.x, 0.0);
    EXPECT_DOUBLE_EQ(vel.y, 8.0);
}

TEST_F(CollisionResolverTest, AngledReflection) {
    Vector2D pos(0.0, 0.0);
    Vector2D vel(10.0, -10.0);
    CollisionInfo info{Vector2D(0.0, 1.0), 0.0, 1.0};
    
    Vector2D accel(0.0, 0.0);
    resolver.resolve(info, pos, vel, accel, 0.8, 1.0);

    EXPECT_DOUBLE_EQ(vel.x, 8.0); 
    EXPECT_DOUBLE_EQ(vel.y, 8.0);
}

TEST_F(CollisionResolverTest, ZeroVelocity) {
    Vector2D pos(0.0, 0.0);
    Vector2D vel(0.0, 0.0);
    CollisionInfo info{Vector2D(1.0, 0.0), 0.0, 1.0};
    
    Vector2D accel(0.0, 0.0);
    resolver.resolve(info, pos, vel, accel, 0.8, 1.0);

    EXPECT_DOUBLE_EQ(vel.x, 0.0);
    EXPECT_DOUBLE_EQ(vel.y, 0.0);
}

TEST_F(CollisionResolverTest, GlancingBlow) {
    Vector2D pos(0.0, 0.0);
    Vector2D vel(10.0, -1.0);
    CollisionInfo info{Vector2D(0.0, 1.0), 0.0, 1.0};
    
    Vector2D accel(0.0, 0.0);
    resolver.resolve(info, pos, vel, accel, 0.8, 1.0);

    EXPECT_DOUBLE_EQ(vel.x, 8.0); 
    EXPECT_DOUBLE_EQ(vel.y, 0.8);
}
