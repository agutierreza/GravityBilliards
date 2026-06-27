#include <gtest/gtest.h>
#include "GravityBilliards/GameLoop.hpp"
#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GravityBilliards/InitialConditions.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using namespace GravityBilliards;

namespace {

InitialConditions LoadFixture(const std::string& filename) {
    std::ifstream f("../test/fixtures/" + filename);
    if (!f.is_open()) throw std::runtime_error("Could not open fixture: " + filename);
    nlohmann::json j;
    f >> j;
    return InitialConditions::fromJson(j);
}

} // namespace

class GameLoopTest : public ::testing::Test {
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

    void verifyTraceMatch(GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver>& loop, 
                          const std::vector<TracePoint>& trace, int startOffset = 0) {
        // Assert that the current game loop state matches the beginning of the trace (before step)
        for (size_t i = 0; i < trace.size() - 1; ++i) { // Ignore the very last point since we step from i to i+1
            // Check state before step
            EXPECT_NEAR(loop.getPosition().x, trace[i].position.x, 1e-9) << "Mismatch at step " << i + startOffset;
            EXPECT_NEAR(loop.getPosition().y, trace[i].position.y, 1e-9) << "Mismatch at step " << i + startOffset;
            EXPECT_NEAR(loop.getVelocity().x, trace[i].velocity.x, 1e-9) << "Mismatch at step " << i + startOffset;
            EXPECT_NEAR(loop.getVelocity().y, trace[i].velocity.y, 1e-9) << "Mismatch at step " << i + startOffset;
            EXPECT_NEAR(loop.getCurrentTime(), trace[i].time, 1e-9) << "Time mismatch at step " << i + startOffset;
            
            // Advance by one tick
            loop.step();
        }
        
        // Check final state
        size_t finalIdx = trace.size() - 1;
        EXPECT_NEAR(loop.getPosition().x, trace[finalIdx].position.x, 1e-9) << "Final position X mismatch";
        EXPECT_NEAR(loop.getPosition().y, trace[finalIdx].position.y, 1e-9) << "Final position Y mismatch";
        EXPECT_NEAR(loop.getVelocity().x, trace[finalIdx].velocity.x, 1e-9) << "Final velocity X mismatch";
        EXPECT_NEAR(loop.getVelocity().y, trace[finalIdx].velocity.y, 1e-9) << "Final velocity Y mismatch";
        EXPECT_NEAR(loop.getCurrentTime(), trace[finalIdx].time, 1e-9) << "Final time mismatch";
    }
};

TEST_F(GameLoopTest, CircularOrbit) {
    InitialConditions ic = LoadFixture("circular_orbit.json");
    ic.particleRadius = 5.0; // match test setup
    
    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 1000.0);
    ASSERT_EQ(trace.size(), 1001);

    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;
    loop.getEngine().dt = 1.0;
    
    verifyTraceMatch(loop, trace);
}

TEST_F(GameLoopTest, EllipticalOrbitDriftBaseline) {
    InitialConditions ic = LoadFixture("elliptical_drift.json");
    ic.particleRadius = 5.0;

    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 500.0);
    ASSERT_EQ(trace.size(), 501);

    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;
    
    verifyTraceMatch(loop, trace);
}

TEST_F(GameLoopTest, TwoPlanetEquilibrium) {
    InitialConditions ic = LoadFixture("two_planet_equilibrium.json");
    ic.particleRadius = 5.0;

    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 1000.0);
    
    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;
    
    verifyTraceMatch(loop, trace);
}

TEST_F(GameLoopTest, Figure8Orbit) {
    InitialConditions ic = LoadFixture("figure_8_orbit.json");
    ic.particleRadius = 5.0;

    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 5000.0);

    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;

    verifyTraceMatch(loop, trace);
}

TEST_F(GameLoopTest, PrecessingFigure8Orbit) {
    InitialConditions ic = LoadFixture("precessing_figure_8.json");
    ic.particleRadius = 5.0;

    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 5000.0);

    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;

    verifyTraceMatch(loop, trace);
}

TEST_F(GameLoopTest, SubOrbitalHop) {
    InitialConditions ic = LoadFixture("suborbital_hop.json");
    ic.particleRadius = 5.0;

    // Simulate for 5000 frames (it will stop before that)
    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 5000.0);

    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;
    
    verifyTraceMatch(loop, trace);
    EXPECT_TRUE(loop.isStopped());
}

TEST_F(GameLoopTest, BarelyEscaping) {
    InitialConditions ic = LoadFixture("barely_escaping.json");
    ic.particleRadius = 5.0;

    auto trace = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, 5000.0);

    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;

    verifyTraceMatch(loop, trace);
}

TEST_F(GameLoopTest, OrbitalInsertionBurn) {
    InitialConditions ic = LoadFixture("orbital_insertion_burn.json");
    ic.particleRadius = 5.0;
    
    // Load maneuvers JSON manually
    struct Maneuver { double frame; Vector2D deltaV; };
    std::vector<Maneuver> maneuvers;
    std::ifstream f("../test/fixtures/orbital_insertion_burn_maneuvers.json");
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
    
    // Generate Stage 1 trace (coast to maneuver)
    double legDuration = maneuvers[0].frame; // 100.0
    auto trace1 = engine.getTrace(ic.world, ic.particleStartPos, ic.particleStartVel, 0.0, legDuration);
    ASSERT_EQ(trace1.size(), 101); // 0 to 100
    
    // Apply Burn
    Vector2D currentPos = trace1.back().position;
    Vector2D currentVel = trace1.back().velocity + maneuvers[0].deltaV;
    
    // Generate Stage 2 trace (Orbit Insertion)
    auto trace2 = engine.getTrace(ic.world, currentPos, currentVel, 0.0, 2000.0);
    // Note: trace2 time will range from 0.0 to 2000.0, but in the game loop it will be from 100.0 to 2100.0
    // So we manually adjust trace2 times to match GameLoop timeline
    for (auto& t : trace2) {
        t.time += maneuvers[0].frame;
    }
    
    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
    loop.getEngine().bounceDamping = 0.8;
    
    // 1. Verify Stage 1
    verifyTraceMatch(loop, trace1);
    
    // 2. Queue command at exactly frame 100 (time 100.0)
    EXPECT_NEAR(loop.getCurrentTime(), maneuvers[0].frame, 1e-9);
    loop.queueCommand(std::make_unique<ApplyThrustCommand>(maneuvers[0].deltaV));
    
    // Call step once to execute the command and the first tick of stage 2
    loop.step();
    
    // Now loop is at time 101.0, which corresponds to trace2[1]
    // Let's create a sub-trace of trace2 starting from index 1
    std::vector<TracePoint> trace2_sub(trace2.begin() + 1, trace2.end());
    
    // 3. Verify Stage 2 (starting at offset 101)
    verifyTraceMatch(loop, trace2_sub, 101);
}
