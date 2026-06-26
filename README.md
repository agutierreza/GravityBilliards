# Gravity Billiards (Core Engine)

A high-performance C++ 2D physics engine designed specifically for calculating gravity, orbital trajectories, and inelastic collisions. 

This engine is the modern C++ reincarnation of the [SpaceGolfSimulator](https://github.com/agutierreza/SpaceGolfSimulator) originally developed in 2013. The core physics logic has been completely extracted, refactored into a highly-modular architecture, and optimized with zero-allocation trace generation. It now serves as the foundational sandbox and engine for future N-body projects, orbital games, or synthwave physics experiments.

## Project Structure

The project builds three separate targets:

1. **`SpaceGolfPhysics` (Static Library)**
   The headless core engine. It contains the pure math and logic for `Vector2D`, `Attractor`, `World`, and `Integrator`. You can link this library directly into your own video games or applications.
2. **`SpaceGolfCLI` (Command Line Interface)**
   A terminal-based diagnostic tool. It loads JSON configurations to simulate physics initial conditions headless, generating automated SVGs and traces for regression testing.
3. **`SpaceGolfUI` (Graphical Sandbox)**
   An interactive Raylib visual debugger and playground. It provides a synthwave-themed UI to tweak vectors, masses, and thresholds in real-time, functioning primarily as a visual "scrubber" and testing tool for the core physics math.

## Building the Project

The project uses CMake and requires a C++17 compatible compiler. 

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

*Note: The UI application relies on `raylib`. The CMake script will automatically download and build `raylib` from source in the background during your first build.*

### Building API Documentation

API documentation is automatically built using Doxygen. 
Ensure Doxygen is installed on your system and available in your PATH. CMake will automatically locate it and configure the documentation target.

Once the build is complete, you can view the interactive HTML documentation by opening:
`SpaceGolfEngine/docs/html/index.html`

## Running the UI Sandbox

Launch the UI executable to interact with the engine visually.

```bash
cd build
./SpaceGolfUI.exe
```

### UI Features:
- **Timeline Scrubber**: Use the playback slider at the bottom of the screen to scrub back and forth through a particle's trajectory to visually debug collision and stop states.
- **Drag-and-Drop JSON Import**: Drag any `.json` physics state file from your computer directly onto the window to instantly load those initial conditions.
- **Diagnostics Export**: Click "Export Variables" to serialize the current integrator state into a JSON file in the `exports/` folder, which can be fed directly to the CLI runner.
- **Dynamic Camera**: Right-click to pan around the scene, and use the mouse wheel to zoom in to micro-worlds. A minimap is provided in the bottom-right for navigation.
