#include "render/Renderer.hpp"

namespace render {
Renderer::Renderer(PhysicsEngine &engine_, int32_t width_, int32_t height_)
    : engine(_engine), width(_width), height(height_),
      window(sf::videoMode(width, height), "Particle Simulator"), shape(1.0f) {
  window.setFrameLimit(60);
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

void Renderer::pollEvent(sf::Event &event) { return window.pollEvent(event); }

} // namespace render
