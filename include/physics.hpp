#pragma once

#include "common.hpp"
#include "grid.hpp"
#include "particle.hpp"
#include "thread_pool/thread_pool.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

#define GRAVITY 15.0f

struct PhysicsEngine {
  Grid<Particle> collisionGrid;
  std::vector<Particle> particles;
  // list of dead (free) particles that have been removed and can be recycled
  std::vector<uint32_t> freeList;
  uint32_t width, height;
  uint8_t subSteps = 4; // collision resolution

  tp::ThreadPool &pool;

  static constexpr int8_t DIRS[5][2] = {
      {0, 0}, {0, 1}, {1, 0}, {1, -1}, {1, 1}};

  PhysicsEngine(uint32_t width_, uint32_t height_, tp::ThreadPool &pool_)
      : collisionGrid{width_, height_}, width(width_), height(height_),
        pool(pool_) {}

  inline bool bandHasWork(uint32_t r0, uint32_t r1) const {
    const auto &active = collisionGrid.rowHasActive;
    if (active.empty())
      return true;
    r1 = std::min(r1, static_cast<uint32_t>(active.size() - 1));
    for (uint32_t r = r0; r <= r1; ++r)
      if (active[r])
        return true;
    if (r1 + 1 < active.size() && active[r1 + 1])
      return true;
    return false;
  }

  void solveCollision(Particle &p1, Particle &p2) {
    // idea: take the difference between their summed radii and actual
    // difference between them. shift them away along axis of intersection
    // difference / 2 units
    constexpr float responseCoef = 1.0f;
    // https://github.com/johnBuffer/VerletSFML-Multithread
    constexpr float epsilon = 1e-4;
    const float expectedDistance = static_cast<float>(p1.radius + p2.radius);
    vec2 axis = p1.position - p2.position;
    const float squaredDistance = axis.x * axis.x + axis.y * axis.y;

    if (squaredDistance < expectedDistance * expectedDistance &&
        squaredDistance > epsilon) {
      const float actualDistance = std::sqrt(squaredDistance);
      const float overlap = expectedDistance - actualDistance;
      const float delta = responseCoef * 0.5f * overlap;

      const vec2 colVec = (axis / actualDistance) * delta;

      p1.position += colVec;
      p2.position -= colVec;
    }
  }

  inline void processSameCellCollisions(const std::vector<uint32_t> &ids) {
    const uint32_t n = static_cast<uint32_t>(ids.size());
    for (uint32_t a = 0; a + 1 < n; ++a) {
      const uint32_t ia = ids[a];
      for (uint32_t b = a + 1; b < n; ++b) {
        const uint32_t ib = ids[b];
        solveCollision(particles[ia], particles[ib]);
      }
    }
  }

  inline void processCrossCellCollisions(const std::vector<uint32_t> &A,
                                         const std::vector<uint32_t> &B) {
    const uint32_t nA = static_cast<uint32_t>(A.size());
    const uint32_t nB = static_cast<uint32_t>(B.size());
    for (uint32_t i = 0; i < nA; ++i) {
      const uint32_t ia = A[i];
      for (uint32_t j = 0; j < nB; ++j) {
        const uint32_t ib = B[j];
        solveCollision(particles[ia], particles[ib]);
      }
    }
  }

  void processNeighboringCells(uint32_t row, uint32_t col) {
    const auto &current = collisionGrid.at(row, col).particleIndices;
    if (current.empty())
      return;

    for (uint8_t i = 0; i < sizeof(DIRS) / sizeof(DIRS[0]); ++i) {
      const int32_t newRow = static_cast<int32_t>(row) + DIRS[i][0];
      const int32_t newCol = static_cast<int32_t>(col) + DIRS[i][1];
      if (!collisionGrid.areCoordsValid(newCol, newRow))
        continue;

      const auto &cellNeighbor =
          collisionGrid
              .at(static_cast<uint32_t>(newRow), static_cast<uint32_t>(newCol))
              .particleIndices;
      const uint32_t nNeighbor = static_cast<uint32_t>(cellNeighbor.size());
      if (nNeighbor == 0)
        continue;

      if (newRow == static_cast<int32_t>(row) &&
          newCol == static_cast<int32_t>(col)) {
        processSameCellCollisions(current);
      } else {
        processCrossCellCollisions(current, cellNeighbor);
      }
    }
  }

