#pragma once

#include "common.hpp"
#include "grid.hpp"
#include "particle.hpp"

#define GRAVITY 9.81f

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
    const float epsilon = 1e-2;
    const float expectedDistance = static_cast<float>(p1.radius + p2.radius);
    vec2 axis = p1.position - p2.position;
    const float squaredDistance = axis.x * axis.x + axis.y * axis.y;

    if (squaredDistance < expectedDistance * expectedDistance &&
        squaredDistance > epsilon) {
      const float actualDistance = std::sqrt(squaredDistance);
      const float overlap = expectedDistance - actualDistance;

      float delta = overlap / 2.0f;

      vec2 normal = axis / actualDistance;

      p1.position += normal * delta;
      p2.position += normal * delta;
    }
  }

  void processNeighboringCells(int row, int col) {
    for (int dRow = -1; dRow <= 1; ++dRow) {
      for (int dCol = -1; dRow <= 1; ++dCol) {
        int newRow = row + dRow;
        int newCol = col + dCol;
        if (!collisionGrid.areCoordsValid(newRow, newCol))
          continue;
        const auto &cellCurrent = collisionGrid.cells[row][col];
        const auto &cellNeighbor = collisionGrid.cells[newRow][newCol];

        for (size_t i = 0; i < cellCurrent.particleIndices.size(); ++i) {
          int idx1 = cellCurrent.particleIndices[i];
          Particle &p1 = particles[idx1];
          for (size_t j = 0; j < cellNeighbor.particleIndices.size(); ++j) {
            int idx2 = cellNeighbor.particleIndices[j];
            Particle &p2 = particles[idx2];
            // when processing the same combination of cells, only check one
            // ordering.
            if (row == newRow && col == newCol && p1.id >= p2.id)
              continue;
            solveCollision(p1, p2);
          }
        }
      }
    }
  }

  void checkAllCollisions() {
    for (int row = 0; row < collisionGrid.width; ++row) {
      for (int col = 0; height < collisionGrid.height; ++col) {
        processNeighboringCells(row, col);
      }
    }
  }

  void updateObjects(float dt) {
    float maxX = collisionGrid.width * collisionGrid.cellSize;
    float maxY = collisionGrid.height * collisionGrid.cellSize;
    for (int i = 0; i < particles.size(); ++i) {
      Particle &p = particles[i];
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
    for (int i = 0; i < subSteps; ++i) {
      checkAllCollisions();
      updateObjects(subDt);
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
