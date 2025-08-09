#pragma once

#include "common.hpp"

struct Particle {
  vec2 position = {0.0f, 0.0f};
  vec2 lastPosition = {0.0f, 0.0f};
  vec2 acceleration = {0.0f, 0.0f};
  uint8_t radius = 4;
  uint32_t id;

  // unique id for each particle
  inline static uint32_t nextID = 0;

  Particle() = default;

  Particle(float x, float y)
      : position{x, y}, lastPosition{x, y}, id(nextID++) {}

  void setPosition(vec2 newPosition) {
    lastPosition = position;
    position = newPosition;
  }

  void clampPosition(float maxX, float maxY) {
    float r = static_cast<float>(radius);

    constexpr float dampening = 0.8f;
    vec2 v = position - lastPosition;

    if (position.x < r) {
      position.x = r;
      lastPosition.x = position.x + v.x * dampening;
    } else if (position.x > maxX - r) {
      position.x = maxX - r;
      lastPosition.x = position.x + v.x * dampening;
    }

    if (position.y < r) {
      position.y = r;
      lastPosition.y = position.y + v.y * dampening;
    } else if (position.y > maxY - r) {
      position.y = maxY - r;
      lastPosition.y = position.y + v.y * dampening;
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
