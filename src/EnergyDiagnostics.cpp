#include "GravityBilliards/EnergyDiagnostics.hpp"
#include <limits>

namespace GravityBilliards {

EnergyTimeline EnergyDiagnostics::analyse(const std::vector<TracePoint>& trace, const World& world) {
    EnergyTimeline timeline;

    if (trace.empty()) {
        timeline.minKE = 0.0;
        timeline.maxKE = 0.0;
        timeline.minPE = 0.0;
        timeline.maxPE = 0.0;
        timeline.minTotal = 0.0;
        timeline.maxTotal = 0.0;
        timeline.energyDrift = 0.0;
        return timeline;
    }

    timeline.snapshots.reserve(trace.size());
    timeline.minKE    =  std::numeric_limits<double>::max();
    timeline.maxKE    = -std::numeric_limits<double>::max();
    timeline.minPE    =  std::numeric_limits<double>::max();
    timeline.maxPE    = -std::numeric_limits<double>::max();
    timeline.minTotal =  std::numeric_limits<double>::max();
    timeline.maxTotal = -std::numeric_limits<double>::max();

    for (const auto& point : trace) {
        double ke = 0.5 * point.velocity.magnitudeSquared();
        double pe = world.getGravityPotentialAt(point.position);
        double total = ke + pe;

        timeline.snapshots.push_back({point.time, ke, pe, total});

        if (ke < timeline.minKE)       timeline.minKE = ke;
        if (ke > timeline.maxKE)       timeline.maxKE = ke;
        if (pe < timeline.minPE)       timeline.minPE = pe;
        if (pe > timeline.maxPE)       timeline.maxPE = pe;
        if (total < timeline.minTotal) timeline.minTotal = total;
        if (total > timeline.maxTotal) timeline.maxTotal = total;
    }

    timeline.energyDrift = timeline.snapshots.back().totalEnergy - timeline.snapshots.front().totalEnergy;

    return timeline;
}

} // namespace GravityBilliards
