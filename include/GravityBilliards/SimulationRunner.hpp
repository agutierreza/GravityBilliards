#pragma once

#include "GravityBilliards/GameLoop.hpp"
#include "GravityBilliards/InputProvider.hpp"
#include "GravityBilliards/GameCommand.hpp"
#include <memory>
#include <optional>

namespace GravityBilliards {

/**
 * @class SimulationRunner
 * @brief Orchestrates real-world time pacing and bridges peripheral inputs to the GameLoop.
 * 
 * Uses an accumulator pattern to ensure deterministic physics steps regardless
 * of variable real-world frame rates or lags.
 */
template <typename TIntegrator, typename TDetector, typename TResolver>
class SimulationRunner {
private:
    GameLoop<TIntegrator, TDetector, TResolver> m_gameLoop;
    std::shared_ptr<InputProvider> m_inputProvider;
    double m_accumulator = 0.0;

public:
    /**
     * @brief Constructs a new SimulationRunner.
     * @param loop The underlying GameLoop instance.
     * @param input An optional InputProvider to poll for user inputs.
     */
    SimulationRunner(GameLoop<TIntegrator, TDetector, TResolver> loop, std::shared_ptr<InputProvider> input = nullptr)
        : m_gameLoop(std::move(loop)), m_inputProvider(std::move(input)) {}

    /**
     * @brief Updates the simulation state based on elapsed real-world time.
     * 
     * Adds the elapsed time to an accumulator and consumes it in discrete steps
     * matching the GameLoop's configured dt. Before each step, the InputProvider
     * is polled to dynamically enqueue ApplyForceCommand inputs.
     * 
     * @param realWorldDeltaTime The time elapsed since the last update, in the same units as dt.
     */
    void update(double realWorldDeltaTime) {
        m_accumulator += realWorldDeltaTime;
        double dt = m_gameLoop.getEngine().dt;

        while (m_accumulator >= dt) {
            // 1. Poll the input provider and queue commands dynamically
            if (m_inputProvider) {
                std::optional<Vector2D> force = m_inputProvider->getForceInput();
                if (force.has_value()) {
                    m_gameLoop.queueCommand(std::make_unique<ApplyForceCommand>(*force));
                }
            }

            // 2. Step the internal game loop forward
            m_gameLoop.step();

            // 3. Consume time from the accumulator
            m_accumulator -= dt;
        }
    }
    
    /**
     * @brief Retrieves the underlying GameLoop instance.
     * @return A reference to the GameLoop.
     */
    GameLoop<TIntegrator, TDetector, TResolver>& getGameLoop() { return m_gameLoop; }

    /**
     * @brief Retrieves the underlying GameLoop instance.
     * @return A const reference to the GameLoop.
     */
    const GameLoop<TIntegrator, TDetector, TResolver>& getGameLoop() const { return m_gameLoop; }

    /**
     * @brief Gets the remaining time in the accumulator.
     * @return The unconsumed time in the accumulator.
     */
    double getAccumulator() const { return m_accumulator; }
};

} // namespace GravityBilliards
