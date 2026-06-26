#include "GravityBilliards/EulerIntegrator.hpp"

namespace GravityBilliards {

void EulerIntegrator::integrate(Vector2D& pos, Vector2D& vel, const Vector2D& accel, double dt) const {
    vel += accel * dt;
    pos += vel * dt;
}

} // namespace GravityBilliards
