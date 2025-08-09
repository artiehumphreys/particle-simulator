#pragma once

#include "common.hpp"
#include "grid.hpp"
#include "particle.hpp"
#include <iostream>

#define GRAVITY 20.0f

struct PhysicsEngine {
  Grid<Particle> collisionGrid;
  vec<Particle> particles;
  // list of dead (free) particles that have been removed and can be recycled
  vec<int> freeList;
  int32_t width, height;
  int8_t subSteps = 8; // collision resolution

  PhysicsEngine(int32_t width_, int32_t height_)
      : collisionGrid{width_, height_}, width(width_), height(height_) {}

  void solveCollision(Particle &p1, Particle &p2) {
    // idea: take the difference between their summed radii and actual
    // difference between them. shift them away along axis of intersection
    // difference / 2 units
    constexpr float response_coef = 1.0f;
    // https://github.com/johnBuffer/VerletSFML-Multithread
    constexpr float epsilon = 1e-4;
    const float expectedDistance = static_cast<float>(p1.radius + p2.radius);
    vec2 axis = p1.position - p2.position;
    const float squaredDistance = axis.x * axis.x + axis.y * axis.y;

    if (squaredDistance < expectedDistance * expectedDistance &&
        squaredDistance > epsilon) {
      const float actualDistance = std::sqrt(squaredDistance);
      const float overlap = expectedDistance - actualDistance;
      const float delta = response_coef * 0.5f * overlap;

      const vec2 col_vec = (axis / actualDistance) * delta;

      p1.position += col_vec;
      p2.position -= col_vec;
    }
  }

  void processNeighboringCells(int row, int col) {
    int R = collisionGrid.rows;
    int C = collisionGrid.cols;
    auto &current = collisionGrid.cells[row][col].particleIndices;
    size_t nCurrent = current.size();
    if (current.empty())
      return;

    static constexpr int DIRS[5][2] = {{0, 0}, {0, 1}, {1, 0}, {1, -1}, {1, 1}};

    for (size_t i = 0; i < sizeof(DIRS) / sizeof(DIRS[0]); ++i) {
      int newRow = row + DIRS[i][0];
      int newCol = col + DIRS[i][1];
      if (!collisionGrid.areCoordsValid(newCol, newRow))
        continue;
      const auto &cellNeighbor =
          collisionGrid.cells[newRow][newCol].particleIndices;
      size_t nNeighbor = cellNeighbor.size();

      if (nNeighbor == 0)
        continue;

      auto processSameCellCollisions = [this](const vec<int> &particles,
                                              size_t nParticles) {
        for (size_t a = 0; a < nParticles; ++a) {
          int idx1 = particles[a];
          for (size_t b = a + 1; b < nParticles; ++b) {
            int idx2 = particles[b];
            solveCollision(this->particles[idx1], this->particles[idx2]);
          }
        }
      };

      auto processCrossCellCollisions =
          [this](const vec<int> &current, size_t nCurrent,
                 const vec<int> &neighbor, int nNeighbor) {
            for (size_t i = 0; i < nCurrent; ++i) {
              int idx1 = current[i];
              for (size_t j = 0; j < nNeighbor; ++j) {
                int idx2 = neighbor[j];
                solveCollision(particles[idx1], particles[idx2]);
              }
            }
          };

      if (newRow == row && newCol == col) {
        processSameCellCollisions(current, nCurrent);
      } else {
        processCrossCellCollisions(current, nCurrent, cellNeighbor, nNeighbor);
      }
    }
  }

  void checkAllCollisions() {
    for (int row = 0; row < collisionGrid.rows; ++row) {
      for (int col = 0; col < collisionGrid.cols; ++col) {
        processNeighboringCells(row, col);
      }
    }
  }

  void updateObjects(float dt) {
    float maxX = static_cast<float>(width);
    float maxY = static_cast<float>(height);
    for (Particle &p : particles) {
      p.acceleration += vec2{0.0f, GRAVITY};
      p.update(dt);
      p.clampPosition(maxX, maxY);
    }
    updateCellOwnership();
  }

  void updateCellOwnership() {
    for (int i = 0; i < particles.size(); ++i) {
      Particle &p = particles[i];
      vec2 oldIndex =
          collisionGrid.getGridIndex(static_cast<int32_t>(p.lastPosition.x),
                                     static_cast<int32_t>(p.lastPosition.y));

      vec2 newIndex =
          collisionGrid.getGridIndex(static_cast<int32_t>(p.position.x),
                                     static_cast<int32_t>(p.position.y));

      if (newIndex != oldIndex) {
        if (collisionGrid.areCoordsValid(oldIndex.x, oldIndex.y)) {
          collisionGrid.removeParticle(i, oldIndex.x, oldIndex.y);
        }
        if (collisionGrid.areCoordsValid(newIndex.x, newIndex.y)) {
          collisionGrid.insertParticle(i, newIndex.x, newIndex.y);
        }
      }
    }
  }

  void update(float dt) {
    float subDt = dt / static_cast<float>(subSteps);
    float maxX = static_cast<float>(width);
    float maxY = static_cast<float>(height);
    for (int i = 0; i < subSteps; ++i) {
      updateObjects(subDt);
      checkAllCollisions();
      for (auto &p : particles) {
        p.clampPosition(maxX, maxY);
      }
      updateCellOwnership();
    }
  }

  int addParticle(const vec2 &pos) {
    int idx;
    if (!freeList.empty()) {
      // there are particles that can be recycled
      idx = freeList.back();
      freeList.pop_back();
      particles[idx] = Particle{pos.x, pos.y};
    } else {
      idx = particles.size();
      particles.emplace_back(pos.x, pos.y);
    }

    particles[idx].id = idx;

    vec2 cell = collisionGrid.getGridIndex(pos.x, pos.y);
    int col = cell.x;
    int row = cell.y;

    if (collisionGrid.areCoordsValid(col, row)) {
      collisionGrid.cells[row][col].addParticle(idx);
    }

    return idx;
  }

  bool removeParticle(int id) {
    if (0 < id || id >= particles.size())
      return false;

    Particle &remove = particles[id];
    vec2 cell =
        collisionGrid.getGridIndex(remove.position.x, remove.position.y);

    int col = cell.x;
    int row = cell.y;

    int backIndex = particles.size() - 1;
    if (id == backIndex) {
      if (collisionGrid.cells[row][col].removeParticle(id)) {
        particles.pop_back();
        return true;
      }
      return false;
    }

    freeList.push_back(id);
    return true;
  }
};
