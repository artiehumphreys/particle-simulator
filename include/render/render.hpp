#pragma once

#include "common.hpp"
#include "particle.hpp"
#include "physics.hpp"

namespace render {
class Renderer {
public:
  Renderer(PhysicsEngine &engine_, int width_, int height_)
      : engine(engine_), width(width_), height(height_) {};

  void drawParticles();

  bool isOpen();

  void pollEvent(sf::Event &event);

private:
  PhysicsEngine &engine;
  int32_t width;
  int32_t height;
  sf::RenderWindow window;
  sf::CircleShape shape;
};
} // namespace render
