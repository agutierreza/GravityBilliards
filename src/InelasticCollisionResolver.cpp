#include "GravityBilliards/InelasticCollisionResolver.hpp"

namespace GravityBilliards {

void InelasticCollisionResolver::resolve(const CollisionInfo& info, Vector2D& pos, Vector2D& vel, const Vector2D& accel, double bounceDamping, double dt) const {
    // 1. Position correction (push out to prevent sticking)
    pos += info.normal * info.penetration;

    // 2. Velocity resolution (inelastic bounce)
    Vector2D pOrtog{-info.normal.y, info.normal.x};
    
    double proyX = vel.dot(info.normal);
    double proyY = vel.dot(pOrtog);
    
    // Calculate the amount of velocity added INTO the surface this frame by gravity
    double gravityNormalVel = accel.dot(info.normal) * dt;
    // A small epsilon margin to account for floating point errors
    double sleepThreshold = gravityNormalVel - 0.1;
    
    // Restitution threshold: dynamically calculated based on actual gravity!
    if (proyX < 0.0 && proyX >= sleepThreshold) {
        proyX = 0.0;
    } else {
        proyX = (-proyX * bounceDamping) * info.surfaceBounciness;
    }
    
    Vector2D Ux = info.normal * proyX;
    Vector2D Uy = pOrtog * ((proyY * bounceDamping) * info.surfaceBounciness);
    
    vel = Ux + Uy;
}

} // namespace GravityBilliards
