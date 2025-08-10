#pragma once

#include "common.hpp"
#include "particle.hpp"
#include "physics.hpp"

namespace render {
class Renderer {
public:
  Renderer(PhysicsEngine &engine_, int width_, int height_);

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
