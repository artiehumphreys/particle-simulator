#pragma once

#include "common.hpp"
#include "particle.hpp"
#include "physics.hpp"

namespace render {
class Renderer {
public:
  Renderer(PhysicsEngine &_engine, int _width, int _height)
      : engine(_engine), width(_width), height(_height) {};

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
