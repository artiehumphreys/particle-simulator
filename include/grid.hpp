#pragma once

#include "common.hpp"
#include <cmath>
#include <cstdint>

template <typename T> struct Grid {

  struct Cell {
    vec2 position;
    // each cell keeps track of particles contained with it.
    vec<int> particleIndices;

    Cell(float x, float y) : position{x, y} {}

    int getParticleCount() { return particleIndices.size(); }

    void addParticle(const int idx) { particleIndices.push_back(idx); }

    bool removeParticle(int idx) {
      for (auto it = particleIndices.begin(); it != particleIndices.end();
           ++it) {
        if (*it != idx)
          continue;
        std::iter_swap(it, particleIndices.end() - 1);
        particleIndices.pop_back();
        return true;
      }
      return false;
    }
  };

  vec<vec<Cell>> cells;
  int32_t width, height;
  static constexpr int8_t cellSize = 8;
  int32_t rows, cols;
  Grid(int32_t width_, int32_t height_) : width(width_), height(height_) {
    cols = (width + cellSize - 1) / cellSize;
    rows = (height + cellSize - 1) / cellSize;
    createGrid();
  }

  void createGrid() {
    cells.clear();
    cells.reserve(rows);
    for (int32_t i = 0; i < rows; ++i) {
      vec<Cell> row;
      row.reserve(cols);
      for (int32_t j = 0; j < cols; ++j) {
        row.emplace_back(j, i);
      }
      cells.push_back(std::move(row));
    }
  }

  static vec2 getGridIndex(int32_t x, int32_t y) {
    float newX = static_cast<float>(std::floor(x / cellSize));
    float newY = static_cast<float>(std::floor(y / cellSize));
    return vec2{newX, newY};
  }

  void insertParticle(const int idx, int32_t x, int32_t y) {
    vec2 newCoords = getGridIndex(x, y);
    if (!areCoordsValid(newCoords.x, newCoords.y))
      return;
    cells[static_cast<int>(newCoords.y)][static_cast<int>(newCoords.x)]
        .addParticle(idx);
  }

  bool removeParticle(const int idx, int32_t x, int32_t y) {
    vec2 newCoords = getGridIndex(x, y);
    if (!areCoordsValid(newCoords.x, newCoords.y))
      return false;
    return cells[static_cast<int>(newCoords.y)][static_cast<int>(newCoords.x)]
        .removeParticle(idx);
  }

  const Cell *get(int32_t x, int32_t y) const {
    if (!areCoordsValid(x, y))
      return nullptr;
    return &cells[y][x];
  }

  bool areCoordsValid(int32_t col, int32_t row) {
    return static_cast<int32_t>(row) >= 0 &&
           static_cast<int32_t>(row) < (rows) &&
           static_cast<int32_t>(col) >= 0 && static_cast<int32_t>(col) < (cols);
  }
};
