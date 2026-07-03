#include <gtest/gtest.h>
#include "GameOrchestrator.hpp"
#include "GravityBilliards/EulerIntegrator.hpp"
#include "GravityBilliards/CircleCollisionDetector.hpp"

using namespace GravityBilliards;
using namespace GravityGame;

TEST(GameOrchestratorTest, PhysicsDelegation) {
    // 1. Create a simple world with one planet
    World world;
    Attractor planet(100.0, 10.0, Vector2D(0.0, 0.0));
    world.attractors.push_back(planet);

    // 2. Start the player at (20, 0) falling towards the planet
    Vector2D startPos(20.0, 0.0);
    GameOrchestrator orchestrator(world, startPos);
    
    // Set a predictable dt
    orchestrator.physicsEngine.dt = 1.0;
    
    // Initial velocity is (0,0). Gravity from (0,0) with mass 100 on (20,0) is:
    // dist = 20
    // force = 100 / (20*20) = 0.25 towards origin (so accel = -0.25 on X)
    
    // Engine step will:
    // v += a * dt = (0,0) + (-0.25, 0) * 1 = (-0.25, 0)
    // p += v * dt = (20,0) + (-0.25, 0) * 1 = (19.75, 0)
    
    orchestrator.update();
    
    EXPECT_NEAR(orchestrator.player.velocity.x, -0.25, 1e-9);
    EXPECT_NEAR(orchestrator.player.velocity.y, 0.0, 1e-9);
    EXPECT_NEAR(orchestrator.player.position.x, 19.75, 1e-9);
    EXPECT_NEAR(orchestrator.player.position.y, 0.0, 1e-9);
}

TEST(GameOrchestratorTest, ThrustApplication) {
    World world;
    Vector2D startPos(0.0, 0.0);
    GameOrchestrator orchestrator(world, startPos);
    orchestrator.physicsEngine.dt = 1.0;
    
    // Apply a thrust vector before update
    orchestrator.playerThrust = Vector2D(5.0, -2.0);
    
    // Update should apply thrust to velocity
    orchestrator.update();
    
    // vel += thrust * dt = (5, -2)
    // pos += vel * dt = (5, -2)
    
    EXPECT_NEAR(orchestrator.player.velocity.x, 5.0, 1e-9);
    EXPECT_NEAR(orchestrator.player.velocity.y, -2.0, 1e-9);
    EXPECT_NEAR(orchestrator.player.position.x, 5.0, 1e-9);
    EXPECT_NEAR(orchestrator.player.position.y, -2.0, 1e-9);
}

TEST(GameOrchestratorTest, CollectibleInteraction) {
    World world;
    Vector2D startPos(0.0, 0.0);
    GameOrchestrator orchestrator(world, startPos);
    orchestrator.physicsEngine.dt = 1.0;
    
    // Create a collectible right in front of the player
    GameEntity col;
    col.position = Vector2D(10.0, 0.0);
    col.radius = 5.0;
    col.scoreValue = 100;
    orchestrator.entities.push_back(col);
    
    // Player radius is 5.0 (so they touch if distance <= 10.0)
    orchestrator.player.radius = 5.0;
    
    // Thrust player into the collectible
    orchestrator.playerThrust = Vector2D(5.0, 0.0); // moves to (5,0)
    
    // (5,0) to (10,0) is distance 5.0. 5.0 <= 10.0 (sum of radii), so they intersect.
    orchestrator.update();
    
    EXPECT_EQ(orchestrator.entities[0].active, false); // Collectible collected
    EXPECT_EQ(orchestrator.score, 100);
}
