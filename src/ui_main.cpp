#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "GravityBilliards/World.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/PhysicsEngine.hpp"
#include "GravityBilliards/DynamicWrappers.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"
#include "GravityBilliards/InelasticCollisionResolver.hpp"
#include "GravityBilliards/InitialConditions.hpp"
#include "GravityBilliards/EnergyDiagnostics.hpp"
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

#define ENABLE_GRAVITY_DEBUG_KNOBS 1

using namespace GravityBilliards;

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

enum class DragState { None, Particle, Attractor, Minimap };

int main(void)
{
    const int screenWidth = 1600;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Space Golf Physics UI & Replay System");
    SetTargetFPS(60);

    // Initial Physics Setup
    int uiPanelWidth = 350;
    int canvasWidth = screenWidth - uiPanelWidth;
    
    World world(10, canvasWidth, screenHeight - 60); 
    auto integrator = std::make_shared<EulerIntegrator>();
    auto detector = std::make_shared<CircleCollisionDetector>();
    auto resolver = std::make_shared<InelasticCollisionResolver>();
    
    DynamicIntegrator dynInt{integrator};
    DynamicDetector dynDet{detector};
    DynamicResolver dynRes{resolver};

    PhysicsEngine<DynamicIntegrator, DynamicDetector, DynamicResolver> sim(dynInt, dynDet, dynRes);
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
    
    int numAttractors = 10;
    float numAttractorsFloat = 10.0f;

    std::vector<TracePoint> trace;
    bool needsTraceUpdate = true;
    int stopFrameIdx = -1;
    
    std::string exportMessage = "";
    float exportMessageTimer = 0.0f;

    bool showEnergyGraph = false;
    bool needsEnergyUpdate = false;
    EnergyTimeline energyTimeline;

    bool showGravityMap = false;
    bool needsGravityMapUpdate = true;
    const int mapResX = 128;
    const int mapResY = 128;
    Image gravityImage = GenImageColor(mapResX, mapResY, BLANK);
    Texture2D gravityTexture = LoadTextureFromImage(gravityImage);

#if ENABLE_GRAVITY_DEBUG_KNOBS
    float dbgPercentileDeep = 0.058f;
    float dbgPercentileShallow = 1.000f;
    float dbgContrastPower = 1.00f; 
    float dbgMaxAlpha = 137.0f;
#endif

    // Neon-80s-Arcade Style Colors
    Color bgDark = {11, 0, 28, 255};          
    Color neonPink = {255, 0, 127, 255};      
    Color neonCyan = {0, 255, 255, 255};      
    Color neonGreen = {57, 255, 20, 255};     
    Color neonYellow = {255, 255, 0, 255};    
    Color attractorFill = {20, 0, 50, 255};
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
    int draggedAttractorIndex = -1;

    while (!WindowShouldClose())
    {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        canvasWidth = currentWidth - uiPanelWidth;
        int canvasHeight = currentHeight - 60;

        static Camera2D lastCamera = camera;
        if (camera.offset.x != lastCamera.offset.x || camera.offset.y != lastCamera.offset.y ||
            camera.target.x != lastCamera.target.x || camera.target.y != lastCamera.target.y ||
            camera.zoom != lastCamera.zoom) {
            needsGravityMapUpdate = true;
            lastCamera = camera;
        }

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
                        
                        InitialConditions s = InitialConditions::fromJson(j);
                        if (!s.world.attractors.empty()) {
                            world.attractors = s.world.attractors;
                            numAttractorsFloat = (float)world.attractors.size();
                        }
                        
                        startX = s.particleStartPos.x;
                        startY = s.particleStartPos.y;
                        velX = s.particleStartVel.x;
                        velY = s.particleStartVel.y;
                        velAngle = std::atan2(velY, velX) * 180.0f / PI;
                        velForce = std::sqrt(velX*velX + velY*velY);
                        
                        logThresholdFloat = std::log10(s.stopVelocityThreshold);
                        particleRadiusFloat = s.particleRadius;
                        
                        needsTraceUpdate = true;
                        needsGravityMapUpdate = true;
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
                    // Check attractors
                    for(int i = 0; i < world.attractors.size(); ++i) {
                        Vector2 pPos = {(float)world.attractors[i].position.x, (float)world.attractors[i].position.y};
                        if (CheckCollisionPointCircle(worldMouse, pPos, world.attractors[i].radius)) {
                            dragState = DragState::Attractor;
                            draggedAttractorIndex = i;
                            break;
                        }
                    }
                }
            }
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragState = DragState::None;
            draggedAttractorIndex = -1;
        }

        // Drag execution
        if (dragState == DragState::Particle || dragState == DragState::Attractor) {
            Vector2 clampedMouse = mousePos;
            if (clampedMouse.x < 0) clampedMouse.x = 0;
            if (clampedMouse.x > canvasWidth) clampedMouse.x = canvasWidth;
            if (clampedMouse.y < 0) clampedMouse.y = 0;
            if (clampedMouse.y > canvasHeight) clampedMouse.y = canvasHeight;
            Vector2 clampedWorldMouse = GetScreenToWorld2D(clampedMouse, camera);

            if (dragState == DragState::Particle) {
                startX = clampedWorldMouse.x;
                startY = clampedWorldMouse.y;
            } else if (draggedAttractorIndex != -1) {
                world.attractors[draggedAttractorIndex].position = {clampedWorldMouse.x, clampedWorldMouse.y};
                needsGravityMapUpdate = true;
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

        if (showGravityMap && needsGravityMapUpdate) {
            Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
            Vector2 bottomRight = GetScreenToWorld2D({(float)canvasWidth, (float)canvasHeight}, camera);
            
            float widthWorld = bottomRight.x - topLeft.x;
            float heightWorld = bottomRight.y - topLeft.y;
            float stepX = widthWorld / mapResX;
            float stepY = heightWorld / mapResY;
            
            Color* pixels = (Color*)gravityImage.data;
            
            // Pass 1: Calculate all potentials
            int totalPixels = mapResX * mapResY;
            std::vector<double> potentials(totalPixels);
            
            for (int y = 0; y < mapResY; ++y) {
                float worldY = topLeft.y + y * stepY;
                for (int x = 0; x < mapResX; ++x) {
                    float worldX = topLeft.x + x * stepX;
                    potentials[y * mapResX + x] = world.getGravityPotentialAt(Vector2D(worldX, worldY));
                }
            }
            
            // Pass 2: Find robust min and max using percentiles to ignore infinite spikes near centers
            std::vector<double> sortedPotentials = potentials;
            std::sort(sortedPotentials.begin(), sortedPotentials.end());
            
            // 2% and 95% percentiles
            // sortedPotentials[0] is the most negative (deepest well)
#if ENABLE_GRAVITY_DEBUG_KNOBS
            int pDeepIdx = (int)(totalPixels * dbgPercentileDeep);
            int pShallowIdx = (int)(totalPixels * dbgPercentileShallow);
            if (pDeepIdx >= totalPixels) pDeepIdx = totalPixels - 1;
            if (pShallowIdx >= totalPixels) pShallowIdx = totalPixels - 1;
            if (pDeepIdx < 0) pDeepIdx = 0;
            if (pShallowIdx < 0) pShallowIdx = 0;
            double deepest = sortedPotentials[pDeepIdx]; 
            double shallowest = sortedPotentials[pShallowIdx]; 
#else
            double deepest = sortedPotentials[(int)(totalPixels * 0.02)]; 
            double shallowest = sortedPotentials[(int)(totalPixels * 0.95)]; 
#endif
            
            // Prevent division by zero if the screen is totally flat
            if (std::abs(shallowest - deepest) < 1.0) {
                deepest = shallowest - 1.0;
            }
            
            // Pass 3: Map to colors
            for (int y = 0; y < mapResY; ++y) {
                for (int x = 0; x < mapResX; ++x) {
                    double p = potentials[y * mapResX + x];
                    
                    // Map potential between shallowest (0.0) and deepest (1.0)
                    float normalized = (float)((p - shallowest) / (deepest - shallowest));
                    if (normalized > 1.0f) normalized = 1.0f;
                    if (normalized < 0.0f) normalized = 0.0f;
                    
#if ENABLE_GRAVITY_DEBUG_KNOBS
                    normalized = std::pow(normalized, dbgContrastPower);
#endif
                    // Smoothstep for slightly richer contrast mid-tones
                    normalized = normalized * normalized * (3.0f - 2.0f * normalized);
                    
                    // Gradient: Deep space (dark/cyan) -> Neon Pink (deep well)
                    unsigned char r = (unsigned char)(normalized * 255);
                    unsigned char g = (unsigned char)((1.0f - normalized) * 40); 
                    unsigned char b = (unsigned char)((1.0f - normalized) * 150 + (normalized * 127)); 
#if ENABLE_GRAVITY_DEBUG_KNOBS
                    float alphaBase = 150.0f + normalized * 105.0f;
                    if (alphaBase > dbgMaxAlpha) alphaBase = dbgMaxAlpha;
                    unsigned char a = (unsigned char)alphaBase;
#else
                    unsigned char a = (unsigned char)(150 + normalized * 105); // Alpha from 150 to 255
#endif
                    
                    pixels[y * mapResX + x] = {r, g, b, a};
                }
            }
            UpdateTexture(gravityTexture, pixels);
            needsGravityMapUpdate = false;
        }
        
        if (needsTraceUpdate) {
            Vector2D particlePos(startX, startY);
            Vector2D particleVel(velX, velY);
            
            // 1. Query the engine for the true stop time 
            IntegratorResult result = sim.simulateUntilStop(world, particlePos, particleVel, 5000.0);
            
            // 2. Fetch the trace up to the timeout for visualization
            trace = sim.getTrace(world, particlePos, particleVel, 0.0, 5000.0);
            
            // 3. Mark the stop frame natively from the engine's result
            stopFrameIdx = -1;
            if (result.stopped) {
                stopFrameIdx = (int)result.timeElapsed;
            }
            
            needsTraceUpdate = false;
            needsEnergyUpdate = true;
        }

        if (showEnergyGraph && needsEnergyUpdate && !trace.empty()) {
            energyTimeline = EnergyDiagnostics::analyse(trace, world);
            needsEnergyUpdate = false;
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

            if (!showEnergyGraph) {
                if (showGravityMap) {
                    DrawTexturePro(gravityTexture, 
                                   {0, 0, (float)mapResX, (float)mapResY}, 
                                   {0, 0, (float)canvasWidth, (float)canvasHeight}, 
                                   {0, 0}, 0.0f, WHITE);
                }

                // --- DRAW CANVAS (Camera View) ---
                BeginMode2D(camera);
                    
                    // Draw origin axes lightly
                    DrawLine(-10000, 0, 10000, 0, gridLine);
                    DrawLine(0, -10000, 0, 10000, gridLine);

                    for (const auto& p : world.attractors) {
                        DrawCircle(p.position.x, p.position.y, p.radius, attractorFill); 
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
            } else {
                // --- DRAW ENERGY GRAPH ---
                if (!energyTimeline.snapshots.empty()) {
                    float graphX = 40.0f;
                    float graphY = 40.0f;
                    float graphW = (float)canvasWidth - 80.0f;
                    float graphH = (float)canvasHeight - 80.0f;
                    
                    double eMin = energyTimeline.minKE;
                    if (energyTimeline.minPE < eMin) eMin = energyTimeline.minPE;
                    if (energyTimeline.minTotal < eMin) eMin = energyTimeline.minTotal;
                    
                    double eMax = energyTimeline.maxKE;
                    if (energyTimeline.maxPE > eMax) eMax = energyTimeline.maxPE;
                    if (energyTimeline.maxTotal > eMax) eMax = energyTimeline.maxTotal;

                    double eRange = eMax - eMin;
                    if (eRange < 0.001) eRange = 1.0;
                    
                    eMin -= eRange * 0.05;
                    eRange *= 1.1; // padding
                    
                    DrawRectangle((int)graphX, (int)graphY, (int)graphW, (int)graphH, {0, 0, 0, 120});
                    DrawRectangleLinesEx({graphX, graphY, graphW, graphH}, 1, {255, 0, 127, 100});
                    
                    auto eToY = [&](double e) -> float {
                        return graphY + graphH - (float)((e - eMin) / eRange) * graphH;
                    };
                    
                    // Zero line
                    if (eMin < 0.0 && (eMin + eRange) > 0.0) {
                        float zeroY = eToY(0.0);
                        DrawLine((int)graphX, (int)zeroY, (int)(graphX + graphW), (int)zeroY, {255, 255, 255, 40});
                    }

                    size_t n = energyTimeline.snapshots.size();
                    
                    // Helper to draw a specific curve
                    auto drawCurve = [&](double (EnergySnapshot::*field), Color col) {
                        float prevX = graphX;
                        float prevY = eToY(energyTimeline.snapshots[0].*field);
                        for (float px = 1.0f; px <= graphW; px += 1.0f) {
                            size_t idx = (size_t)(px * (n - 1) / graphW);
                            if (idx >= n) idx = n - 1;
                            float curX = graphX + px;
                            float curY = eToY(energyTimeline.snapshots[idx].*field);
                            DrawLine((int)prevX, (int)prevY, (int)curX, (int)curY, col);
                            prevX = curX;
                            prevY = curY;
                        }
                    };
                    
                    drawCurve(&EnergySnapshot::potentialEnergy, neonCyan);
                    drawCurve(&EnergySnapshot::kineticEnergy, neonGreen);
                    drawCurve(&EnergySnapshot::totalEnergy, neonYellow);
                    
                    // Playhead
                    float playX = graphX + ((float)playbackFrame / (float)(n - 1)) * graphW;
                    DrawLine((int)playX, (int)graphY, (int)playX, (int)(graphY + graphH), {255, 255, 255, 180});
                    
                    // Legend
                    float legendX = graphX + 10;
                    float legendY = graphY + 10;
                    DrawRectangle((int)legendX - 5, (int)legendY - 5, 100, 70, {0, 0, 0, 150});
                    DrawText("KE",    (int)legendX + 20, (int)legendY,      10, neonGreen);
                    DrawRectangle((int)legendX, (int)legendY + 2,  10, 8, neonGreen);
                    DrawText("PE",    (int)legendX + 20, (int)legendY + 20, 10, neonCyan);
                    DrawRectangle((int)legendX, (int)legendY + 22, 10, 8, neonCyan);
                    DrawText("Total", (int)legendX + 20, (int)legendY + 40, 10, neonYellow);
                    DrawRectangle((int)legendX, (int)legendY + 42, 10, 8, neonYellow);
                    
                    int frameIdx = (int)playbackFrame;
                    if (frameIdx >= 0 && frameIdx < (int)n) {
                        DrawText(TextFormat("KE:%.2f  PE:%.2f  E:%.2f", 
                                 energyTimeline.snapshots[frameIdx].kineticEnergy, 
                                 energyTimeline.snapshots[frameIdx].potentialEnergy, 
                                 energyTimeline.snapshots[frameIdx].totalEnergy),
                                 (int)graphX + 10, (int)(graphY + graphH - 30), 20, {255, 255, 255, 160});
                    }
                } else {
                    DrawText("No Trace Data for Energy Graph", canvasWidth / 2 - 150, canvasHeight / 2, 20, neonPink);
                }
            }

            // --- DRAW RIGHT UI PANEL ---
            DrawRectangle(canvasWidth, 0, uiPanelWidth, currentHeight, uiBg);
            DrawLine(canvasWidth, 0, canvasWidth, currentHeight, neonPink);

            int panelX = canvasWidth + 20;
            int currentY = 20;
            
            DrawText("LEVEL CONTROLS", panelX, currentY, 20, neonCyan);
            currentY += 30;
            
            if (GuiToggle(Rectangle{(float)panelX, (float)currentY, 260, 25}, "Show Gravity Map", &showGravityMap)) {
                if (showGravityMap) needsGravityMapUpdate = true;
            }
            currentY += 35;
            
#if ENABLE_GRAVITY_DEBUG_KNOBS
            if (showGravityMap) {
                DrawText("GRAVITY DEBUG KNOBS", panelX, currentY, 10, neonYellow);
                currentY += 15;
                GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Deep %:");
                if (DrawFineSlider(Rectangle{(float)panelX + 70, (float)currentY, 170, 20}, NULL, TextFormat("%.3f", dbgPercentileDeep), &dbgPercentileDeep, 0.0f, 0.5f, 0.01f)) needsGravityMapUpdate = true;
                currentY += 25;
                
                GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Shallow %:");
                if (DrawFineSlider(Rectangle{(float)panelX + 70, (float)currentY, 170, 20}, NULL, TextFormat("%.3f", dbgPercentileShallow), &dbgPercentileShallow, 0.5f, 1.0f, 0.01f)) needsGravityMapUpdate = true;
                currentY += 25;
                
                GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Curve Pow:");
                if (DrawFineSlider(Rectangle{(float)panelX + 70, (float)currentY, 170, 20}, NULL, TextFormat("%.2f", dbgContrastPower), &dbgContrastPower, 0.1f, 3.0f, 0.1f)) needsGravityMapUpdate = true;
                currentY += 25;
                
                GuiLabel(Rectangle{(float)panelX, (float)currentY, 80, 20}, "Max Alpha:");
                if (DrawFineSlider(Rectangle{(float)panelX + 70, (float)currentY, 170, 20}, NULL, TextFormat("%.0f", dbgMaxAlpha), &dbgMaxAlpha, 0.0f, 255.0f, 5.0f)) needsGravityMapUpdate = true;
                currentY += 35;
            }
#endif
            
            GuiLabel(Rectangle{(float)panelX, (float)currentY, 120, 20}, "Attractor Count:");
            if (DrawFineSlider(Rectangle{(float)panelX + 100, (float)currentY, 140, 20}, NULL, TextFormat("%d", (int)numAttractorsFloat), &numAttractorsFloat, 1, 30, 1.0f)) {}
            currentY += 30;
            
            if (GuiButton(Rectangle{(float)panelX, (float)currentY, 260, 30}, "Generate Random World")) {
                world = World((int)numAttractorsFloat, canvasWidth, canvasHeight);
                needsTraceUpdate = true;
                needsGravityMapUpdate = true;
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

            {
                bool prev = showEnergyGraph;
                GuiToggle(Rectangle{(float)panelX, (float)currentY, 260, 25}, "Energy Graph (Diagnostics)", &showEnergyGraph);
                if (showEnergyGraph && !prev) needsEnergyUpdate = true;
            }
            currentY += 40;
            
            DrawLine(panelX, currentY, panelX + 260, currentY, neonPink);
            currentY += 20;

            // --- MINIMAP ---
            DrawText("MINIMAP", panelX, currentY, 20, neonCyan);
            
            DrawRectangleRec(minimapRect, {15, 0, 30, 255});
            DrawRectangleLinesEx(minimapRect, 1, neonPink);

            // Draw attractors on minimap
            for (const auto& p : world.attractors) {
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
            if (GuiButton(Rectangle{(float)panelX, (float)currentHeight - 75, 260, 30}, "Export Variables")) {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                
                std::filesystem::path exeDir = GetApplicationDirectory();
                std::filesystem::path exportDir = exeDir / ".." / "exports";
                std::error_code ec;
                std::filesystem::create_directories(exportDir, ec);
                
                std::stringstream ssFilename;
                ssFilename << "export_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << ".json";
                std::string filename = (exportDir / ssFilename.str()).string();
                
                InitialConditions s;
                s.world = world;
                s.particleStartPos = {startX, startY};
                s.particleStartVel = {velX, velY};
                s.particleRadius = particleRadiusFloat;
                s.stopVelocityThreshold = actualStopThreshold;
                
                nlohmann::json j = s.toJson();
                
                std::ofstream out(filename);
                out << j.dump(4);
                out.close();
                
                exportMessage = "Exported to exports/" + ssFilename.str();
                exportMessageTimer = 3.0f;
            }
            if (GuiButton(Rectangle{(float)panelX, (float)currentHeight - 40, 260, 30}, "Export Graph CSV")) {
                if (!energyTimeline.snapshots.empty()) {
                    auto now = std::chrono::system_clock::now();
                    auto in_time_t = std::chrono::system_clock::to_time_t(now);
                    
                    std::filesystem::path exeDir = GetApplicationDirectory();
                    std::filesystem::path exportDir = exeDir / ".." / "exports";
                    std::error_code ec;
                    std::filesystem::create_directories(exportDir, ec);
                    
                    std::stringstream ssFilename;
                    ssFilename << "energy_graph_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << ".csv";
                    std::string filename = (exportDir / ssFilename.str()).string();
                    
                    std::ofstream out(filename);
                    out << "Frame,Time,KineticEnergy,PotentialEnergy,TotalEnergy\n";
                    for (size_t i = 0; i < energyTimeline.snapshots.size(); ++i) {
                        const auto& snap = energyTimeline.snapshots[i];
                        out << i << "," << snap.time << "," << snap.kineticEnergy << "," << snap.potentialEnergy << "," << snap.totalEnergy << "\n";
                    }
                    out.close();
                    
                    exportMessage = "Exported to exports/" + ssFilename.str();
                    exportMessageTimer = 3.0f;
                } else {
                    exportMessage = "No graph data to export!";
                    exportMessageTimer = 3.0f;
                }
            }
            DrawText("Drag & Drop a .json file anywhere to import", panelX, currentHeight - 105, 10, {255, 0, 127, 200});
            if (exportMessageTimer > 0) {
                DrawText(exportMessage.c_str(), panelX, currentHeight - 90, 10, neonGreen);
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
