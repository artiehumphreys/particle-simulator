#pragma once

#include "common.hpp"
#include "grid.hpp"
#include "particle.hpp"

struct PhysicsEngine {
    Grid<Particle> collisionGrid;
    int32_t width, height;
    int8_t sub_steps = 8; // collision resolution

    PhysicsEngine (int32_t width_, int32_t height_) : collisionGrid{width_, height_}, width(width_), height(height_) {}


};
