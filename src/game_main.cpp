#include <iostream>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <random>

#include "raylib.h"

#include "GravityBilliards/World.hpp"
#include "GravityBilliards/Geometry.hpp"
#include "GravityBilliards/Trajectories.hpp"
#include "GravityBilliards/Topology.hpp"
#include "Game/GameOrchestrator.hpp"
#include "Graphics/ToroidalCamera.hpp"
#include <variant>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

using namespace GravityBilliards;
using namespace GravityGame;

class PlayerInputProvider {
public:
    double aimingAngle = 0.0;
    double currentPower = 0.0;
    
    // Tuning parameters
    const double maxPower = 2.0/20;
    const double powerIncrement = 0.2/20;
    const double decayRate =1.0/20; // Power lost per second
    const double rotationSpeed = 2.0; // Radians per second
    
    int lastKeyTapped = 0;

    void update(float dt) {
        // Aiming
        if (IsKeyDown(KEY_LEFT)) {
            aimingAngle -= rotationSpeed * dt;
        }
        if (IsKeyDown(KEY_RIGHT)) {
            aimingAngle += rotationSpeed * dt;
        }

        // Tapping for power
        if (IsKeyPressed(KEY_D) && lastKeyTapped != KEY_D) {
            currentPower += powerIncrement;
            lastKeyTapped = KEY_D;
        }
        if (IsKeyPressed(KEY_F) && lastKeyTapped != KEY_F) {
            currentPower += powerIncrement;
            lastKeyTapped = KEY_F;
        }

        // Decay
        currentPower -= decayRate * dt;
        
        // Clamp
        if (currentPower < 0.0) currentPower = 0.0;
        if (currentPower > maxPower) currentPower = maxPower;
    }

    std::optional<Vector2D> getForceInput() {
        if (currentPower > 0.0) {
            return Vector2D(std::cos(aimingAngle), std::sin(aimingAngle)) * currentPower;
        }
        return std::nullopt;
    }
};

// Game State Struct
struct GameState {
    int health = 5;
    int totalCollectibles = 0;
    double damageCooldown = 0.0;
    double accumulator = 0.0;
    std::variant<std::shared_ptr<GameOrchestrator<EuclideanTopology>>, 
                 std::shared_ptr<GameOrchestrator<ToroidalTopology>>> orchestrator;
    std::shared_ptr<PlayerInputProvider> inputProvider;
    std::optional<Graphics::ToroidalCamera> camera;
};

