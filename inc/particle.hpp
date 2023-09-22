#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <cmath>
#include <raylib-cpp.hpp>

struct Particle {
    Particle(raylib::Vector2 pos);
    Particle(int pos_x, int pos_y);

    void CalcAccel(Particle& extern_particle, double dt);
    void Update(double dt);
    void Draw() const;

    raylib::Vector2 pos;
    raylib::Vector2 vel;
    raylib::Vector2 accel;
    double size;
    double mass;
    raylib::Color colour;
};

#endif // PARTICLE_HPP
