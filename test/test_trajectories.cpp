#include <gtest/gtest.h>
#include "GravityBilliards/Trajectories.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include <cmath>

using namespace GravityBilliards;

TEST(TrajectoriesTest, KinematicTrajectoryExactValues) {
    KinematicTrajectory traj;
    traj.initialPos = Vector2D(10.0, 5.0);
    traj.initialVel = Vector2D(2.0, -1.0);
    traj.acceleration = Vector2D(0.0, -9.8);
    traj.startTime = 0.0;

    // We hardcode the exact expected values mathematically calculated beforehand!
    // At t = 1.0:
    // px = 10.0 + 2.0(1) + 0 = 12.0
    // py = 5.0 - 1.0(1) - 4.9(1^2) = -0.9
    Vector2D p1 = Trajectories::solveKinematicTrajectory(traj, 1.0);
    EXPECT_DOUBLE_EQ(p1.x, 12.0);
    EXPECT_DOUBLE_EQ(p1.y, -0.9);

    // At t = 2.0:
    // px = 10.0 + 2.0(2) + 0 = 14.0
    // py = 5.0 - 1.0(2) - 4.9(2^2) = 3.0 - 19.6 = -16.6
    Vector2D p2 = Trajectories::solveKinematicTrajectory(traj, 2.0);
    EXPECT_DOUBLE_EQ(p2.x, 14.0);
    EXPECT_DOUBLE_EQ(p2.y, -16.6);
}

TEST(TrajectoriesTest, KeplerOrbitExactValues) {
    KeplerOrbit orbit;
    orbit.a = 10.0;
    orbit.e = 0.0; // Perfect circle
    orbit.omega = 0.0;
    orbit.M0 = 0.0;

    double parentMass = 1000.0; // This makes n = sqrt(1000 / 1000) = 1.0 rad/sec
    Vector2D parentPos(0.0, 0.0);

    // At t = 0
    Vector2D p0 = Trajectories::solveKeplerOrbit(orbit, parentPos, parentMass, 0.0);
    EXPECT_DOUBLE_EQ(p0.x, 10.0);
    EXPECT_DOUBLE_EQ(p0.y, 0.0);

    // At t = PI/2 (~1.57079632679), n = 1.0 -> M = PI/2.
    // Particle should have rotated 90 degrees CCW to (0, 10)
    double pi_over_2 = 1.5707963267948966;
    Vector2D p1 = Trajectories::solveKeplerOrbit(orbit, parentPos, parentMass, pi_over_2);
    EXPECT_NEAR(p1.x, 0.0, 1e-9);
    EXPECT_NEAR(p1.y, 10.0, 1e-9);

    // Test Orbital Velocity at t = PI/2
    // It is at (0, 10), so it should be moving entirely in the -X direction at speed = 10.0 * n = 10.0
    Vector2D v1 = Trajectories::getOrbitalVelocity(orbit, parentMass, pi_over_2);
    EXPECT_NEAR(v1.x, -10.0, 1e-9);
    EXPECT_NEAR(v1.y, 0.0, 1e-9);
}

TEST(TrajectoriesTest, KeplerDerivedValues) {
    KeplerOrbit orbit;
    orbit.a = 10.0;
    double parentMass = 1000.0; // n = 1.0

    // Period should be 2*PI / n = 2*PI
    double expectedPeriod = 2.0 * 3.14159265358979323846;
    EXPECT_DOUBLE_EQ(Trajectories::getOrbitalPeriod(orbit, parentMass), expectedPeriod);

    // Escape Velocity at distance 10.0 from mass 1000.0
    // v_esc = sqrt(2 * M / r) = sqrt(2000 / 10) = sqrt(200) = 14.142135623730951
    double expectedEsc = 14.142135623730950488;
    EXPECT_NEAR(Trajectories::getEscapeVelocity(parentMass, 10.0), expectedEsc, 1e-9);
}
