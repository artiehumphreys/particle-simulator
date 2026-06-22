#pragma once

#include "common.hpp"
#include <cstdint>

inline constexpr float PARTICLE_RADIUS = 4.0f;

// solveCollision touches only the position.
struct Hot {
  vec2 position = {0.0f, 0.0f};
};

// Cold half. Index-aligned with the Hot array.
struct Particle {
  vec2 lastPosition = {0.0f, 0.0f};
  vec2 acceleration = {0.0f, 0.0f};
  uint32_t id = 0;

  inline static uint32_t nextID = 0;
};
