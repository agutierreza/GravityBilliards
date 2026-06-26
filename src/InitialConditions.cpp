#include "GravityBilliards/InitialConditions.hpp"

namespace GravityBilliards {

InitialConditions InitialConditions::fromJson(const nlohmann::json& j) {
    InitialConditions s;
    if (j.contains("attractors")) {
        for (const auto& p : j["attractors"]) {
            Attractor attractor(
                p["mass"].get<int>(), 
                Vector2D(p["position"]["x"].get<double>(), p["position"]["y"].get<double>())
            );
            if (p.contains("radius")) {
                attractor.radius = p["radius"].get<double>();
            }
            s.world.attractors.push_back(attractor);
        }
    }
    
    if (j.contains("particle")) {
        s.particleStartPos = Vector2D(
            j["particle"]["startPosition"]["x"].get<double>(), 
            j["particle"]["startPosition"]["y"].get<double>()
        );
        s.particleStartVel = Vector2D(
            j["particle"]["startVelocity"]["x"].get<double>(), 
            j["particle"]["startVelocity"]["y"].get<double>()
        );
    }
    
    if (j.contains("simulation")) {
        if (j["simulation"].contains("stopVelocityThreshold")) {
            s.stopVelocityThreshold = j["simulation"]["stopVelocityThreshold"].get<double>();
        }
        if (j["simulation"].contains("particleRadius")) {
            s.particleRadius = j["simulation"]["particleRadius"].get<double>();
        }
    }
    return s;
}

nlohmann::json InitialConditions::toJson() const {
    nlohmann::json j;
    
    j["simulation"]["stopVelocityThreshold"] = stopVelocityThreshold;
    j["simulation"]["particleRadius"] = particleRadius;
    
    j["particle"]["startPosition"] = {{"x", particleStartPos.x}, {"y", particleStartPos.y}};
    j["particle"]["startVelocity"] = {{"x", particleStartVel.x}, {"y", particleStartVel.y}};
    
    j["attractors"] = nlohmann::json::array();
    for (const auto& p : world.attractors) {
        j["attractors"].push_back({
            {"position", {{"x", p.position.x}, {"y", p.position.y}}},
            {"mass", p.mass},
            {"radius", p.radius}
        });
    }
    
    return j;
}

} // namespace GravityBilliards
