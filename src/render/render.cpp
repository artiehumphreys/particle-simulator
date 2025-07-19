#include "render/render.hpp"

namespace render {
Renderer::Renderer(PhysicsEngine &engine_, int32_t width_, int32_t height_)
    : engine(engine_), width(width_), height(height_),
      window(sf::VideoMode(width, height), "Particle Simulator"), shape(4.0f) {
  window.setFramerateLimit(60);
  shape.setOrigin(4, 4);
  shape.setFillColor(sf::Color::Cyan);
}

void Renderer::drawParticles() {
  window.clear(sf::Color::Black);
  for (const Particle &p : engine.particles) {
    shape.setPosition(p.position.x, p.position.y);
    window.draw(shape);
  }
  window.display();
}

bool Renderer::isOpen() const { return window.isOpen(); }

bool Renderer::pollEvent(sf::Event &event) { return window.pollEvent(event); }

} // namespace render
