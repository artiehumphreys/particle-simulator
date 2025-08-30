#include "common.hpp"
#include "particle.hpp"
#include "physics.hpp"
#include "render/render.hpp"
#include "thread_pool/thread_pool.hpp"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <thread>

static inline float frand(float a, float b) {
  return a + (b - a) * (float(std::rand()) / float(RAND_MAX));
}

static inline void setInitialVelocity(Particle &p, const vec2 &v0, float dt) {
  p.lastPosition = p.position - v0 * dt;
}

struct ParticleDropper {
  uint32_t remaining;
  float rate;
  float acc = 0.f;
  float minX, maxX;
  float spawnY;
  float vDown;
  float vJitterX = 2.0f;

  ParticleDropper(uint32_t total, float pps, float minX_, float maxX_,
                  float spawnY_, float vDown_)
      : remaining(total), rate(pps), minX(minX_), maxX(maxX_), spawnY(spawnY_),
        vDown(vDown_) {}

  void update(float dt, PhysicsEngine &engine, float verletDt) {
    if (remaining == 0)
      return;
    acc += rate * dt;
    uint32_t n = (uint32_t)acc; // spawn this many now
    if (n == 0)
      return;
    if (n > remaining)
      n = remaining;
    acc -= float(n);

    for (uint32_t i = 0; i < n; ++i) {
      float x = frand(minX, maxX);
      uint32_t id = engine.addParticle({x, spawnY});
      Particle &p = engine.particles[id];
      vec2 v0{frand(-vJitterX, vJitterX), vDown};
      setInitialVelocity(p, v0, verletDt);
    }
    remaining -= n;
  }
};

int main() {
  sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  uint32_t width = desktop.width;
  uint32_t height = desktop.height - 54;
  tp::ThreadPool pool(std::thread::hardware_concurrency());
  PhysicsEngine engine(width, height, pool);
  render::Renderer renderer(engine, width, height);

  const uint32_t targetCount = 12500;
  engine.particles.reserve(targetCount);

  // for (int i = 0; i < numParticles; ++i)
  // engine.addParticle(
  // {float(std::rand() % width), float(std::rand() % height)});

  const float step = 1 / 60.f;

  ParticleDropper emitter(targetCount, 1000.f, width * 0.15f, width * 0.85f,
                          -100.0f, 100.0f);
  sf::Clock clock;
  float acc = 0.f;

  while (renderer.isOpen()) {
    sf::Event e;
    while (renderer.pollEvent(e))
      if (e.type == sf::Event::Closed)
        return 0;

    acc += clock.restart().asSeconds();
    while (acc >= step) {
      emitter.update(step, engine, step / 10);
      engine.update(step);
      acc -= step;
    }

    renderer.drawParticles();
  }
  return 0;
}
