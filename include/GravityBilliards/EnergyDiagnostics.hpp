#pragma once

#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/World.hpp"
#include <vector>

namespace GravityBilliards {

/**
 * @struct EnergySnapshot
 * @brief A single energy measurement at one point in time.
 *
 * Captures the kinetic, potential, and total mechanical energy of the
 * particle at a specific frame in the simulation timeline.
 */
struct EnergySnapshot {
    double time;            ///< The timestamp from the corresponding TracePoint.
    double kineticEnergy;   ///< Kinetic energy: 0.5 * v^2 (unit mass assumed).
    double potentialEnergy; ///< Gravitational potential energy at the particle's position.
    double totalEnergy;     ///< The sum of kinetic and potential energy.
};

/**
 * @struct EnergyTimeline
 * @brief The result of a full energy analysis over a pre-computed trace.
 *
 * Contains per-frame snapshots as well as summary statistics useful for
 * automated diagnostics and conservation checks.
 */
struct EnergyTimeline {
    std::vector<EnergySnapshot> snapshots; ///< Per-frame energy data, parallel to the source trace.

    double minKE;       ///< Minimum kinetic energy across all frames.
    double maxKE;       ///< Maximum kinetic energy across all frames.
    double minPE;       ///< Minimum potential energy across all frames.
    double maxPE;       ///< Maximum potential energy across all frames.
    double minTotal;    ///< Minimum total energy across all frames.
    double maxTotal;    ///< Maximum total energy across all frames.

    double energyDrift; ///< totalEnergy[last] - totalEnergy[first]. Useful for conservation checks.
};

/**
 * @class EnergyDiagnostics
 * @brief Post-processing diagnostic utility that computes energy data from a pre-computed trace.
 *
 * This class does not modify or participate in trace generation. It operates
 * entirely as an opt-in post-processing step, ensuring zero performance
 * impact on the core simulation pipeline.
 */
class EnergyDiagnostics {
public:
    /**
     * @brief Computes an energy timeline from an existing trace and world layout.
     *
     * Iterates over the trace, computing kinetic energy (0.5 * v^2, unit mass),
     * gravitational potential energy, and total mechanical energy at each frame.
     * Also computes summary statistics (min/max/drift).
     *
     * @param trace The pre-computed trace from PhysicsEngine::getTrace().
     * @param world The world layout used to compute gravitational potential.
     * @return An EnergyTimeline containing per-frame snapshots and summary statistics.
     */
    static EnergyTimeline analyse(const std::vector<TracePoint>& trace, const World& world);
};

} // namespace GravityBilliards
