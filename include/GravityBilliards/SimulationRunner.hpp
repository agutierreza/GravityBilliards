#pragma once

#include "GravityBilliards/ParticleSimulator.hpp"
#include "GravityBilliards/InputProvider.hpp"
#include "GravityBilliards/GameCommand.hpp"
#include <memory>
#include <optional>

namespace GravityBilliards {

/**
 * @class SimulationRunner
 * @brief Orchestrates real-world time pacing and bridges peripheral inputs to the ParticleSimulator.
 * 
 * Uses an accumulator pattern to ensure deterministic physics steps regardless
 * of variable real-world frame rates or lags.
 */
template <typename TIntegrator, typename TDetector, typename TResolver>
class SimulationRunner {
private:
    ParticleSimulator<TIntegrator, TDetector, TResolver> m_simulator;
    std::shared_ptr<InputProvider> m_inputProvider;
    double m_accumulator = 0.0;

public:
    /**
     * @brief Constructs a new SimulationRunner.
     * @param loop The underlying ParticleSimulator instance.
     * @param input An optional InputProvider to poll for user inputs.
     */
    SimulationRunner(ParticleSimulator<TIntegrator, TDetector, TResolver> loop, std::shared_ptr<InputProvider> input = nullptr)
        : m_simulator(std::move(loop)), m_inputProvider(std::move(input)) {}

    /**
     * @brief Updates the simulation state based on elapsed real-world time.
     * 
     * Adds the elapsed time to an accumulator and consumes it in discrete steps
     * matching the ParticleSimulator's configured dt. Before each step, the InputProvider
     * is polled to dynamically enqueue ApplyForceCommand inputs.
     * 
     * @param realWorldDeltaTime The time elapsed since the last update, in the same units as dt.
     */
    void update(double realWorldDeltaTime) {
        m_accumulator += realWorldDeltaTime;
        double dt = m_simulator.getEngine().dt;

        while (m_accumulator >= dt) {
            // 1. Poll the input provider and queue commands dynamically
            if (m_inputProvider) {
                std::optional<Vector2D> force = m_inputProvider->getForceInput();
                if (force.has_value()) {
                    m_simulator.queueCommand(std::make_unique<ApplyForceCommand>(*force));
                }
            }

            // 2. Step the internal game loop forward
            m_simulator.step();

            // 3. Consume time from the accumulator
            m_accumulator -= dt;
        }
    }
    
    /**
     * @brief Retrieves the underlying ParticleSimulator instance.
     * @return A reference to the ParticleSimulator.
     */
    ParticleSimulator<TIntegrator, TDetector, TResolver>& getParticleSimulator() { return m_simulator; }

    /**
     * @brief Retrieves the underlying ParticleSimulator instance.
     * @return A const reference to the ParticleSimulator.
     */
    const ParticleSimulator<TIntegrator, TDetector, TResolver>& getParticleSimulator() const { return m_simulator; }

    /**
     * @brief Gets the remaining time in the accumulator.
     * @return The unconsumed time in the accumulator.
     */
    double getAccumulator() const { return m_accumulator; }
};

} // namespace GravityBilliards
