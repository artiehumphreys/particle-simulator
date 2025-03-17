#include "common.hpp"
#include "particle.hpp"

template <typename T>
struct Grid {

    struct Cell {
        vec2 position;
        vec<T> points;
        int32_t pointCount = 0;

        Cell(float x, float y) : position {x, y} {}

        void addPoint(const vec2& p) {
            points.push_back(p);
            ++pointCount;
        }

        bool removePoint(int id) {
            for (auto it = points.begin(); it != points.end(); ++it){
                if (it->id != id) continue;
                std::iter_swap(it, points.end() - 1);
                points.pop_back();
                --pointCount;
                return true;
            }
            return false;
        }

    };

    vec<vec<Cell>> cells;
    int32_t width, height;
    static constexpr int8_t cellSize = 2;
    int32_t rows = height / cellSize, cols = width / cellSize;
    Grid(int32_t width_, int32_t height_) : width(width_), height(height_) {
        createGrid();
    }

    void createGrid() {
        cells.clear();
        cells.reserve(height);
        for (size_t i = 0; i < rows; ++i) {
            vec<Cell> row;
            row.reserve(rows);
            for (size_t j = 0; j < cols; ++j){
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

    template <typename vec2Type>
    bool areCoordsValid (const vec2Type& p) {
        return areCoordsValid(static_cast<int32_t>(p.x), static_cast<int32_t>(p.y));
    }

    bool areCoordsValid(int32_t x, int32_t y) {
        return static_cast<int32_t>(x) > 0 && static_cast<int32_t>(x) < (width - 1) &&
               static_cast<int32_t>(y) > 0 && static_cast<int32_t>(y) < (height - 1);
    }

};
