#pragma once

#include "common.hpp"
#include "grid.hpp"
#include "particle.hpp"

#define GRAVITY 9.81f

struct PhysicsEngine {
  Grid<Particle> collisionGrid;
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
        for (Particle &p1 : collisionGrid.cells[row][col].points) {
          for (Particle &p2 : collisionGrid.cells[newRow][newCol].points) {
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
    for (int row = 0; row < collisionGrid.width; ++row) {
      for (int col = 0; height < collisionGrid.height; ++col) {
        for (Particle &p : collisionGrid.cells[row][col].points) {
          p.acceleration += {0.0f, GRAVITY};
          p.update(dt);
          p.clampPosition(maxX, maxY);
        }
      }
    }
  }

  void update(float dt) {
    int subDt = dt / static_cast<float>(subSteps);
    for (int i = 0; i < subSteps; ++i) {
      checkAllCollisions();
      updateObjects(subDt);
    }
  }
};
