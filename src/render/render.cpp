#include "render/render.hpp"
#include <cstdint>

namespace render {
Renderer::Renderer(PhysicsEngine &engine_, uint32_t width_, uint32_t height_)
    : engine(engine_), width(width_), height(height_),
      window(sf::VideoMode(width, height), "Particle Simulator"),
      vertices(sf::Quads) {
  // window.setFramerateLimit(60);
  window.setVerticalSyncEnabled(false);
}

void Renderer::drawParticles() {
  window.clear(sf::Color::Black);

  vertices.resize(engine.hot.size() * 4);
  std::size_t v = 0;
  for (const Hot &p : engine.hot) {
    constexpr float r = PARTICLE_RADIUS;
    const float x = p.position.x, y = p.position.y;
    vertices[v + 0] = sf::Vertex({x - r, y - r}, sf::Color::Cyan);
    vertices[v + 1] = sf::Vertex({x + r, y - r}, sf::Color::Cyan);
    vertices[v + 2] = sf::Vertex({x + r, y + r}, sf::Color::Cyan);
    vertices[v + 3] = sf::Vertex({x - r, y + r}, sf::Color::Cyan);
    v += 4;
  }

  window.draw(vertices); // one draw call for all particles
  window.display();
}

bool Renderer::isOpen() const { return window.isOpen(); }

bool Renderer::pollEvent(sf::Event &event) { return window.pollEvent(event); }

} // namespace render
