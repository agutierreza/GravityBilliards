#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <optional>
#include <nlohmann/json.hpp>
#include "GravityBilliards/World.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/InitialConditions.hpp"

using namespace GravityBilliards;

// Simple argument parser helper
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void printHelp() {
    std::cout << "GravityBilliardsCLI Test Tool\n";
    std::cout << "Usage: GravityBilliardsCLI [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --config <file.json>      Load initialConditions from a JSON export file\n";
    std::cout << "  --attractors-random <N>      Number of random attractors to generate (default: 10 if no fixed attractors)\n";
    std::cout << "  --attractor <x,y,mass>       Add a fixed attractor. Can be used multiple times.\n";
    std::cout << "  --pos <x,y>               Starting position of the particle (default: random)\n";
    std::cout << "  --vel <x,y>               Starting velocity of the particle (default: random)\n";
    std::cout << "  --trace-time <start,end>  Time range for the trace (default: 100,1000)\n";
    std::cout << "  --timeout <T>             Max integrator timeout in frames (default: 5000)\n";
    std::cout << "  --help                    Show this message\n";
}

int main(int argc, char* argv[]) {
    std::vector<Attractor> fixedAttractors;
    std::optional<int> randomAttractorsOpt;
    std::optional<Vector2D> startPos;
    std::optional<Vector2D> startVel;
    std::optional<double> customStopThreshold;
    double traceStart = 100.0;
    double traceEnd = 1000.0;
    double timeout = 5000.0;
    double width = 1920.0;
    double height = 1080.0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "--config" && i + 1 < argc) {
            std::string configFile = argv[++i];
            std::ifstream f(configFile);
            if (!f.is_open()) {
                std::cerr << "Failed to open config file: " << configFile << "\n";
                return 1;
            }
            nlohmann::json j;
            try {
                f >> j;
                InitialConditions s = InitialConditions::fromJson(j);
                if (!s.world.attractors.empty()) {
                    fixedAttractors = s.world.attractors;
                }
                if (j.contains("particle")) {
                    startPos = s.particleStartPos;
                    startVel = s.particleStartVel;
                }
                if (j.contains("integrator") && j["integrator"].contains("stopVelocityThreshold")) {
                    customStopThreshold = s.stopVelocityThreshold;
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse JSON config: " << e.what() << "\n";
                return 1;
            }
        } else if (arg == "--attractors-random" && i + 1 < argc) {
            randomAttractorsOpt = std::stoi(argv[++i]);
        } else if (arg == "--attractor" && i + 1 < argc) {
            auto parts = split(argv[++i], ',');
            if (parts.size() == 3) {
                fixedAttractors.emplace_back(std::stoi(parts[2]), Vector2D(std::stod(parts[0]), std::stod(parts[1])));
            }
        } else if (arg == "--pos" && i + 1 < argc) {
            auto parts = split(argv[++i], ',');
            if (parts.size() == 2) startPos = Vector2D(std::stod(parts[0]), std::stod(parts[1]));
        } else if (arg == "--vel" && i + 1 < argc) {
            auto parts = split(argv[++i], ',');
            if (parts.size() == 2) startVel = Vector2D(std::stod(parts[0]), std::stod(parts[1]));
        } else if (arg == "--trace-time" && i + 1 < argc) {
            auto parts = split(argv[++i], ',');
            if (parts.size() == 2) {
                traceStart = std::stod(parts[0]);
                traceEnd = std::stod(parts[1]);
            }
        } else if (arg == "--timeout" && i + 1 < argc) {
            timeout = std::stod(argv[++i]);
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    World world(fixedAttractors);
    int numRandomAttractors = randomAttractorsOpt.value_or(fixedAttractors.empty() ? 10 : 0);

    if (numRandomAttractors > 0) {
        world.addRandomAttractors(numRandomAttractors, width, height);
    }

    if (!startPos) {
        std::uniform_real_distribution<> posDistX(100.0, width - 100.0);
        std::uniform_real_distribution<> posDistY(100.0, height - 100.0);
        startPos = Vector2D(posDistX(gen), posDistY(gen));
    }
    
    if (!startVel) {
        std::uniform_real_distribution<> velDist(-15.0, 15.0);
        startVel = Vector2D(velDist(gen), velDist(gen));
    }

    std::cout << "--- Space Golf Physics Engine CLI ---\n\n";
    std::cout << "World contains " << world.attractors.size() << " attractors (" 
              << fixedAttractors.size() << " fixed, " << (world.attractors.size() - fixedAttractors.size()) << " random).\n";
              
    for (size_t i = 0; i < world.attractors.size(); ++i) {
        const auto& p = world.attractors[i];
        std::cout << "  Attractor " << i << ": Mass=" << p.mass 
                  << ", Radius=" << p.radius 
                  << ", Pos=(" << p.position.x << ", " << p.position.y << ")\n";
    }

    std::cout << "\nParticle Start Pos: (" << startPos->x << ", " << startPos->y << ")\n";
    std::cout << "Particle Start Vel: (" << startVel->x << ", " << startVel->y << ")\n";

    EulerIntegrator sim;
    sim.stopVelocityThreshold = customStopThreshold.value_or(0.01);
    sim.dt = 1.0; 

    // 1. Simulate until stop
    std::cout << "\nSimulating until stop (timeout " << timeout << " frames)...\n";
    auto start_sim = std::chrono::high_resolution_clock::now();
    IntegratorResult result = sim.simulateUntilStop(world, *startPos, *startVel, timeout);
    auto end_sim = std::chrono::high_resolution_clock::now();
    auto duration_sim = std::chrono::duration_cast<std::chrono::microseconds>(end_sim - start_sim).count();
    
    std::cout << "Integrator complete in " << duration_sim << " microseconds (" 
              << (duration_sim / 1000.0) << " ms).\n";
    std::cout << "  Stopped? " << (result.stopped ? "Yes" : "No (Timeout)") << "\n";
    std::cout << "  Elapsed Time: " << result.timeElapsed << "\n";
    std::cout << "  Final Pos: (" << result.finalPosition.x << ", " << result.finalPosition.y << ")\n";
    std::cout << "  Final Vel: (" << result.finalVelocity.x << ", " << result.finalVelocity.y << ")\n";

    // 2. Get Trace
    std::cout << "\nGenerating fast trace for time " << traceStart << " to " << traceEnd << "...\n";
    auto start_trace = std::chrono::high_resolution_clock::now();
    std::vector<TracePoint> trace = sim.getTrace(world, *startPos, *startVel, traceStart, traceEnd);
    auto end_trace = std::chrono::high_resolution_clock::now();
    auto duration_trace = std::chrono::duration_cast<std::chrono::microseconds>(end_trace - start_trace).count();
    
    std::cout << "Trace complete in " << duration_trace << " microseconds (" 
              << (duration_trace / 1000.0) << " ms).\n";
    std::cout << "Trace points collected: " << trace.size() << "\n";

    // 3. Generate SVG
    std::cout << "\nGenerating SVG visualization...\n";
    std::ofstream svg("visualization.svg");
    if (svg.is_open()) {
        svg << "<svg width=\"1920\" height=\"1080\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        svg << "<rect width=\"100%\" height=\"100%\" fill=\"#1a1a2e\"/>\n";
        for (const auto& p : world.attractors) {
            svg << "<circle cx=\"" << p.position.x << "\" cy=\"" << p.position.y 
                << "\" r=\"" << p.radius << "\" fill=\"#16213e\" stroke=\"#0f3460\" stroke-width=\"2\"/>\n";
        }
        if (!trace.empty()) {
            svg << "<polyline points=\"";
            for (const auto& t : trace) {
                svg << t.position.x << "," << t.position.y << " ";
            }
            svg << "\" fill=\"none\" stroke=\"#e94560\" stroke-width=\"2\"/>\n";
            svg << "<circle cx=\"" << trace.front().position.x << "\" cy=\"" << trace.front().position.y 
                << "\" r=\"5\" fill=\"#00ff00\"/>\n";
            svg << "<circle cx=\"" << trace.back().position.x << "\" cy=\"" << trace.back().position.y 
                << "\" r=\"5\" fill=\"#ff0000\"/>\n";
        }
        svg << "</svg>\n";
        svg.close();
        std::cout << "Saved to visualization.svg\n";
    }

    return 0;
}
