#pragma once

#include "common.hpp"

struct Particle {
  vec2 position = {0.0f, 0.0f};
  vec2 lastPosition = {0.0f, 0.0f};
  vec2 acceleration = {0.0f, 0.0f};
  int8_t radius = 4;
  int id;

  // unique id for each particle
  inline static int nextID = 0;

  Particle() = default;

  Particle(float x, float y)
      : position{x, y}, lastPosition{x, y}, id(nextID++) {}

  void setPosition(vec2 newPosition) {
    lastPosition = position;
    position = newPosition;
  }

  void clampPosition(float maxX, float maxY) {
    float diam = static_cast<float>(radius * 2);
    position.x = fmax(fmin(position.x, maxX - diam), diam);
    position.y = fmax(fmin(position.y, maxY - diam), diam);
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
