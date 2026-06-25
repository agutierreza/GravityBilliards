#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "SpaceGolf/Level.hpp"
#include "SpaceGolf/Simulation.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace SpaceGolf;

// Helper to draw a slider with fine keyboard controls when hovering
bool DrawFineSlider(Rectangle bounds, const char* textLeft, const char* textRight, float* value, float min, float max, float fineStep) {
    bool changed = GuiSlider(bounds, textLeft, textRight, value, min, max);
    
    // Add fine-tuning with arrow keys while hovering over the slider
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP)) {
            *value += fineStep;
            changed = true;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN)) {
            *value -= fineStep;
            changed = true;
        }
        // Clamp
        if (*value > max) *value = max;
        if (*value < min) *value = min;
    }
    return changed;
}

enum class DragState { None, Particle, Planet, Minimap };

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Space Golf Physics UI & Replay System");
    SetTargetFPS(60);

    // Initial Physics Setup
    int uiPanelWidth = 350;
    int canvasWidth = screenWidth - uiPanelWidth;
    
    Level level(10, canvasWidth, screenHeight - 60); 
    Simulation sim;
    sim.stopVelocityThreshold = 0.01;
    sim.dt = 1.0;

    // GUI State Variables
    float startX = 100.0f;
    float startY = (screenHeight - 60) / 2.0f;
    float velX = 2.0f;
    float velY = 0.0f;
    
    float velAngle = std::atan2(velY, velX) * 180.0f / PI;
    float velForce = std::sqrt(velX*velX + velY*velY);
    
    float particleRadiusFloat = 6.0f;
    float logThresholdFloat = -1.35f; // 10^-1.35
    float checkAfterTimeFloat = 0.0f;
    
    bool isPlaying = false;
    float playbackFrame = 0.0f;
    float playbackSpeed = 1.0f;
    
    int numPlanets = 10;
    float numPlanetsFloat = 10.0f;

    std::vector<TracePoint> trace;
    bool needsTraceUpdate = true;
    
    std::string exportMessage = "";
    float exportMessageTimer = 0.0f;

    // Neon-80s-Arcade Style Colors
    Color bgDark = {11, 0, 28, 255};          
    Color neonPink = {255, 0, 127, 255};      
    Color neonCyan = {0, 255, 255, 255};      
    Color neonGreen = {57, 255, 20, 255};     
    Color neonYellow = {255, 255, 0, 255};    
    Color planetFill = {20, 0, 50, 255};
    Color uiBg = {20, 0, 40, 255};
    Color gridLine = {255, 0, 255, 40};

    // Apply Raygui styles
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(neonCyan));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(uiBg));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(neonPink));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(neonYellow));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt({40, 0, 80, 255}));
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(neonCyan));
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ColorToInt(neonGreen));
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, ColorToInt({60, 0, 120, 255}));
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(neonGreen));
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(uiBg));

    // Camera setup
    Camera2D camera = { 0 };
    camera.target = (Vector2){ (float)canvasWidth/2.0f, (float)(screenHeight-60)/2.0f };
    camera.offset = (Vector2){ (float)canvasWidth/2.0f, (float)(screenHeight-60)/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    DragState dragState = DragState::None;
    int draggedPlanetIndex = -1;

    while (!WindowShouldClose())
    {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        canvasWidth = currentWidth - uiPanelWidth;
        int canvasHeight = currentHeight - 60;

        // (Removed constant offset resetting so zoom-to-mouse works perfectly)

        Vector2 mousePos = GetMousePosition();
        Vector2 worldMouse = GetScreenToWorld2D(mousePos, camera);
        
        // Define Minimap bounds
        Rectangle minimapRect = { (float)canvasWidth + 20, (float)currentHeight - 200, (float)uiPanelWidth - 40, 130.0f };
        float mapWorldWidth = 2500.0f; // Logical world bounds for minimap
        float mapWorldHeight = 1500.0f;
        float mapWorldOffsetX = -200.0f;
        float mapWorldOffsetY = -200.0f;

        // Check for File Drop (JSON Import)
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".json")) {
                try {
                    std::ifstream f(droppedFiles.paths[0]);
                    if (f.is_open()) {
                        nlohmann::json j;
                        f >> j;
                        
                        if (j.contains("planets")) {
                            level.planets.clear();
                            for (const auto& p : j["planets"]) {
                                Planet planet(p["mass"].get<int>(), Vector2D(p["position"]["x"].get<double>(), p["position"]["y"].get<double>()));
                                if (p.contains("radius")) planet.radius = p["radius"];
                                level.planets.push_back(planet);
                            }
                            numPlanetsFloat = (float)level.planets.size();
                        }
                        
                        if (j.contains("particle")) {
                            startX = j["particle"]["startPosition"]["x"].get<float>();
                            startY = j["particle"]["startPosition"]["y"].get<float>();
                            velX = j["particle"]["startVelocity"]["x"].get<float>();
                            velY = j["particle"]["startVelocity"]["y"].get<float>();
                            velAngle = std::atan2(velY, velX) * 180.0f / PI;
                            velForce = std::sqrt(velX*velX + velY*velY);
                        }
                        
                        if (j.contains("simulation")) {
                            if (j["simulation"].contains("stopVelocityThreshold")) {
                                float threshold = j["simulation"]["stopVelocityThreshold"];
                                logThresholdFloat = std::log10(threshold);
                            }
                            if (j["simulation"].contains("particleRadius")) {
                                particleRadiusFloat = j["simulation"]["particleRadius"];
                            }
                        }
                        
                        needsTraceUpdate = true;
                        playbackFrame = 0;
                        exportMessage = "Imported " + std::string(GetFileName(droppedFiles.paths[0]));
                        exportMessageTimer = 3.0f;
                    }
                } catch (...) {
                    exportMessage = "Failed to parse JSON!";
                    exportMessageTimer = 3.0f;
                }
            }
            UnloadDroppedFiles(droppedFiles);
        }

        // Input Logic
        bool mouseInCanvas = (mousePos.x >= 0 && mousePos.x <= canvasWidth && mousePos.y >= 0 && mousePos.y <= canvasHeight);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, minimapRect)) {
                dragState = DragState::Minimap;
            } else if (mouseInCanvas) {
                // Check particle
                if (CheckCollisionPointCircle(worldMouse, {startX, startY}, particleRadiusFloat + 10.0f)) { // slightly larger hitbox
                    dragState = DragState::Particle;
                } else {
                    // Check planets
                    for(int i = 0; i < level.planets.size(); ++i) {
                        Vector2 pPos = {(float)level.planets[i].position.x, (float)level.planets[i].position.y};
                        if (CheckCollisionPointCircle(worldMouse, pPos, level.planets[i].radius)) {
                            dragState = DragState::Planet;
                            draggedPlanetIndex = i;
                            break;
                        }
                    }
                }
            }
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragState = DragState::None;
            draggedPlanetIndex = -1;
        }

        // Drag execution
        if (dragState == DragState::Particle || dragState == DragState::Planet) {
            Vector2 clampedMouse = mousePos;
            if (clampedMouse.x < 0) clampedMouse.x = 0;
            if (clampedMouse.x > canvasWidth) clampedMouse.x = canvasWidth;
            if (clampedMouse.y < 0) clampedMouse.y = 0;
            if (clampedMouse.y > canvasHeight) clampedMouse.y = canvasHeight;
            Vector2 clampedWorldMouse = GetScreenToWorld2D(clampedMouse, camera);

            if (dragState == DragState::Particle) {
                startX = clampedWorldMouse.x;
                startY = clampedWorldMouse.y;
            } else if (draggedPlanetIndex != -1) {
                level.planets[draggedPlanetIndex].position = {clampedWorldMouse.x, clampedWorldMouse.y};
            }
            needsTraceUpdate = true;
        } else if (dragState == DragState::Minimap) {
            Vector2 delta = GetMouseDelta();
            camera.target.x += delta.x * (mapWorldWidth / minimapRect.width);
            camera.target.y += delta.y * (mapWorldHeight / minimapRect.height);
        }

        // Camera Pan
        if ((IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) && mouseInCanvas) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        // Camera Zoom
        float wheel = GetMouseWheelMove();
        if (wheel != 0 && mouseInCanvas) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);
            camera.offset = mousePos;
            camera.target = mouseWorldPos;
            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.1f, 10.0f);
        }

        // Vector Sync
        velX = std::cos(velAngle * PI / 180.0f) * velForce;
        velY = std::sin(velAngle * PI / 180.0f) * velForce;

        // 1. Logic Update
        sim.particleRadius = particleRadiusFloat; // Sync radius
        float actualStopThreshold = std::pow(10.0f, logThresholdFloat);
        sim.stopVelocityThreshold = actualStopThreshold;
        
        if (needsTraceUpdate) {
            Vector2D particlePos(startX, startY);
            Vector2D particleVel(velX, velY);
            trace = sim.getTrace(level, particlePos, particleVel, 0.0, 5000.0);
            needsTraceUpdate = false;
        }

        // Find stop frame marker
        int stopFrameIdx = -1;
        if (!trace.empty()) {
            double threshSq = actualStopThreshold * actualStopThreshold;
            for (size_t i = 0; i < trace.size(); ++i) {
                if (trace[i].time >= checkAfterTimeFloat) {
                    if (trace[i].velocity.magnitudeSquared() < threshSq) {
                        stopFrameIdx = (int)i;
                        break;
                    }
                }
            }
        }

        if (isPlaying && !trace.empty()) {
            playbackFrame += playbackSpeed;
            if (playbackFrame >= trace.size()) {
                playbackFrame = trace.size() - 1;
                isPlaying = false;
            }
        }
        
        if (exportMessageTimer > 0) exportMessageTimer -= GetFrameTime();

        // 2. Render Phase
        BeginDrawing();
            ClearBackground(bgDark); 

            // --- DRAW CANVAS (Camera View) ---
            BeginMode2D(camera);
                
                // Draw origin axes lightly
                DrawLine(-10000, 0, 10000, 0, gridLine);
                DrawLine(0, -10000, 0, 10000, gridLine);

                for (const auto& p : level.planets) {
                    DrawCircle(p.position.x, p.position.y, p.radius, planetFill); 
                    DrawCircleLines(p.position.x, p.position.y, p.radius, neonCyan); 
                }

                if (trace.size() > 1) {
                    for (size_t i = 0; i < trace.size() - 1; ++i) {
                        Vector2 start = {(float)trace[i].position.x, (float)trace[i].position.y};
                        Vector2 end = {(float)trace[i+1].position.x, (float)trace[i+1].position.y};
                        DrawLineEx(start, end, 1.5f, {255, 0, 127, 200}); 
                    }
                }

                // Draw moving particle / start particle
                DrawCircle(startX, startY, particleRadiusFloat, {255, 255, 0, 100}); // Ghost start
                if (!trace.empty()) {
                    int frameIdx = (int)playbackFrame;
                    if (frameIdx >= trace.size()) frameIdx = trace.size() - 1;
                    
                    Vector2D currentPos = trace[frameIdx].position;
                    DrawCircle(currentPos.x, currentPos.y, particleRadiusFloat, neonGreen); 
                }

                // Draw initial velocity vector
                DrawLineEx({startX, startY}, {startX + velX*5.0f, startY + velY*5.0f}, 2.0f, neonYellow);

                // Draw Stop Marker
                if (stopFrameIdx != -1) {
                    Vector2D stopPos = trace[stopFrameIdx].position;
                    DrawCircleLines(stopPos.x, stopPos.y, 15.0f, neonYellow);
                    DrawText(TextFormat("Stop Frame: %d", stopFrameIdx), stopPos.x + 20, stopPos.y - 10, 10, neonYellow);
                }
            EndMode2D();

            // --- DRAW RIGHT UI PANEL ---
            DrawRectangle(canvasWidth, 0, uiPanelWidth, currentHeight, uiBg);
            DrawLine(canvasWidth, 0, canvasWidth, currentHeight, neonPink);

            int panelX = canvasWidth + 20;
            int currentY = 20;
            
            DrawText("LEVEL CONTROLS", panelX, currentY, 20, neonCyan);
            currentY += 30;
            
            GuiLabel(Rectangle{(float)panelX, (float)currentY, 120, 20}, "Planet Count:");
            if (DrawFineSlider(Rectangle{(float)panelX + 100, (float)currentY, 140, 20}, NULL, TextFormat("%d", (int)numPlanetsFloat), &numPlanetsFloat, 1, 30, 1.0f)) {}
            currentY += 30;
            
            if (GuiButton(Rectangle{(float)panelX, (float)currentY, 260, 30}, "Generate Random Level")) {
                level = Level((int)numPlanetsFloat, canvasWidth, canvasHeight);
                needsTraceUpdate = true;
                playbackFrame = 0;
            }
            currentY += 50;
            
            DrawLine(panelX, currentY, panelX + 260, currentY, neonPink);
            currentY += 20;

            DrawText("PARTICLE CONTROLS", panelX, currentY, 20, neonCyan);
            currentY += 30;

            GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Angle:");
            if (DrawFineSlider(Rectangle{(float)panelX + 60, (float)currentY, 180, 20}, NULL, TextFormat("%.1f deg", velAngle), &velAngle, -180.0f, 180.0f, 1.0f)) needsTraceUpdate = true;
            currentY += 30;

            GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Force:");
            if (DrawFineSlider(Rectangle{(float)panelX + 60, (float)currentY, 180, 20}, NULL, TextFormat("%.1f", velForce), &velForce, 0.0f, 15.0f, 0.1f)) needsTraceUpdate = true;
            currentY += 30;

            GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Radius:");
            if (DrawFineSlider(Rectangle{(float)panelX + 60, (float)currentY, 180, 20}, NULL, TextFormat("%.1f", particleRadiusFloat), &particleRadiusFloat, 1.0f, 50.0f, 1.0f)) needsTraceUpdate = true;
            currentY += 30;

            GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Stop Thresh:");
            if (DrawFineSlider(Rectangle{(float)panelX + 90, (float)currentY, 150, 20}, NULL, TextFormat("10^%.2f (%.5f)", logThresholdFloat, actualStopThreshold), &logThresholdFloat, -3.0f, -1.0f, 0.01f)) needsTraceUpdate = true;
            currentY += 50;
            
            DrawLine(panelX, currentY, panelX + 260, currentY, neonPink);
            currentY += 20;
            
            DrawText("PLAYBACK", panelX, currentY, 20, neonCyan);
            currentY += 30;

            if (GuiButton(Rectangle{(float)panelX, (float)currentY, 260, 40}, isPlaying ? "PAUSE" : "PLAY SIMULATION")) {
                isPlaying = !isPlaying;
                if (isPlaying && playbackFrame >= trace.size() - 1) playbackFrame = 0;
            }
            currentY += 60;
            
            GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Speed:");
            DrawFineSlider(Rectangle{(float)panelX + 60, (float)currentY, 180, 20}, NULL, TextFormat("%.1fx", playbackSpeed), &playbackSpeed, 0.1f, 10.0f, 0.1f);
            currentY += 40;
            
            DrawLine(panelX, currentY, panelX + 260, currentY, neonPink);
            currentY += 20;

            // --- MINIMAP ---
            DrawText("MINIMAP", panelX, currentY, 20, neonCyan);
            
            DrawRectangleRec(minimapRect, {15, 0, 30, 255});
            DrawRectangleLinesEx(minimapRect, 1, neonPink);

            // Draw planets on minimap
            for (const auto& p : level.planets) {
                float mx = minimapRect.x + ((p.position.x - mapWorldOffsetX) / mapWorldWidth) * minimapRect.width;
                float my = minimapRect.y + ((p.position.y - mapWorldOffsetY) / mapWorldHeight) * minimapRect.height;
                float mr = (p.radius / mapWorldWidth) * minimapRect.width;
                if (mr < 1.0f) mr = 1.0f;
                // Only draw if inside minimap
                if (mx > minimapRect.x && mx < minimapRect.x + minimapRect.width &&
                    my > minimapRect.y && my < minimapRect.y + minimapRect.height) {
                    DrawCircle(mx, my, mr, neonCyan);
                }
            }

            // Draw Camera Viewport on Minimap
            Vector2 viewTL = GetScreenToWorld2D({0, 0}, camera);
            Vector2 viewBR = GetScreenToWorld2D({(float)canvasWidth, (float)canvasHeight}, camera);
            
            float vmx = minimapRect.x + ((viewTL.x - mapWorldOffsetX) / mapWorldWidth) * minimapRect.width;
            float vmy = minimapRect.y + ((viewTL.y - mapWorldOffsetY) / mapWorldHeight) * minimapRect.height;
            float vmw = ((viewBR.x - viewTL.x) / mapWorldWidth) * minimapRect.width;
            float vmh = ((viewBR.y - viewTL.y) / mapWorldHeight) * minimapRect.height;
            
            Rectangle viewRect = {vmx, vmy, vmw, vmh};
            // Use Scissor mode to prevent drawing the view rect outside the minimap!
            BeginScissorMode((int)minimapRect.x, (int)minimapRect.y, (int)minimapRect.width, (int)minimapRect.height);
            DrawRectangleLinesEx(viewRect, 1, neonGreen);
            EndScissorMode();

            // Diagnostics export
            if (GuiButton(Rectangle{(float)panelX, (float)currentHeight - 40, 260, 30}, "Export Variables")) {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                
                std::filesystem::path exeDir = GetApplicationDirectory();
                std::filesystem::path exportDir = exeDir / ".." / "exports";
                std::error_code ec;
                std::filesystem::create_directories(exportDir, ec);
                
                std::stringstream ssFilename;
                ssFilename << "export_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << ".json";
                std::string filename = (exportDir / ssFilename.str()).string();
                
                nlohmann::json j;
                j["simulation"]["stopVelocityThreshold"] = actualStopThreshold;
                j["simulation"]["particleRadius"] = particleRadiusFloat;
                j["particle"]["startPosition"] = {{"x", startX}, {"y", startY}};
                j["particle"]["startVelocity"] = {{"x", velX}, {"y", velY}};
                j["planets"] = nlohmann::json::array();
                for (const auto& p : level.planets) {
                    j["planets"].push_back({
                        {"position", {{"x", p.position.x}, {"y", p.position.y}}},
                        {"mass", p.mass},
                        {"radius", p.radius}
                    });
                }
                
                std::ofstream out(filename);
                out << j.dump(4);
                out.close();
                
                exportMessage = "Exported to exports/" + ssFilename.str();
                exportMessageTimer = 3.0f;
            }
            DrawText("Drag & Drop a .json file anywhere to import", panelX, currentHeight - 70, 10, {255, 0, 127, 200});
            if (exportMessageTimer > 0) {
                DrawText(exportMessage.c_str(), panelX, currentHeight - 55, 10, neonGreen);
            }

            // --- DRAW BOTTOM TIMELINE SCRUBBER ---
            DrawRectangle(0, currentHeight - 60, canvasWidth, 60, uiBg);
            DrawLine(0, currentHeight - 60, canvasWidth, currentHeight - 60, neonPink);
            
            GuiLabel(Rectangle{20, (float)currentHeight - 40, 60, 20}, "Timeline:");
            
            float maxFrames = (float)(trace.empty() ? 0 : trace.size() - 1);
            if (DrawFineSlider(Rectangle{90, (float)currentHeight - 40, (float)canvasWidth - 120, 20}, NULL, TextFormat("Frame %d", (int)playbackFrame), &playbackFrame, 0, maxFrames, 1.0f)) {
                isPlaying = false; 
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
