#include "SpaceGolf/Scenario.hpp"

namespace SpaceGolf {

Scenario Scenario::fromJson(const nlohmann::json& j) {
    Scenario s;
    if (j.contains("planets")) {
        for (const auto& p : j["planets"]) {
            Planet planet(
                p["mass"].get<int>(), 
                Vector2D(p["position"]["x"].get<double>(), p["position"]["y"].get<double>())
            );
            if (p.contains("radius")) {
                planet.radius = p["radius"].get<double>();
            }
            s.level.planets.push_back(planet);
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

nlohmann::json Scenario::toJson() const {
    nlohmann::json j;
    
    j["simulation"]["stopVelocityThreshold"] = stopVelocityThreshold;
    j["simulation"]["particleRadius"] = particleRadius;
    
    j["particle"]["startPosition"] = {{"x", particleStartPos.x}, {"y", particleStartPos.y}};
    j["particle"]["startVelocity"] = {{"x", particleStartVel.x}, {"y", particleStartVel.y}};
    
    j["planets"] = nlohmann::json::array();
    for (const auto& p : level.planets) {
        j["planets"].push_back({
            {"position", {{"x", p.position.x}, {"y", p.position.y}}},
            {"mass", p.mass},
            {"radius", p.radius}
        });
    }
    
    return j;
}

} // namespace SpaceGolf