  void processNeighboringCells_band(uint32_t beginRow, uint32_t endRow,
                                    uint32_t row, uint32_t col) {
    const auto &current = collisionGrid.at(row, col).particleIndices;
    const uint32_t nCurrent = static_cast<uint32_t>(current.size());
    if (nCurrent == 0)
      return;

    for (uint8_t i = 0; i < sizeof(DIRS) / sizeof(DIRS[0]); ++i) {
      const int32_t newRow = static_cast<int32_t>(row) + DIRS[i][0];
      const int32_t newCol = static_cast<int32_t>(col) + DIRS[i][1];
      if (newRow < beginRow || newRow > endRow)
        continue;
      if (!collisionGrid.areCoordsValid(newCol, newRow))
        continue;

      const auto &cellNeighbor =
          collisionGrid
              .at(static_cast<uint32_t>(newRow), static_cast<uint32_t>(newCol))
              .particleIndices;
      uint32_t nNeighbor = static_cast<int32_t>(cellNeighbor.size());

      if (nNeighbor == 0)
        continue;

      if (row == newRow && col == newCol) {
        for (uint32_t i = 0; i + 1 < nCurrent; ++i) {
          for (uint32_t j = i + 1; j < nCurrent; ++j) {
            solveCollision(particles[current[i]], particles[current[j]]);
          }
        }
      } else {
        for (uint32_t i = 0; i < nCurrent; ++i) {
          for (uint32_t j = 0; j < nNeighbor; ++j) {
            solveCollision(particles[current[i]], particles[cellNeighbor[j]]);
          }
        }
      }
    }
  }

  void processBand(uint32_t beginRow, uint32_t endRow) {
    for (uint32_t row = beginRow; row <= endRow; ++row) {
      if (!collisionGrid.rowHasActive[row])
        continue;
      const uint32_t c0 = collisionGrid.rowMinCol[row],
                     c1 = collisionGrid.rowMaxCol[row];
      for (uint32_t col = c0; col <= c1; ++col) {
        processNeighboringCells_band(beginRow, endRow, row, col);
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
          collisionGrid.getGridIndex(static_cast<uint32_t>(p.lastPosition.x),
                                     static_cast<uint32_t>(p.lastPosition.y));

      vec2 newIndex =
          collisionGrid.getGridIndex(static_cast<uint32_t>(p.position.x),
                                     static_cast<uint32_t>(p.position.y));

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

  void checkAllCollisions() {
    for (int row = 0; row < collisionGrid.rows; ++row) {
      for (int col = 0; col < collisionGrid.cols; ++col) {
        processNeighboringCells(row, col);
      }
    }
  }

  void checkAllCollisions_rowChunkWavefront(uint32_t rowsPerTask = 2) {
    const uint32_t R = collisionGrid.rows, C = collisionGrid.cols;
    rowsPerTask = std::max<uint32_t>(1, rowsPerTask);

    auto stripe = [&](uint32_t startRow) {
      for (uint32_t row = startRow; row < R; row += rowsPerTask) {
        const uint32_t endRow =
            std::min<uint32_t>(R - 1, row + rowsPerTask - 1);
        if (!bandHasWork(row, endRow))
          continue;
        pool.addTask([this, row, endRow] { processBand(row, endRow); });
      }
      pool.waitIdle();
    };

    stripe(0);
    stripe(1);
  }

  void update(float dt) {
    float subDt = dt / static_cast<float>(subSteps);
    float maxX = static_cast<float>(width);
    float maxY = static_cast<float>(height);
    for (int i = 0; i < subSteps; ++i) {
      updateObjects(subDt);
      // checkAllCollisions();
      checkAllCollisions_rowChunkWavefront();
      for (auto &p : particles) {
        p.clampPosition(maxX, maxY);
      }
      updateCellOwnership();
    }
  }

  int addParticle(const vec2 &pos) {
    uint32_t idx;
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
    uint32_t col = cell.x;
    uint32_t row = cell.y;

    if (collisionGrid.areCoordsValid(col, row)) {
      collisionGrid.at(row, col).addParticle(idx);
    }

    return idx;
  }

  bool removeParticle(uint32_t id) {
    if (0 < id || id >= particles.size())
      return false;

    Particle &remove = particles[id];
    vec2 cell =
        collisionGrid.getGridIndex(remove.position.x, remove.position.y);

    uint32_t col = cell.x;
    uint32_t row = cell.y;

    uint32_t backIndex = particles.size() - 1;
    if (id == backIndex) {
      if (collisionGrid.at(row, col).removeParticle(id)) {
        particles.pop_back();
        return true;
      }
      return false;
    }

    freeList.push_back(id);
    return true;
  }
};
