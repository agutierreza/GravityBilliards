cmake_minimum_required(VERSION 3.15)
project(GravityBilliardsEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()



# Library
add_library(GravityBilliardsPhysics
    src/Attractor.cpp
    src/World.cpp
    src/EulerIntegrator.cpp
    src/CircleCollisionDetector.cpp
    src/InelasticCollisionResolver.cpp
    src/InitialConditions.cpp
    src/EnergyDiagnostics.cpp
)

target_include_directories(GravityBilliardsPhysics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
# UI Tool
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/master.tar.gz
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(GravityBilliardsUI src/ui_main.cpp)
target_link_libraries(GravityBilliardsUI PRIVATE GravityBilliardsPhysics raylib)
# CLI Tool
add_executable(GravityBilliardsCLI test/main.cpp)
target_link_libraries(GravityBilliardsCLI PRIVATE GravityBilliardsPhysics)

# Tests
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(GravityBilliardsTests
    test/test_euler_integrator.cpp
    test/test_collision_detector.cpp
    test/test_collision_resolver.cpp
    test/test_e2e_scenarios.cpp
)
target_link_libraries(GravityBilliardsTests PRIVATE GravityBilliardsPhysics gtest_main)

include(GoogleTest)
gtest_discover_tests(GravityBilliardsTests)


find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME "Space Golf Physics Engine")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    
    doxygen_add_docs(GravityBilliardsDocs
        include/GravityBilliards
        src
        ALL
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

