#include "physics.hpp"
#include "render/render.hpp"
#include "thread_pool/thread_pool.hpp"
#include <cstdint>
#include <cstdlib>

int main() {
  sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  uint32_t width = desktop.width;
  uint32_t height = desktop.height - 54;
  tp::ThreadPool pool(0);
  PhysicsEngine engine(width, height, pool);
  render::Renderer renderer(engine, width, height);

  for (int i = 0; i < 4000; ++i)
    engine.addParticle(
        {float(std::rand() % width), float(std::rand() % height)});

  const float step = 1 / 60.f;
  sf::Clock clock;
  float acc = 0.f;

  while (renderer.isOpen()) {
    sf::Event e;
    while (renderer.pollEvent(e))
      if (e.type == sf::Event::Closed)
        return 0;

    acc += clock.restart().asSeconds();
    while (acc >= step) {
      engine.update(step);
      acc -= step;
    }

    renderer.drawParticles();
  }
  return 0;
}
