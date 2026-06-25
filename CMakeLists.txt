cmake_minimum_required(VERSION 3.15)
project(SpaceGolfEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# Library
add_library(SpaceGolfPhysics
    src/Planet.cpp
    src/Level.cpp
    src/Simulation.cpp
)

target_include_directories(SpaceGolfPhysics PUBLIC
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

add_executable(SpaceGolfUI src/ui_main.cpp)
target_link_libraries(SpaceGolfUI PRIVATE SpaceGolfPhysics raylib)
# CLI Tool
add_executable(SpaceGolfCLI test/main.cpp)
target_link_libraries(SpaceGolfCLI PRIVATE SpaceGolfPhysics)


find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME "Space Golf Physics Engine")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    
    doxygen_add_docs(SpaceGolfDocs
        include/SpaceGolf
        src
        ALL
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

