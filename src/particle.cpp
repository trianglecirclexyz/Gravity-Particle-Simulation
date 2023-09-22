#include "particle.hpp"

const double G = 6.674 * 100; // modified gravitational constant

Particle::Particle(raylib::Vector2 pos) : 
            pos(pos), 
            vel(0.0f, 0.0f),
            accel(0.0f, 0.0f), 
            size(1), 
            colour(RAYWHITE) {
                mass = 1000*size;
            }

Particle::Particle(int pos_x, int  pos_y) : 
    vel(0.0f, 0.0f),
    accel(0.0f, 0.0f), 
    size(1), 
    colour(RAYWHITE) {
        pos.x = pos_x;
        pos.y = pos_y;

        mass = 1000*size;
    }

void Particle::CalcAccel(Particle& extern_particle, double dt) {

    // Calculate the distance between the particles
    double d_x = extern_particle.pos.x - pos.x;
    double d_y = extern_particle.pos.y - pos.y;

    // Calculate the squared distance
    double distance_squared = d_x * d_x + d_y * d_y;
    double distance = sqrt(distance_squared);

    // Don't allow particles to accelerate arbitrarily by adding a limit
    if (size + extern_particle.size > distance / 20) {
        return;
    }

    // Calculate the gravitational force magnitude
    double force_magnitude = (G * mass * extern_particle.mass) / distance_squared;

    // Calculate the direction of the force as a unit vector
    double force_direction_x = d_x / distance;
    double force_direction_y = d_y / distance;

    // Calculate the acceleration due to gravity for both particles
    double tmp_accel_x = force_magnitude * force_direction_x / mass;
    double tmp_accel_y = force_magnitude * force_direction_y / mass;

    // Update the acceleration for the current particle (this)
    accel.x += tmp_accel_x * dt;
    accel.y += tmp_accel_y * dt;

    // Update the acceleration for the external particle (extern_particle)
    extern_particle.accel.x += -tmp_accel_x * dt;
    extern_particle.accel.y += -tmp_accel_y * dt;
}

void Particle::Update(double dt) {

    vel.x += accel.x * dt;
    vel.y += accel.y * dt;

    // TODO: update particle colour so that it changes with speed
    // std::cout << sqrt((accel.x*accel.x)+(accel.y*accel.y)) << std::endl;
    double mag_vel = sqrt((vel.x*vel.x)+(vel.y*vel.y));
    double k = 0.01; // smoothness factor
    if (mass > 0) {
        colour = raylib::Color(255*k*mag_vel / (1+k*mag_vel), 0, 255, 255);
    } 
    else {
        colour = raylib::Color(255*k*mag_vel / (1+k*mag_vel), 255, 0, 255);
    }

    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    // std::cout << "position: " << pos.x << ", " << pos.y << std::endl;
    // std::cout << "velocity: " << vel.x << ", " << vel.y << std::endl;
    // std::cout << "acceleration: " << accel.x << ", " << accel.y << std::endl;
}

void Particle::Draw() const {

    // Draw lines for velocity and acceleration
    int sf1 = 1; // vel line scale factor
    int sf2 = 2; // accel line scale factor
    // DrawLine(pos.x, pos.y, pos.x+sf1*vel.x, pos.y+sf1*vel.y, raylib::Color::Blue());
    // DrawLine(pos.x, pos.y, pos.x+sf2*accel.x, pos.y+sf2*accel.y, raylib::Color::Red());

    DrawCircle(pos.x, pos.y, size, colour);
}