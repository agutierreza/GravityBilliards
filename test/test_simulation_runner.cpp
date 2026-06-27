#include <gtest/gtest.h>
#include "GravityBilliards/SimulationRunner.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GravityBilliards/InitialConditions.hpp"

using namespace GravityBilliards;

class MockInputProvider : public InputProvider {
public:
    int pollCount = 0;
    std::optional<Vector2D> forceToReturn = std::nullopt;

    std::optional<Vector2D> getForceInput() override {
        pollCount++;
        return forceToReturn;
    }
};

class SimulationRunnerTest : public ::testing::Test {
protected:
    InitialConditions ic;
    std::shared_ptr<MockInputProvider> mockInput;

    void SetUp() override {
        ic.particleStartPos = Vector2D(0, 0);
        ic.particleStartVel = Vector2D(0, 0);
        ic.particleRadius = 5.0;
        ic.stopVelocityThreshold = 0.01;
        mockInput = std::make_shared<MockInputProvider>();
    }
    
    GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> createGameLoop() {
        GameLoop<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> loop(ic);
        loop.getEngine().dt = 1.0;
        return loop;
    }
};

TEST_F(SimulationRunnerTest, AccumulatorLogicExactlyTwoSteps) {
    auto loop = createGameLoop();
    SimulationRunner<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> runner(std::move(loop), mockInput);
    
    // dt is 1.0. We pass 2.0 real world delta time.
    runner.update(2.0);
    
    // It should step exactly 2 times and leave 0.0 in the accumulator
    EXPECT_EQ(mockInput->pollCount, 2);
    EXPECT_NEAR(runner.getAccumulator(), 0.0, 1e-9);
    EXPECT_NEAR(runner.getGameLoop().getCurrentTime(), 2.0, 1e-9);
}

TEST_F(SimulationRunnerTest, AccumulatorLogicPartialStep) {
    auto loop = createGameLoop();
    SimulationRunner<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> runner(std::move(loop), mockInput);
    
    // dt is 1.0. We pass 2.5 real world delta time.
    runner.update(2.5);
    
    // It should step exactly 2 times and leave 0.5 in the accumulator
    EXPECT_EQ(mockInput->pollCount, 2);
    EXPECT_NEAR(runner.getAccumulator(), 0.5, 1e-9);
    EXPECT_NEAR(runner.getGameLoop().getCurrentTime(), 2.0, 1e-9);
    
    // Pass 0.6 more. Accumulator becomes 1.1, so it steps 1 more time and leaves 0.1
    runner.update(0.6);
    EXPECT_EQ(mockInput->pollCount, 3);
    EXPECT_NEAR(runner.getAccumulator(), 0.1, 1e-9);
    EXPECT_NEAR(runner.getGameLoop().getCurrentTime(), 3.0, 1e-9);
}

TEST_F(SimulationRunnerTest, ForceInputAppliedToGameLoop) {
    auto loop = createGameLoop();
    SimulationRunner<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> runner(std::move(loop), mockInput);
    
    // Set mock to return a constant force of (10, 0)
    mockInput->forceToReturn = Vector2D(10.0, 0.0);
    
    // dt is 1.0. One step will apply Force * dt to velocity. Since V0 is (0,0), V1 should be (10,0).
    // Note: The physics step also integrates velocity to position, so P1 will be (10,0) as well since dt=1.0.
    runner.update(1.0);
    
    EXPECT_EQ(mockInput->pollCount, 1);
    EXPECT_NEAR(runner.getGameLoop().getVelocity().x, 10.0, 1e-9);
    EXPECT_NEAR(runner.getGameLoop().getVelocity().y, 0.0, 1e-9);
    
    // Another step of 1.0. Force applies again, V2 becomes (20,0). 
    runner.update(1.0);
    
    EXPECT_EQ(mockInput->pollCount, 2);
    EXPECT_NEAR(runner.getGameLoop().getVelocity().x, 20.0, 1e-9);
    EXPECT_NEAR(runner.getGameLoop().getVelocity().y, 0.0, 1e-9);
}

TEST_F(SimulationRunnerTest, NullInputProviderHandledGracefully) {
    auto loop = createGameLoop();
    // Instantiate runner without an input provider
    SimulationRunner<EulerIntegrator, CircleCollisionDetector, InelasticCollisionResolver> runner(std::move(loop), nullptr);
    
    // Should not crash, just processes the steps normally
    runner.update(3.0);
    
    EXPECT_NEAR(runner.getGameLoop().getCurrentTime(), 3.0, 1e-9);
}
