#include <gtest/gtest.h>
#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GravityBilliards/EnergyDiagnostics.hpp"
#include <cmath>

#include "GravityBilliards/InitialConditions.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace GravityBilliards;

// Helper function to load fixture
InitialConditions LoadFixture(const std::string& filename) {
    std::ifstream f("test/fixtures/" + filename);
    if (!f.is_open()) throw std::runtime_error("Could not open fixture: " + filename);
    nlohmann::json j;
    f >> j;
    return InitialConditions::fromJson(j);
}

class EndToEndTest : public ::testing::Test {
protected:
    EulerIntegrator integrator;
    CircleCollisionDetector detector;
    InelasticCollisionResolver resolver;
    PhysicsEngine<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> engine{integrator, detector, resolver};

    void SetUp() override {
        engine.particleRadius = 5.0;
        engine.stopVelocityThreshold = 0.01;
        engine.dt = 1.0;
        engine.bounceDamping = 0.8;
    }
};

TEST_F(EndToEndTest, CircularOrbit) {
    InitialConditions ic = LoadFixture("circular_orbit.json");
    
    // Simulate for 100 seconds
    auto result = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 100.0);
    EXPECT_FALSE(result.stopped);
    
    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 1000.0);
    ASSERT_EQ(trace.size(), 1001);

    for (const auto& t : trace) {
        double r = t.position.magnitude();
        EXPECT_GT(r, 20.0);
        EXPECT_LT(r, 200.0);
    }
}

TEST_F(EndToEndTest, EllipticalOrbitDriftBaseline) {
    InitialConditions ic = LoadFixture("elliptical_drift.json");

    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 500.0);
    auto timeline = EnergyDiagnostics::analyse(trace, ic.world);

    double minPE = 0.0;
    double teAtPeriapsis = 0.0;
    for (const auto& snap : timeline.snapshots) {
        if (snap.potentialEnergy < minPE) {
            minPE = snap.potentialEnergy;
            teAtPeriapsis = snap.totalEnergy;
        }
    }

    // BASELINE: With Euler integrator, the energy at periapsis dips.
    // We expect it to be around -8.5 or lower due to Euler drift.
    // Setting a wide margin just to ensure it's deterministic.
    EXPECT_NEAR(teAtPeriapsis, -10.0, 5.0); 
}

TEST_F(EndToEndTest, TwoPlanetEquilibrium) {
    InitialConditions ic = LoadFixture("two_planet_equilibrium.json");

    auto result = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 100.0);
    
    auto longResult = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 1000.0);
    EXPECT_TRUE(longResult.stopped); 
}

TEST_F(EndToEndTest, Figure8Orbit) {
    InitialConditions ic = LoadFixture("figure_8_orbit.json");

    // This is a stable repeating orbit, so it should NOT stop
    auto result = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 5000.0);
    EXPECT_FALSE(result.stopped);
}

TEST_F(EndToEndTest, PrecessingFigure8Orbit) {
    InitialConditions ic = LoadFixture("precessing_figure_8.json");

    // This quasi-periodic orbit also never crashes or escapes within 5000 frames
    auto result = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 5000.0);
    EXPECT_FALSE(result.stopped);
}

TEST_F(EndToEndTest, SubOrbitalHop) {
    InitialConditions ic = LoadFixture("suborbital_hop.json");

    auto result = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 5000.0);
    
    EXPECT_TRUE(result.stopped);
    EXPECT_LT(result.timeElapsed, 5000.0);
}

TEST_F(EndToEndTest, BarelyEscaping) {
    InitialConditions ic = LoadFixture("barely_escaping.json");

    auto result = engine.simulateUntilStop(ic.world, ic.particleStartPos, ic.particleStartVel, 5000.0);
    
    EXPECT_FALSE(result.stopped);
}

TEST_F(EndToEndTest, OrbitalInsertionBurn) {
    InitialConditions ic = LoadFixture("orbital_insertion_burn.json");
    
    // Define Maneuver locally for the test
    struct Maneuver { double frame; Vector2D deltaV; };
    std::vector<Maneuver> maneuvers;
    
    // Load maneuvers JSON manually
    std::ifstream f("test/fixtures/orbital_insertion_burn_maneuvers.json");
    if (f.is_open()) {
        nlohmann::json j;
        f >> j;
        for (const auto& m : j) {
            Maneuver man;
            man.frame = m["frame"].get<double>();
            man.deltaV = Vector2D(m["deltaV"]["x"].get<double>(), m["deltaV"]["y"].get<double>());
            maneuvers.push_back(man);
        }
    }
    
    ASSERT_EQ(maneuvers.size(), 1);
    
    // Trace-Stitching Logic
    Vector2D currentPos = ic.particleStartPos;
    Vector2D currentVel = ic.particleStartVel;
    double currentTime = 0.0;
    
    // Stage 1: Coast to maneuver
    double legDuration = maneuvers[0].frame - currentTime;
    auto trace1 = engine.getTrace(ic.world, currentPos, currentVel, 0.0, legDuration);
    ASSERT_EQ(trace1.size(), 101); // 0 to 100
    
    currentPos = trace1.back().position;
    currentVel = trace1.back().velocity;
    currentTime = maneuvers[0].frame;
    
    // Apply Burn
    currentVel += maneuvers[0].deltaV;
    
    // Stage 2: Orbit Insertion
    auto burnResult = engine.simulateUntilStop(ic.world, currentPos, currentVel, 2000.0);
    EXPECT_FALSE(burnResult.stopped); // Perfect orbit doesn't stop
}
