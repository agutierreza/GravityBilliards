# Gravity Billiards (Core Engine)

A high-performance C++ 2D physics engine designed specifically for calculating gravity, orbital trajectories, and inelastic collisions. 

This engine is the modern C++ reincarnation of the [GravityBilliardsSimulator](https://github.com/agutierreza/GravityBilliardsSimulator) originally developed in 2013. The core physics logic has been completely extracted, refactored into a highly-modular architecture, and optimized with zero-allocation trace generation. It now serves as the foundational sandbox and engine for future N-body projects, orbital games, or synthwave physics experiments.

## Core Architecture

### Dual Static/Dynamic Polymorphism
The heart of the physics system is a header-only template class (`PhysicsEngine<TIntegrator, TDetector, TResolver>`). This provides two massive advantages depending on your use case:
- **Pure Static Dispatch (Maximum Performance):** When instantiated with raw concrete classes, the compiler inlines all math operations. This eliminates virtual function overhead, making batch trace generation for visualizers blazing fast.
- **Dynamic Type Erasure (Maximum Flexibility):** The library provides `DynamicWrappers` which encapsulate `std::shared_ptr` interfaces. Games can inject these wrappers into the static template to hot-swap collision and integration logic at runtime, without duplicating the engine core.

### Data-Oriented Containers
Physics entities (like `Attractor`) are built as pure, simple data structures rather than complex polymorphic hierarchies. The engine prioritizes cache-friendly data structs with decoupled properties (mass, radius, surface bounciness, friction) to ensure simplicity and raw speed.

## Project Structure

The project builds three separate targets:

1. **`GravityBilliardsPhysics` (Static Library)**
   The headless core engine. It contains the math and logic for `Vector2D`, `Attractor`, `World`, `PhysicsEngine`, and the collision interfaces. You can link this library directly into your own video games or applications.
2. **`GravityBilliardsCLI` (Command Line Interface)**
   A terminal-based diagnostic tool. It loads JSON configurations to simulate physics initial conditions headless, generating automated SVGs, traces, and performance benchmarks for regression testing.
3. **`GravityBilliardsUI` (Graphical Sandbox)**
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
`GravityBilliardsEngine/docs/html/index.html`

## Testing

The project includes a comprehensive testing suite powered by GoogleTest, containing both **Unit Tests** (for the core math and physics integrations) and **End-to-End (E2E) Tests** (for the scenarios). Right now, we run all our tests locally...

To run the tests, navigate to your build directory and run:

```bash
cmake --build . --config Debug --target GravityBilliardsTests
ctest --output-on-failure
```

While the Unit Tests are strict, the E2E tests are very relaxed on purpose. Their main point is simply to mathematically verify the deterministic simulations so that you can load them up in the UI and confidently check that what you would expect is what actually happens. Building these E2E physics tests was a lot of fun, and they look super cool when you scrub through them in the GUI!

## Running the UI Sandbox

Launch the UI executable to interact with the engine visually.

```bash
cd build
./GravityBilliardsUI.exe
```

### UI Features:
- **Timeline Scrubber**: Use the playback slider at the bottom of the screen to scrub back and forth through a particle's trajectory to visually debug collision and stop states.
- **Drag-and-Drop JSON Import**: Drag any `.json` physics state file from your computer directly onto the window to instantly load those initial conditions.
- **Diagnostics Export**: Click "Export Variables" to serialize the current integrator state into a JSON file in the `exports/` folder, which can be fed directly to the CLI runner.
- **Dynamic Camera**: Right-click to pan around the scene, and use the mouse wheel to zoom in to micro-worlds. A minimap is provided in the bottom-right for navigation.
