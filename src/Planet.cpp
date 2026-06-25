#include "SpaceGolf/Planet.hpp"

namespace SpaceGolf {

Planet::Planet() : mass(1), position({0.0, 0.0}) {
    calculateRadius();
}

Planet::Planet(int mass) : mass(mass), position({0.0, 0.0}) {
    calculateRadius();
}

Planet::Planet(int mass, Vector2D position) : mass(mass), position(position) {
    calculateRadius();
}

void Planet::calculateRadius() {
    radius = 4.0 * std::sqrt(static_cast<double>(mass));
}

} // namespace SpaceGolf
