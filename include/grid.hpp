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
    std::vector<uint32_t> particleIndices;

    Cell(float x, float y) : position{x, y} {}

    uint32_t getParticleCount() const noexcept {
      return static_cast<uint32_t>(particleIndices.size());
    }

    void addParticle(uint32_t idx) { particleIndices.push_back(idx); }

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

  std::vector<Cell> cells;
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

  inline uint32_t idx(uint32_t r, uint32_t c) const { return r * cols + c; }

  inline Cell &at(uint32_t r, uint32_t c) { return cells[idx(r, c)]; }
  inline const Cell &at(uint32_t r, uint32_t c) const {
    return cells[idx(r, c)];
  }

  void createGrid() {
    cells.clear();
    cells.reserve(static_cast<size_t>(rows) * static_cast<size_t>(cols));
    for (uint32_t r = 0; r < rows; ++r) {
      for (uint32_t c = 0; c < cols; ++c) {
        cells.emplace_back(static_cast<float>(c), static_cast<float>(r));
      }
    }

    rowHasActive.assign(rows, 0);
    rowNonEmptyCount.assign(rows, 0);
    // min > max => empty row
    rowMinCol.assign(rows, cols);
    rowMaxCol.assign(rows, 0);
  }

  inline void recomputeRowMin(uint32_t r, uint32_t from) {
    uint32_t c = from;
    while (c < cols && at(r, c).empty())
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
    int64_t c = static_cast<int64_t>(from);
    while (c >= 0 && at(r, static_cast<uint32_t>(c)).empty())
      --c;
    if (c < 0) {
      rowMinCol[r] = 1;
      rowMaxCol[r] = 0;
      rowHasActive[r] = 0;
    } else {
      rowMaxCol[r] = static_cast<uint32_t>(c);
    }
  }

  static vec2 getGridIndex(uint32_t x, uint32_t y) {
    const float gx =
        std::floor(static_cast<double>(x) / static_cast<double>(cellSize));
    const float gy =
        std::floor(static_cast<double>(y) / static_cast<double>(cellSize));
    return vec2{gx, gy};
  }

  void insertParticle(uint32_t idxP, uint32_t x, uint32_t y) {
    vec2 gi = getGridIndex(x, y);
    if (!areCoordsValid(static_cast<int32_t>(gi.x), static_cast<int32_t>(gi.y)))
      return;

    const uint32_t r = static_cast<uint32_t>(gi.y);
    const uint32_t c = static_cast<uint32_t>(gi.x);

    Cell &cell = at(r, c);
    const bool wasEmpty = cell.empty();
    cell.addParticle(idxP);

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

  bool removeParticle(uint32_t idxP, uint32_t x, uint32_t y) {
    vec2 gi = getGridIndex(x, y);
    if (!areCoordsValid(static_cast<int32_t>(gi.x), static_cast<int32_t>(gi.y)))
      return false;

    const uint32_t r = static_cast<uint32_t>(gi.y);
    const uint32_t c = static_cast<uint32_t>(gi.x);

    Cell &cell = at(r, c);
    const bool removed = cell.removeParticle(idxP);
    if (!removed)
      return false;

    if (cell.empty()) {
      const uint32_t cnt = --rowNonEmptyCount[r];
      if (cnt == 0) {
        rowHasActive[r] = 0;
        rowMinCol[r] = 1;
        rowMaxCol[r] = 0;
      } else {
        if (c == rowMinCol[r])
          recomputeRowMin(r, c + 1);
        if (c == rowMaxCol[r])
          recomputeRowMax(r, (c == 0 ? 0u : c - 1));
      }
    }
    return true;
  }

  const Cell *get(int32_t x, int32_t y) const {
    if (!areCoordsValid(x, y))
      return nullptr;
    return &at(static_cast<uint32_t>(y), static_cast<uint32_t>(x));
  }

  bool areCoordsValid(const vec2 &coords) const {
    return areCoordsValid(static_cast<int32_t>(coords.x),
                          static_cast<int32_t>(coords.y));
  }

  bool areCoordsValid(int32_t col, int32_t row) const {
    return col >= 0 && col < static_cast<int32_t>(cols) && row >= 0 &&
           row < static_cast<int32_t>(rows);
  }
};
