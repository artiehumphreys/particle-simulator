#pragma once

#include "common.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

template <typename T> struct Grid {

  struct Cell {
    vec2 position;
    // each cell keeps track of particles contained with it.
    std::vector<uint32_t> particleIndices;

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

    bool empty() const noexcept { return particleIndices.empty(); }
  };

  std::vector<std::vector<Cell>> cells;
  uint32_t width, height;
  static constexpr uint8_t cellSize = 4;
  uint32_t rows, cols;

  std::vector<uint8_t> rowHasActive;
  std::vector<uint32_t> rowMinCol;
  std::vector<uint32_t> rowMaxCol;
  std::vector<uint32_t> rowNonEmptyCount;

  Grid(uint32_t width_, uint32_t height_) : width(width_), height(height_) {
    cols = (width + cellSize - 1) / cellSize;
    rows = (height + cellSize - 1) / cellSize;
    createGrid();
  }

  void createGrid() {
    cells.clear();
    cells.reserve(rows);
    for (uint32_t i = 0; i < rows; ++i) {
      std::vector<Cell> row;
      row.reserve(cols);
      for (uint32_t j = 0; j < cols; ++j) {
        row.emplace_back(j, i);
      }
      cells.push_back(std::move(row));
    }

    rowHasActive.assign(rows, 0);
    rowNonEmptyCount.assign(rows, 0);
    // min > max => empty
    rowMinCol.assign(rows, cols);
    rowMaxCol.assign(rows, 0);
  }

  inline void recomputeRowMin(uint32_t r, uint32_t from) {
    uint32_t c = from;
    while (c < cols && cells[r][c].empty())
      ++c;
    if (c >= cols) {
      rowMinCol[r] = 1;
      rowMaxCol[r] = 0;
      rowHasActive[r] = 0;
    } else {
      rowMinCol[r] = c;
    }
  }

  inline void recomputeRowMax(uint32_t r, uint32_t from) {
    // guard underflow
    int64_t c = (int64_t)from;
    while (c >= 0 && cells[r][(uint32_t)c].empty())
      --c;
    if (c < 0) {
      rowMinCol[r] = 1;
      rowMaxCol[r] = 0;
      rowHasActive[r] = 0;
    } else {
      rowMaxCol[r] = (uint32_t)c;
    }
  }

  static vec2 getGridIndex(uint32_t x, uint32_t y) {
    float newX = std::floor(static_cast<double>(x) / cellSize);
    float newY = std::floor(static_cast<double>(y) / cellSize);
    return vec2{newX, newY};
  }

  void insertParticle(uint32_t idx, uint32_t x, uint32_t y) {
    vec2 gi = getGridIndex(x, y);
    if (!areCoordsValid((int32_t)gi.x, (int32_t)gi.y))
      return;

    uint32_t r = (uint32_t)gi.y;
    uint32_t c = (uint32_t)gi.x;

    auto &cell = cells[r][c];
    bool wasEmpty = cell.empty();
    cell.addParticle(idx);

    if (wasEmpty) {
      if (rowNonEmptyCount[r]++ == 0) {
        rowHasActive[r] = 1;
        rowMinCol[r] = c;
        rowMaxCol[r] = c;
      } else {
        if (c < rowMinCol[r])
          rowMinCol[r] = c;
        if (c > rowMaxCol[r])
          rowMaxCol[r] = c;
      }
    }
  }

  bool removeParticle(uint32_t idx, uint32_t x, uint32_t y) {
    vec2 gi = getGridIndex(x, y);
    if (!areCoordsValid((int32_t)gi.x, (int32_t)gi.y))
      return false;

    uint32_t r = (uint32_t)gi.y;
    uint32_t c = (uint32_t)gi.x;

    auto &cell = cells[r][c];
    bool removed = cell.removeParticle(idx);
    if (!removed)
      return false;

    if (cell.empty()) {
      uint32_t cnt = --rowNonEmptyCount[r];
      if (cnt == 0) {
        rowHasActive[r] = 0;
        rowMinCol[r] = 1;
        rowMaxCol[r] = 0;
      } else {
        if (c == rowMinCol[r])
          recomputeRowMin(r, c + 1);
        if (c == rowMaxCol[r]) {
          recomputeRowMax(r, (c == 0 ? 0u : c - 1));
        }
      }
    }
    return true;
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
