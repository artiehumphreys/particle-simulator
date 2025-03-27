#pragma once

#include "common.hpp"
#include "grid.hpp"
#include "particle.hpp"

struct PhysicsEngine {
  Grid<Particle> collisionGrid;
  int32_t width, height;
  int8_t sub_steps = 8; // collision resolution

  PhysicsEngine(int32_t width_, int32_t height_)
      : collisionGrid{width_, height_}, width(width_), height(height_) {}

  void solveCollision(Particle& p1, Particle& p2) {
    // idea: take the difference between their summed radii and actual difference between them.
    // shift them away along axis of intersection difference / 2 units
    const float expectedDistance = static_cast<float>(p1.radius + p2.radius);
    vec2 axis = p1.position - p2.position;
    const float squaredDistance = axis.x * axis.x + axis.y * axis.y;

    if (squaredDistance < expectedDistance * expectedDistance && squaredDistance > 0) {
      const float actualDistance = std::sqrt(squaredDistance);
      const float overlap = expectedDistance - actualDistance;

      float delta = overlap / 2.0f;

      vec2 normal = axis / actualDistance;

      p1.position += normal * delta;
      p2.position += normal * delta;
    }
  }
};
