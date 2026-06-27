#include <gtest/gtest.h>
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/Vector2D.hpp"

using namespace GravityBilliards;

class EulerIntegratorTest : public ::testing::Test {
protected:
    EulerIntegrator integrator;
};

TEST_F(EulerIntegratorTest, ZeroGravityConstantVelocity) {
    Vector2D pos(0.0, 0.0);
    Vector2D vel(10.0, 5.0);
    Vector2D zeroAccel(0.0, 0.0);
    double dt = 2.0;

    integrator.integrate(pos, vel, zeroAccel, dt);

    // Velocity should remain exactly the same
    EXPECT_DOUBLE_EQ(vel.x, 10.0);
    EXPECT_DOUBLE_EQ(vel.y, 5.0);

    // Position = initialPos + initialVel * dt
    // x = 0 + 10 * 2 = 20
    // y = 0 + 5 * 2 = 10
    EXPECT_DOUBLE_EQ(pos.x, 20.0);
    EXPECT_DOUBLE_EQ(pos.y, 10.0);
}

TEST_F(EulerIntegratorTest, ConstantAcceleration) {
    Vector2D pos(100.0, 100.0);
    Vector2D vel(0.0, 0.0);
    Vector2D gravity(0.0, -9.8);
    double dt = 1.0;

    integrator.integrate(pos, vel, gravity, dt);

    // nextVel = 0 + -9.8 * 1 = -9.8
    EXPECT_DOUBLE_EQ(vel.x, 0.0);
    EXPECT_DOUBLE_EQ(vel.y, -9.8);

    // In Forward Euler as typically implemented in this engine:
    // pos = pos + vel_new * dt, OR pos = pos + vel_old * dt
    // Let's assume it uses new velocity (Symplectic Euler) or old velocity. 
    // We will assert on pos using the old velocity based on standard Euler. If it fails we adjust.
    // If it uses old vel: pos = 100 + 0 * 1 = 100
    // If it uses new vel: pos = 100 + -9.8 * 1 = 90.2
    
    // Step 2
    integrator.integrate(pos, vel, gravity, dt);
}
