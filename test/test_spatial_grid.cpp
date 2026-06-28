#include <gtest/gtest.h>
#include "GravityBilliards/SpatialGrid.hpp"
#include "GravityBilliards/Vector2D.hpp"
#include <vector>
#include <algorithm>

using namespace GravityBilliards;

TEST(SpatialGridTest, InsertAndQuery) {
    SpatialGrid<int> grid(100.0); // Cell size of 100x100

    // Insert entity ID 42 at (50, 50) with radius 10
    grid.insert(42, Vector2D(50.0, 50.0), 10.0);

    // Query near it
    std::vector<int> results = grid.getPotentialCollisions(Vector2D(50.0, 50.0), 10.0);
    EXPECT_EQ(results.size(), 1);
    if (!results.empty()) {
        EXPECT_EQ(results[0], 42);
    }

    // Query far away
    std::vector<int> emptyResults = grid.getPotentialCollisions(Vector2D(500.0, 500.0), 10.0);
    EXPECT_TRUE(emptyResults.empty());
}

TEST(SpatialGridTest, BoundaryOverlap) {
    SpatialGrid<int> grid(100.0);

    // Entity exactly on the boundary of (0,0) and (100,0) cell, at x=100.
    // With radius 10, it should be in cell X=0 and cell X=1.
    grid.insert(99, Vector2D(100.0, 50.0), 10.0);

    // Query exclusively in cell X=0
    auto results1 = grid.getPotentialCollisions(Vector2D(50.0, 50.0), 10.0);
    EXPECT_TRUE(std::find(results1.begin(), results1.end(), 99) != results1.end());

    // Query exclusively in cell X=1
    auto results2 = grid.getPotentialCollisions(Vector2D(150.0, 50.0), 10.0);
    EXPECT_TRUE(std::find(results2.begin(), results2.end(), 99) != results2.end());
}

TEST(SpatialGridTest, NegativeCoordinates) {
    SpatialGrid<int> grid(100.0);

    // Insert at deeply negative coordinates to ensure hash function doesn't fail
    grid.insert(77, Vector2D(-150.0, -150.0), 10.0);

    // Query at negative coords
    auto results = grid.getPotentialCollisions(Vector2D(-150.0, -150.0), 10.0);
    EXPECT_EQ(results.size(), 1);
    if (!results.empty()) {
        EXPECT_EQ(results[0], 77);
    }

    // Query slightly off
    auto resultsOff = grid.getPotentialCollisions(Vector2D(-50.0, -50.0), 10.0);
    EXPECT_TRUE(resultsOff.empty());
}

TEST(SpatialGridTest, Clear) {
    SpatialGrid<int> grid(100.0);
    grid.insert(1, Vector2D(50.0, 50.0), 10.0);
    
    // Should have 1
    EXPECT_EQ(grid.getPotentialCollisions(Vector2D(50.0, 50.0), 10.0).size(), 1);

    grid.clear();

    // Should be empty now
    EXPECT_TRUE(grid.getPotentialCollisions(Vector2D(50.0, 50.0), 10.0).empty());
}
