#include "common.hpp"

struct Particle {
    vec2 position = {0.0f, 0.0f};
    vec2 lastPosition = {0.0f, 0.0f};
    vec2 acceleration  = {0.0f, 0.0f};

    c color;

    Particle() = default;

    Particle(float x, float y) : position {x, y}, lastPosition{x, y} {}

    void setPosition (vec2 newPosition) {
        lastPosition = position;
        position = newPosition;
    }

    void update(float timeDiff) {
        // formula: 2 * current position - previous position + acceleration * time difference ** 2
        // implicitly account for velocity
        vec2 newPosition = 2.0f * position - lastPosition + acceleration * (timeDiff * timeDiff);
        setPosition(newPosition);
        acceleration = {0.0f, 0.0f};
    }
};
