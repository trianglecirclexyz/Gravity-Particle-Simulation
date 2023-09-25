#include <iostream>
#include <cmath>
#include <random>
#include <filesystem>
#include <format>
#include <thread>
#include <raylib-cpp.hpp>

#include "quad_tree.hpp"
#include "particle.hpp"

void mt_CalcParticleAccels(std::vector<Particle>& particles, double dt, int start, int end) {
    for (int i = start; i < end; i++) {
        Particle& p_i = particles[i];
        for (int j = i + 1; j < particles.size(); j++) {
            Particle& p_j = particles[j];
            p_i.CalcAccel(p_j, dt);
        }
    }
}

void mt_UpdateParticles(std::vector<Particle>& particles, double dt, int start, int end) {
    for (int i = start; i < end; i++) {
        particles[i].Update(dt);
    }
}

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
    int square_dim = 400;
    raylib::Vector2 center = {screen_w / 2, screen_h / 2};
    for(int i = center.x - square_dim / 2; i < center.x + square_dim / 2; i += 5) {
        for(int j = center.y - square_dim / 2; j < center.y + square_dim / 2; j += 5) {
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
                vel_vector.x *= 0.5;
                vel_vector.y *= 0.5;

                particle.vel += vel_vector;
            }

            particle_instances.push_back(particle);
        }
    }

    double sim_speed = 0.25;

    bool isMiddleMouseButtonDown = false;
    raylib::Vector2 lastMousePosition;

    Quad boundary(
        camera_bounds.x - 10 * camera_bounds.width,
        camera_bounds.y - 10 * camera_bounds.width,
        camera_bounds.width * 20,
        camera_bounds.width * 20);

    int frame_count = 0;

    int target_fps = 60;
    int target_frame = 4*600;

    double dt = sim_speed / target_fps; // Set the delta time to be consistent at 60 fps

    while (!window.ShouldClose()) {   // Detect window close button or ESC key

        // double dt = simulation_speed*GetFrameTime(); // Get the delta time
        // std::cout << "dt: " << dt << std::endl;

        // ** Calculations ** //

        // construct new Quad Tree
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

            // quad_tree.Insert(particle_point);

        }

        // Calculate particle accelerations in parallel
        int numThreads = std::thread::hardware_concurrency(); // Get the number of available CPU cores
        std::vector<std::thread> threads;
        int particlesPerThread = particle_instances.size() / numThreads;
        int start = 0;
        int end = 0;

        // Create and start threads
        for (int i = 0; i < numThreads; i++) {
            start = i * particlesPerThread;
            end = (i == numThreads - 1) ? particle_instances.size() : (i + 1) * particlesPerThread;
            threads.emplace_back(mt_CalcParticleAccels, std::ref(particle_instances), dt, start, end);
        }

        // Wait for threads to finish
        for (std::thread& t : threads) {
            t.join();
        }

        // After all accelerations are calculated, update particles in parallel
        std::vector<std::thread> updateThreads;

        // Create and start threads for updating particles
        for (int i = 0; i < numThreads; i++) {
            start = i * particlesPerThread;
            end = (i == numThreads - 1) ? particle_instances.size() : (i + 1) * particlesPerThread;
            updateThreads.emplace_back(mt_UpdateParticles, std::ref(particle_instances), dt, start, end);
        }

        // Wait for update threads to finish
        for (std::thread& t : updateThreads) {
            t.join();
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
            // quad_tree.Draw(cam);

            // save frames
            std::string filename = std::format("frames/frame_{:04d}.png", frame_count);
            raylib::TakeScreenshot(filename.c_str());

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

        frame_count++;

        if (frame_count == target_frame) {
            break;
        }
    }

    // stitch frames into video
    std::string input_pattern = "frames/frame_%04d.png";
    std::string output = "output.mp4";

    std::string ffmpeg_cmd = std::format(
        "ffmpeg -y -framerate {} -i {} -c:v libx264 -r {} -pix_fmt yuv420p {}",
        target_fps, input_pattern, target_fps, output);

    int result = std::system(ffmpeg_cmd.c_str());

        if (result == 0) {
        std::cout << "Video rendered successfully." << std::endl;
    } else {
        std::cerr << "Video failed to render." << std::endl;
        return 1;
    }
    
    // frames cleanup
    for (const auto& entry : std::filesystem::directory_iterator("frames")) {
        std::filesystem::remove(entry.path());
    }

    return 0;
}