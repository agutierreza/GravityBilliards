#include <gtest/gtest.h>
#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Trajectories.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include <cmath>

using namespace GravityBilliards;

TEST(WorldTest, AdvanceMobileAttractor) {
    World world;
    
    // Parent attractor at origin, massive
    Attractor parent(1000.0, 10.0, Vector2D(0.0, 0.0));
    world.attractors.push_back(parent); // Index 0
    
    // Mobile attractor orbiting the parent
    Attractor satellite(1.0, 2.0, Vector2D(10.0, 0.0));
    satellite.parentAttractorIndex = 0;
    
    // Setup a circular orbit: a=10, e=0
    KeplerOrbit orbit;
    orbit.a = 10.0;
    orbit.e = 0.0;
    orbit.omega = 0.0;
    orbit.M0 = 0.0;
    satellite.trajectory = orbit;
    
    world.attractors.push_back(satellite); // Index 1
    
    // n = sqrt(1000 / 1000) = 1.0 rad/sec
    // Period = 2*pi seconds
    double period = Trajectories::getOrbitalPeriod(orbit, parent.mass);
    EXPECT_DOUBLE_EQ(period, 2.0 * M_PI);
    
    // Advance by 1/4 of a period (pi/2 seconds). 
    // It should move from (10, 0) to (0, 10)
    world.advance(M_PI / 2.0);
    
    EXPECT_NEAR(world.attractors[1].position.x, 0.0, 1e-9);
    EXPECT_NEAR(world.attractors[1].position.y, 10.0, 1e-9);
    
    // Advance to full period. Should be back at (10, 0)
    world.advance(period);
    EXPECT_NEAR(world.attractors[1].position.x, 10.0, 1e-9);
    EXPECT_NEAR(world.attractors[1].position.y, 0.0, 1e-9);
}
