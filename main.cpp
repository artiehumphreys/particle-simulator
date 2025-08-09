#include "physics.hpp"
#include "render/render.hpp"

int main() {
  sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  int width = desktop.width;
  int height = desktop.height - 54;
  PhysicsEngine engine(width, height);
  render::Renderer renderer(engine, width, height);

  for (int i = 0; i < 600; ++i)
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
