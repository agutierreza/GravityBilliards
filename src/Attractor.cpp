#include "GravityBilliards/Attractor.hpp"

namespace GravityBilliards {

Attractor::Attractor() : mass(1.0), radius(4.0), position({0.0, 0.0}) {}

Attractor::Attractor(int mass) : mass(static_cast<double>(mass)), position({0.0, 0.0}) {
    radius = 4.0 * std::sqrt(this->mass);
}

Attractor::Attractor(int mass, Vector2D position) : mass(static_cast<double>(mass)), position(position) {
    radius = 4.0 * std::sqrt(this->mass);
}

Attractor::Attractor(double m, double r, const Vector2D& pos) : mass(m), radius(r), position(pos) {}

} // namespace GravityBilliards
