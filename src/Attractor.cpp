#include "SpaceGolf/Attractor.hpp"

namespace SpaceGolf {

Attractor::Attractor() : mass(1), position({0.0, 0.0}) {
    calculateRadius();
}

Attractor::Attractor(int mass) : mass(mass), position({0.0, 0.0}) {
    calculateRadius();
}

Attractor::Attractor(int mass, Vector2D position) : mass(mass), position(position) {
    calculateRadius();
}

void Attractor::calculateRadius() {
    radius = 4.0 * std::sqrt(static_cast<double>(mass));
}

} // namespace SpaceGolf
