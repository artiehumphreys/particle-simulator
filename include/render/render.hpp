#pragma once

#include "particle.hpp"
#include "physics.hpp"
#include <SMFL/Graphics.hpp>
#include <cstdint>

namespace render {
class Renderer {
  int32_t width;
  int32_t height;

  PhysicsEngine &engine;

  Renderer(PhysicsEngine &_engine) : engine(_engine) {};
};
} // namespace render
