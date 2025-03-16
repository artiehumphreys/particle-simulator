#include "common.hpp"
#include "particle.hpp"

template <typename T>
struct Grid {

    struct Cell {
        vec2 position;
        vec<T> points;

        Cell(float x, float y) : position {x, y} {}

        void addPoint(const T& p) {
            points.push_back(p);
        }

    };

    vec<vec<Cell>> cells;
    // width * height squares
    int32_t width, height;
    Grid(int32_t width_, int32_t height_) : width(width_), height(height_) {
        createGrid();
    }

    void createGrid() {
        cells.clear();
        cells.reserve(height);
        for (size_t i = 0; i < height; ++i) {
            vec<Cell> row;
            row.reserve(width);
            for (size_t j = 0; j < width; ++j) {
                row.emplace_back(i, j);
            }
            cells.push_back(std::move(row));
        }
    }

    template <typename vec2Type>
    const Cell& get(const vec2Type& p) const {
        return get(static_cast<int32_t>(p.x), static_cast<int32_t>(p.y));
    }

    const Cell& get(int32_t x, int32_t y) const {
        return cells[y][x];
    }

    template <typename vec2Type>
    void set (const vec2Type& p, const T& cell) {
        set(static_cast<int32_t>(p.x), static_cast<int32_t>(p.y), cell);
    }

    void set(int32_t x, int32_t y, const T& cell) {
        cells[y][x] = &cell;
    }

};