void resetLevel(GameState& state, int screenWidth, int viewportHeight, bool useTorus) {
    double w = screenWidth;
    double h = viewportHeight;
    if (useTorus) {
        w = screenWidth * 4.0;
        h = viewportHeight * 4.0;
    }
    int numPlanets = useTorus ? 20 : 5;
    World world(numPlanets, w, h);
    
    // Start planet is index 0
    const Attractor& startPlanet = world.attractors.front();
    double p1Radius = startPlanet.radius;
    // Spawn just outside the start planet
    Vector2D startPos = startPlanet.position + Vector2D(p1Radius + 10.0 + 2.0, 0.0);
    
    // Target planet is the last one
    int targetIndex = world.attractors.size() - 1;
    const Attractor& targetPlanet = world.attractors.back();
    
    if (useTorus) {
        ToroidalTopology top(w, h);
        auto orch = std::make_shared<GameOrchestrator<ToroidalTopology>>(world, startPos, top);
        orch->physicsEngine.particleRadius = 10.0;
        orch->physicsEngine.dt = 1.0 / 120.0;
        orch->player.radius = 10.0;
        state.orchestrator = orch;
        
        state.camera = Graphics::ToroidalCamera(w, h, screenWidth, viewportHeight);
        state.camera->position = startPos; // Start camera at player
    } else {
        EuclideanTopology top;
        auto orch = std::make_shared<GameOrchestrator<EuclideanTopology>>(world, startPos, top);
        orch->physicsEngine.particleRadius = 10.0;
        orch->physicsEngine.dt = 1.0 / 120.0;
        orch->player.radius = 10.0;
        state.orchestrator = orch;
        
        state.camera.reset();
    }
    
    // Find safe apoapsis
    double safeDistance = 1e9;
    for (int i = 0; i < (int)world.attractors.size(); ++i) {
        if (i == targetIndex) continue;
        double dist = targetPlanet.position.distanceTo(world.attractors[i].position) - world.attractors[i].radius;
        if (dist < safeDistance) safeDistance = dist;
    }
    double maxApoapsis = safeDistance - 20.0;
    
    state.totalCollectibles = 3 + rand() % 3; // 3 to 5 satellites
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> eDist(0.1, 0.6);
    std::uniform_real_distribution<> angleDist(0.0, 2.0 * PI);
    
    for (int i = 0; i < state.totalCollectibles; ++i) {
        GameEntity col;
        col.scoreValue = 10;
        
        KeplerOrbit orbit;
        orbit.e = eDist(gen);
        double minA = (targetPlanet.radius + 15.0) / (1.0 - orbit.e);
        double maxA = maxApoapsis / (1.0 + orbit.e);
        if (maxA < minA) maxA = minA + 10.0; 
        
        std::uniform_real_distribution<> aDist(minA, maxA);
        orbit.a = aDist(gen);
        orbit.omega = angleDist(gen);
        orbit.M0 = angleDist(gen);
        
        col.trajectory = orbit;
        col.parentAttractorIndex = targetIndex;
        col.radius = 5.0;
        col.isPhysicsBound = false;
        
        // Initial position before first update
        col.position = Trajectories::solveKeplerOrbit(orbit, targetPlanet.position, targetPlanet.mass, 0.0);
        
        std::visit([&](auto& orch) {
            orch->entities.push_back(col);
            orch->totalCollectibles = state.totalCollectibles;
        }, state.orchestrator);
    }
    
    // Spawn Asteroids
    for (int i = 0; i < 2; ++i) {
        GameEntity asteroid;
        asteroid.damageValue = 1;
        asteroid.scoreValue = 0;
        asteroid.radius = 8.0;
        asteroid.mass = 2.0;
        
        KeplerOrbit orbit;
        orbit.e = eDist(gen);
        double minA = (targetPlanet.radius + 20.0) / (1.0 - orbit.e);
        double maxA = maxApoapsis / (1.0 + orbit.e);
        if (maxA < minA) maxA = minA + 10.0; 
        
        std::uniform_real_distribution<> aDist(minA, maxA);
        orbit.a = aDist(gen);
        orbit.omega = angleDist(gen);
        orbit.M0 = angleDist(gen);
        
        asteroid.trajectory = orbit;
        asteroid.parentAttractorIndex = targetIndex;
        asteroid.isPhysicsBound = false;
        
        asteroid.position = Trajectories::solveKeplerOrbit(orbit, targetPlanet.position, targetPlanet.mass, 0.0);
        
        std::visit([&](auto& orch) {
            orch->entities.push_back(asteroid);
        }, state.orchestrator);
    }
    
    state.accumulator = 0.0;
    state.inputProvider = std::make_shared<PlayerInputProvider>();
}

