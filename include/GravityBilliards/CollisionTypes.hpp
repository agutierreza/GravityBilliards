#pragma once

#include "GravityBilliards/Vector2D.hpp"
#include <optional>

namespace GravityBilliards {

/**
 * @struct CollisionInfo
 * @brief Holds information about a detected collision.
 */
struct CollisionInfo {
    Vector2D normal;       ///< The normalized direction pointing out from the surface of impact.
    double penetration;    ///< The depth of penetration, used to resolve sticking.
    double surfaceBounciness = 1.0; ///< The local bounciness of the hit object.
};

} // namespace GravityBilliards
