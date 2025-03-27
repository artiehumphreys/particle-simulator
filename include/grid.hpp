#pragma once

#include "common.hpp"

template <typename T> struct Grid {

  struct Cell {
    vec2 position;
    vec<T> points;

    Cell(float x, float y) : position{x, y} {}

    int getPointCount() { return points.size(); }

    void addPoint(const vec2 &p) { points.push_back(p); }

    bool removePoint(int id) {
      for (auto it = points.begin(); it != points.end(); ++it) {
        if (it->id != id)
          continue;
        std::iter_swap(it, points.end() - 1);
        points.pop_back();
        return true;
      }
      return false;
    }
  };

  vec<vec<Cell>> cells;
  int32_t width, height;
  static constexpr int8_t cellSize = 2;
  int32_t rows, cols;
  Grid(int32_t width_, int32_t height_) : width(width_), height(height_) {
    cols = (width + cellSize - 1) / cellSize;
    rows = (height + cellSize - 1) / cellSize;
    createGrid();
  }

  void createGrid() {
    cells.clear();
    cells.reserve(rows);
    for (size_t i = 0; i < rows; ++i) {
      vec<Cell> row;
      row.reserve(cols);
      for (size_t j = 0; j < cols; ++j) {
        row.emplace_back(i, j);
      }
      cells.push_back(std::move(row));
    }
  }

  const Cell &get(const vec2 &p) const {
    return get(static_cast<int32_t>(p.x), static_cast<int32_t>(p.y));
  }

  const Cell &get(int32_t x, int32_t y) const {
    if (!areCoordsValid(x, y))
      return nullptr;
    return cells[y][x];
  }

  void set(const vec2 &p, const T &cell) {
    set(static_cast<int32_t>(p.x), static_cast<int32_t>(p.y), cell);
  }

  void set(int32_t x, int32_t y, const T &cell) {
    if (!areCoordsValid(x, y))
      return;
    cells[y][x] = &cell;
  }

  bool areCoordsValid(const vec2 &p) {
    return areCoordsValid(static_cast<int32_t>(p.x), static_cast<int32_t>(p.y));
  }

  bool areCoordsValid(int32_t x, int32_t y) {
    return static_cast<int32_t>(x) >= 0 && static_cast<int32_t>(x) < (rows) &&
           static_cast<int32_t>(y) >= 0 && static_cast<int32_t>(y) < (cols);
  }
};
