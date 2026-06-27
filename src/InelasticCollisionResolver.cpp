#include "GravityBilliards/InelasticCollisionResolver.hpp"

namespace GravityBilliards {

void InelasticCollisionResolver::resolve(const CollisionInfo& info, Vector2D& pos, Vector2D& vel, double bounceDamping, double dt) const {
    // 1. Position correction (push out to prevent sticking)
    pos += info.normal * info.penetration;

    // 2. Velocity resolution (inelastic bounce)
    Vector2D pOrtog{-info.normal.y, info.normal.x};
    
    double proyX = vel.dot(info.normal);
    double proyY = vel.dot(pOrtog);
    
    // Restitution threshold: if impact is very slow, do not bounce to allow sleeping
    if (proyX < 0.0 && proyX > -0.5) {
        proyX = 0.0;
    } else {
        proyX = -proyX / bounceDamping;
    }
    
    Vector2D Ux = info.normal * proyX;
    Vector2D Uy = pOrtog * (proyY / bounceDamping);
    
    vel = Ux + Uy;
    
    // 3. Integrate position over the remaining time step
    pos += vel * dt;
}

} // namespace GravityBilliards
