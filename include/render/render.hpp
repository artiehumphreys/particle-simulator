#pragma once

#include "common.hpp"
#include "particle.hpp"
#include "physics.hpp"
#include <cstdint>

namespace render {
class Renderer {
public:
  Renderer(PhysicsEngine &engine_, uint32_t width_, uint32_t height_);

  void drawParticles();

  bool isOpen() const;

  bool pollEvent(sf::Event &event);

private:
  PhysicsEngine &engine;
  uint32_t width;
  uint32_t height;
  sf::RenderWindow window;
  sf::CircleShape shape;
};
} // namespace render
