#pragma once

#include "common.hpp"

struct Particle {
  vec2 position = {0.0f, 0.0f};
  vec2 lastPosition = {0.0f, 0.0f};
  vec2 acceleration = {0.0f, 0.0f};
  int8_t radius = 1;
  int id;

  // unique id for each particle
  inline static int nextID = 0;

  c color;

  Particle() = default;

  Particle(float x, float y)
      : position{x, y}, lastPosition{x, y}, id(nextID++) {}

  void setPosition(vec2 newPosition) {
    lastPosition = position;
    position = newPosition;
  }

  void clampPosition(float maxX, float maxY) {
    if (position.x < radius) {
      position.x = static_cast<float>(radius);
      lastPosition.x = position.x;
    } else if (position.x > maxX - radius) {
      position.x = maxX - radius;
      lastPosition.x = position.x;
    }

    if (position.y < radius) {
      position.y = static_cast<float>(radius);
      lastPosition.y = position.y;
    } else if (position.y > maxY - radius) {
      position.y = maxY - radius;
      lastPosition.y = position.y;
    }
  }

  void update(float timeDiff) {
    // formula: 2 * current position - previous position + acceleration * time
    // difference ** 2 implicitly account for velocity
    vec2 newPosition =
        2.0f * position - lastPosition + acceleration * (timeDiff * timeDiff);
    setPosition(newPosition);
    acceleration = {0.0f, 0.0f};
  }
};
