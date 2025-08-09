#pragma once

#include "common.hpp"
#include <cmath>

template <typename T> struct Grid {

  struct Cell {
    vec2 position;
    // each cell keeps track of particles contained with it.
    vec<uint32_t> particleIndices;

    Cell(float x, float y) : position{x, y} {}

    uint32_t getParticleCount() { return particleIndices.size(); }

    void addParticle(const uint32_t idx) { particleIndices.push_back(idx); }

    bool removeParticle(uint32_t idx) {
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
  uint32_t width, height;
  static constexpr uint8_t cellSize = 8;
  uint32_t rows, cols;
  Grid(uint32_t width_, uint32_t height_) : width(width_), height(height_) {
    cols = (width + cellSize - 1) / cellSize;
    rows = (height + cellSize - 1) / cellSize;
    createGrid();
  }

  void createGrid() {
    cells.clear();
    cells.reserve(rows);
    for (uint32_t i = 0; i < rows; ++i) {
      vec<Cell> row;
      row.reserve(cols);
      for (uint32_t j = 0; j < cols; ++j) {
        row.emplace_back(j, i);
      }
      cells.push_back(std::move(row));
    }
  }

  static vec2 getGridIndex(uint32_t x, uint32_t y) {
    float newX = std::floor(static_cast<double>(x) / cellSize);
    float newY = std::floor(static_cast<double>(y) / cellSize);
    return vec2{newX, newY};
  }

  void insertParticle(const uint32_t idx, uint32_t x, uint32_t y) {
    vec2 newCoords = getGridIndex(x, y);
    if (!areCoordsValid(newCoords.x, newCoords.y))
      return;
    cells[static_cast<uint32_t>(newCoords.y)]
         [static_cast<uint32_t>(newCoords.x)]
             .addParticle(idx);
  }

  bool removeParticle(const uint32_t idx, uint32_t x, uint32_t y) {
    vec2 newCoords = getGridIndex(x, y);
    if (!areCoordsValid(newCoords.x, newCoords.y))
      return false;
    return cells[static_cast<uint32_t>(newCoords.y)]
                [static_cast<uint32_t>(newCoords.x)]
                    .removeParticle(idx);
  }

  const Cell *get(int32_t x, int32_t y) const {
    if (!areCoordsValid(x, y)) {
      return nullptr;
    }
    return &cells[y][x];
  }

  bool areCoordsValid(vec2 &coords) {
    return areCoordsValid(static_cast<int32_t>(coords.x),
                          static_cast<int32_t>(coords.y));
  }

  bool areCoordsValid(int32_t col, int32_t row) {
    return col >= 0 && col < cols && row >= 0 && row < rows;
  }
};
