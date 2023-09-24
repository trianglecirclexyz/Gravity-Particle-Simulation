#include <iostream>
#include <cmath>
#include <random>
#include <format>
#include <raylib-cpp.hpp>

#include "quad_tree.hpp"
#include "particle.hpp"

int screen_w = 2*800;
int screen_h = 2*450;
raylib::Window window(screen_w, screen_h, "raylib [core] example - basic window");

raylib::Font font("res/fonts/Hack-Regular.ttf", 20);\
raylib::Color text_colour = raylib::Color::White();

raylib::Color background(0, 0, 10, 0);

int main() {

    // SetTargetFPS(60);

    // Create a 2D camera
    raylib::Camera2D cam;
    cam.target = raylib::Vector2(static_cast<float>(screen_w) / 2, static_cast<float>(screen_h) / 2);
    cam.offset = raylib::Vector2(static_cast<float>(screen_w) / 2, static_cast<float>(screen_h) / 2);
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    Rectangle camera_bounds = {
        -cam.offset.x / cam.zoom + cam.target.x, 
        -cam.offset.y / cam.zoom + cam.target.y,
        screen_w / cam.zoom,
        screen_h / cam.zoom
    };

    std::vector<Particle> particle_instances;

    // Spawn particles
    for(int i = 0; i < screen_w; i += 45) {
        for(int j = 0; j < screen_w; j += 45) {
            Particle particle(i, j);

            // assign each particle a random speed
            {
                std::random_device rd;
                std::mt19937 gen(rd()); 
                std::uniform_real_distribution<double> distribution(-400.0, 400.0);
                particle.vel.x = distribution(gen) / 10;
                particle.vel.y = distribution(gen) / 10;    
            }
            // assign velocities according to a spiral shape
            {
                raylib::Vector2 delta;
                delta.x = cam.offset.x - particle.pos.x; 
                delta.y = cam.offset.y - particle.pos.y;

                raylib::Vector2 vel_vector;
                vel_vector.x = delta.y;
                vel_vector.y = -1*delta.x;
                vel_vector.x *= 0.2;
                vel_vector.y *= 0.2;

                particle.vel += vel_vector;
            }

            particle_instances.push_back(particle);
        }
    }

    double simulation_speed = 1;

    bool isMiddleMouseButtonDown = false;
    raylib::Vector2 lastMousePosition;

    while (!window.ShouldClose()) {   // Detect window close button or ESC key

        // double dt = simulation_speed*GetFrameTime(); // Get the delta time
        // std::cout << "dt: " << dt << std::endl;
        double dt = 0.016; // Set the delta time to be consistent at 60 fps 

        // ** Calculations ** //

        // construct new Quad Tree
        Quad boundary(
            camera_bounds.x - 10 * camera_bounds.width,
            camera_bounds.y - 10 * camera_bounds.width,
            camera_bounds.width * 20,
            camera_bounds.width * 20);
        QuadTree quad_tree(boundary, 2);

        // Reset acceleration for all particles to zero at the start of each frame
        // Add the particles to the quad_tree
        // Remove particle if outside of quad tree boundary
        // TODO do the acceleration calcs here or something 
        for (int i = 0; i < particle_instances.size(); i++) {

            Point particle_point(particle_instances[i].pos.x, particle_instances[i].pos.y);
            if (!quad_tree.GetBoundary().Contains(particle_point)) {
                particle_instances.erase(particle_instances.begin() + i);
                continue;
            }

            particle_instances[i].accel.x = 0;
            particle_instances[i].accel.y = 0;

            quad_tree.Insert(particle_point);

        }

        // Update existing particle instances after calculating the accelerations due to gravity
        for (int i = 0; i < particle_instances.size(); i++) {
            Particle& p_i = particle_instances[i];
            for (int j = i + 1; j < particle_instances.size(); j++) {
                    Particle& p_j = particle_instances[j];
                    p_i.CalcAccel(p_j, dt);
                }
        }
        for (int i = 0; i < particle_instances.size(); i++) {
            particle_instances[i].Update(dt);
        }

        // ** Input Handling ** //

        raylib::Vector2 mouse_pos(
            static_cast<float>(raylib::Mouse::GetX()), 
            static_cast<float>(raylib::Mouse::GetY()));
        
        // position relative to camera
        raylib::Vector2 adj_mouse_pos(
            (1 / cam.zoom) * (mouse_pos.x - cam.offset.x) + cam.target.x, 
            (1 / cam.zoom) * (mouse_pos.y - cam.offset.y) + cam.target.y);
            
        // std::cout << "mouse_pos: " << mouse_pos.x << ", " << mouse_pos.y << std::endl;
        // std::cout << "adj pos: " << adj_mouse_pos.x << ", " << adj_mouse_pos.y << std::endl;

        if (raylib::Mouse::IsButtonDown(MOUSE_LEFT_BUTTON)) {
            Particle particle(adj_mouse_pos);
            particle_instances.push_back(particle);

        }
        if (raylib::Mouse::IsButtonDown(MOUSE_RIGHT_BUTTON)) {
            // hehe
            Particle particle(adj_mouse_pos);
            particle.mass = -particle.mass;
            particle_instances.push_back(particle);
        }
        if (IsKeyPressed(KEY_C)) {
            particle_instances.clear();
            std::cout << "Screen cleared" << std::endl;
        }

        // middle mouse button panning
        if (raylib::Mouse::IsButtonDown(MOUSE_MIDDLE_BUTTON)) {
            if (!isMiddleMouseButtonDown) {
                lastMousePosition = mouse_pos;
                isMiddleMouseButtonDown = true;
            }
            else {
                raylib::Vector2 currentMousePosition = mouse_pos;
                raylib::Vector2 mouseDelta = currentMousePosition - lastMousePosition;
                cam.target.x -= (1 / cam.zoom) * mouseDelta.x;
                cam.target.y -= (1 / cam.zoom) * mouseDelta.y;
                
                lastMousePosition = currentMousePosition;
            }
        }
        else {
            isMiddleMouseButtonDown = false;
        }

        // scroll to zoom in and out 
        cam.zoom += 0.05*raylib::Mouse::GetWheelMove();
        if(cam.zoom < 0.1) {
            cam.zoom = 0.1;
        }
 
        // ** Rendering ** //

        BeginDrawing(); {

            window.ClearBackground(background);

            cam.BeginMode(); // start drawing to camera

            // Draw all particle instances
            for(const Particle& particle : particle_instances) {
                particle.Draw();
            }

            // Draw the quadtree 
            quad_tree.Draw(cam);

            cam.EndMode(); // stop drawing to camera

            // Draw FPS
            std::string fps_text = "FPS: " + std::to_string(window.GetFPS());
            text_colour.DrawText(font, fps_text.c_str(), {10, 10}, 20, 0);
            // std::cout << fps_text << std::endl;

            // Draw number of particles
            std::string num_particles_text = "Particles: " + std::to_string(particle_instances.size());
            text_colour.DrawText(font, num_particles_text.c_str(), {10, 30}, 20, 0);

            // Draw simulation speed
            std::string zoom_text = "Zoom: " + std::format("{:.2f}", cam.zoom);
            text_colour.DrawText(font, zoom_text.c_str(), {10, 50}, 20, 0);

            // Draw keymap legend
            text_colour.DrawText(font, "hi", {10, static_cast<float>(screen_h - 20)}, 20, 0);
        }
        EndDrawing();

        // std::cout << "Frame end" << std::endl;
    }
 
    return 0;
}