int main() {
    int screenWidth = 1000;
    int screenHeight = 900;
    int viewportHeight = 800;
    
    InitWindow(screenWidth, screenHeight, "Gravity Billiards - Space Golf");
    SetTargetFPS(60);

    bool useTorus = false;
    GameState state;
    resetLevel(state, screenWidth, viewportHeight, useTorus);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        bool resetRequested = false;

        std::visit([&](auto& orchestrator) {
            state.inputProvider->update(dt);
            // Fixed step physics accumulator
            double simDt = dt * 30.0;
            state.accumulator += simDt;
            double engineDt = orchestrator->physicsEngine.dt;
            
            while (state.accumulator >= engineDt) {
                auto force = state.inputProvider->getForceInput();
                if (force.has_value()) {
                    orchestrator->playerThrust = force.value();
                } else {
                    orchestrator->playerThrust = {0.0, 0.0};
                }
                
                orchestrator->update();
                state.accumulator -= engineDt;
            }
            
            // Draw Win/Loss text if game is over
            std::string gameStatusText = "";
            Color statusColor = BLACK;
            if (orchestrator->status == GameStatus::WON) {
                gameStatusText = "MISSION ACCOMPLISHED!";
                statusColor = GREEN;
            } else if (orchestrator->status == GameStatus::LOST) {
                gameStatusText = "SHIP DESTROYED!";
                statusColor = RED;
            }
            
            if (resetRequested) return;

            if (state.camera.has_value()) {
                state.camera->trackTarget(orchestrator->player.position, 0.1);
            }

            // Rendering
            BeginDrawing();
            ClearBackground(RAYWHITE);

            if (state.camera.has_value()) {
                Camera2D rayCam = { 0 };
                rayCam.target = {(float)state.camera->position.x, (float)state.camera->position.y};
                rayCam.offset = {(float)screenWidth / 2.0f, (float)viewportHeight / 2.0f};
                rayCam.rotation = 0.0f;
                rayCam.zoom = 1.0f;
                BeginMode2D(rayCam);
            }

            const auto& currentWorld = orchestrator->world;
            const auto& player = orchestrator->player;

            auto drawAtPositions = [&](const Vector2D& physPos, double radius, auto drawFunc) {
                if (state.camera.has_value()) {
                    auto positions = state.camera->getVisibleRenderPositions(physPos, radius);
                    for (const auto& pos : positions) {
                        drawFunc(pos);
                    }
                } else {
                    drawFunc(physPos);
                }
            };

            // Draw Planets
            for (int i = 0; i < (int)currentWorld.attractors.size(); ++i) {
                const auto& attractor = currentWorld.attractors[i];
                Color c = DARKGRAY;
                if (i == 0) c = BLUE; // Home Planet
                if (i == (int)currentWorld.attractors.size() - 1) c = GREEN; // Target Planet
                
                drawAtPositions(attractor.position, attractor.radius, [&](const Vector2D& pos) {
                    DrawCircleV({(float)pos.x, (float)pos.y}, (float)attractor.radius, c);
                });
            }

            // Draw Entities
            for (const auto& ent : orchestrator->entities) {
                if (!ent.active) continue;
                
                if (ent.scoreValue > 0) { // Collectible
                    drawAtPositions(ent.position, ent.radius, [&](const Vector2D& pos) {
                        DrawCircleV({(float)pos.x, (float)pos.y}, (float)ent.radius, ORANGE);
                        DrawCircleLines((int)pos.x, (int)pos.y, ent.radius + 1.0f, YELLOW);
                    });
                } else if (ent.damageValue > 0) { // Asteroid
                    drawAtPositions(ent.position, ent.radius, [&](const Vector2D& pos) {
                        DrawCircleV({(float)pos.x, (float)pos.y}, (float)ent.radius, GRAY);
                        DrawCircleLines((int)pos.x, (int)pos.y, ent.radius + 1.0f, DARKGRAY);
                    });
                }
            }

            // Draw Player Particle
            Color pColor = (state.damageCooldown > 0.0) ? ColorAlpha(RED, 0.5f) : RED;
            drawAtPositions(player.position, player.radius, [&](const Vector2D& pos) {
                DrawCircleV({(float)pos.x, (float)pos.y}, (float)player.radius, pColor);
            });

            // Draw aiming caret (^)
            Vector2D aimDir(std::cos(state.inputProvider->aimingAngle), std::sin(state.inputProvider->aimingAngle));
            Vector2D perpDir(-aimDir.y, aimDir.x);
            
            drawAtPositions(player.position, player.radius * 2.0, [&](const Vector2D& pos) {
                Vector2D nose = pos + aimDir * (player.radius * 0.8);
                Vector2D backLeft = pos - aimDir * (player.radius * 0.2) + perpDir * (player.radius * 0.5);
                Vector2D backRight = pos - aimDir * (player.radius * 0.2) - perpDir * (player.radius * 0.5);
                DrawLineEx({(float)nose.x, (float)nose.y}, {(float)backLeft.x, (float)backLeft.y}, 2.0f, WHITE);
                DrawLineEx({(float)nose.x, (float)nose.y}, {(float)backRight.x, (float)backRight.y}, 2.0f, WHITE);
            });

            // --- DEBUG VECTORS ---
            const float FORCE_SCALE = 1000.0f; 
            const float VEL_SCALE = 10.0f;     

            Vector2D gravityForce{0.0, 0.0};
            for (const auto& attractor : currentWorld.attractors) {
                Vector2D dir = orchestrator->physicsEngine.topology.getShortestDirection(player.position, attractor.position);
                double distSq = dir.x * dir.x + dir.y * dir.y;
                double dist = std::sqrt(distSq);
                double cubeDistance = dist * distSq;
                if (cubeDistance > 0.0001) { 
                    gravityForce += dir * (static_cast<double>(attractor.mass) / cubeDistance);
                }
            }

            drawAtPositions(player.position, 100.0, [&](const Vector2D& pos) {
                Vector2D gravEnd = pos + gravityForce * FORCE_SCALE;
                DrawLineEx({(float)pos.x, (float)pos.y}, {(float)gravEnd.x, (float)gravEnd.y}, 2.0f, PURPLE);

                Vector2D thrustForce = aimDir * state.inputProvider->currentPower;
                Vector2D thrustEnd = pos + thrustForce * FORCE_SCALE;
                DrawLineEx({(float)pos.x, (float)pos.y}, {(float)thrustEnd.x, (float)thrustEnd.y}, 3.0f, BLUE);

                Vector2D velEnd = pos + player.velocity * VEL_SCALE;
                DrawLineEx({(float)pos.x, (float)pos.y}, {(float)velEnd.x, (float)velEnd.y}, 2.0f, GREEN);
            });

            if (state.camera.has_value()) EndMode2D();

            // Draw Power Bar UI
            DrawText("POWER", 20, 20, 20, DARKGRAY);
            DrawRectangleLines(20, 50, 200, 30, DARKGRAY);
            float powerRatio = (float)(state.inputProvider->currentPower / state.inputProvider->maxPower);
            DrawRectangle(20, 50, (int)(200 * powerRatio), 30, RED);

            // Status UI
            DrawText(TextFormat("Health: %d / 5", orchestrator->health), 20, 100, 20, (orchestrator->health <= 2) ? RED : DARKGRAY);
            int collectedCount = orchestrator->score / 10;
            DrawText(TextFormat("Collected: %d / %d", collectedCount, state.totalCollectibles), 20, 130, 20, DARKGRAY);
            
            if (!gameStatusText.empty()) {
                int textW = MeasureText(gameStatusText.c_str(), 40);
                DrawText(gameStatusText.c_str(), screenWidth/2 - textW/2, viewportHeight/2 - 20, 40, statusColor);
                int subW = MeasureText("Press 'Generate Fresh Level' to play again", 20);
                DrawText("Press 'Generate Fresh Level' to play again", screenWidth/2 - subW/2, viewportHeight/2 + 30, 20, DARKGRAY);
            }
            
            // Off-screen indicators
            if (state.camera.has_value()) {
                auto drawIndicator = [&](const Vector2D& targetPos, Color c) {
                    auto positions = state.camera->getVisibleRenderPositions(targetPos, 50.0);
                    if (positions.empty()) {
                        Vector2D dir = orchestrator->physicsEngine.topology.getShortestDirection(player.position, targetPos).normalized();
                        Vector2D center(screenWidth / 2.0, viewportHeight / 2.0);
                        
                        double scaleX = 1e9, scaleY = 1e9;
                        if (std::abs(dir.x) > 0.001) scaleX = (screenWidth / 2.0 - 30.0) / std::abs(dir.x);
                        if (std::abs(dir.y) > 0.001) scaleY = (viewportHeight / 2.0 - 30.0) / std::abs(dir.y);
                        double scale = std::min(scaleX, scaleY);
                        
                        Vector2D arrowPos = center + dir * scale;
                        
                        // Draw a circle and line instead of triangle to ensure visibility
                        DrawCircleV({(float)arrowPos.x, (float)arrowPos.y}, 15.0f, c);
                        Vector2D p1 = arrowPos + dir * 20.0;
                        DrawLineEx({(float)arrowPos.x, (float)arrowPos.y}, {(float)p1.x, (float)p1.y}, 5.0f, WHITE);
                        DrawCircleLines((int)arrowPos.x, (int)arrowPos.y, 16.0f, WHITE);
                    }
                };
                
                // Home planet (0)
                drawIndicator(currentWorld.attractors.front().position, BLUE);
                // Target planet (last)
                drawIndicator(currentWorld.attractors.back().position, GREEN);
            }

            // Menu Background at the bottom
            DrawRectangle(0, viewportHeight, screenWidth, screenHeight - viewportHeight, LIGHTGRAY);
            DrawLine(0, viewportHeight, screenWidth, viewportHeight, DARKGRAY);

            // Instructions
            DrawText("Collect all ORANGE satellites around the GREEN planet.", 20, viewportHeight + 15, 20, DARKGRAY);
            DrawText("Land gently on the BLUE home planet to win! Bang hard into planets: Damage!", 20, viewportHeight + 45, 20, DARKGRAY);

            // UI for toggling
            if (GuiButton({ (float)screenWidth - 340, (float)viewportHeight + 35, 150, 30 }, useTorus ? "Mode: Torus" : "Mode: Plane")) {
                useTorus = !useTorus;
                resetRequested = true;
            }
            if (GuiButton({ (float)screenWidth - 170, (float)viewportHeight + 35, 150, 30 }, "Generate Fresh Level")) {
                resetRequested = true;
            }

            EndDrawing();
        }, state.orchestrator);

        if (resetRequested) {
            resetLevel(state, screenWidth, viewportHeight, useTorus);
        }
    }

    CloseWindow();
    return 0;
}
