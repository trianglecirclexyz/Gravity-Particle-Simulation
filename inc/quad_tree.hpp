#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <vector>
#include <memory>

#include <raylib-cpp.hpp>

struct Point {

    int x, y;

    Point(int x, int y) : 
        x(x), 
        y(y) {}

};

struct Quad {
    int x, y, width, height;

    Quad(int x, int y, int width, int height) : 
        x(x), 
        y(y), 
        width(width), 
        height(height) {}

    bool Contains(Point point) const {
        // check if the point is are in the quad
        return (
            point.x >= x &&
            point.x <= x + width &&
            point.y >= y &&
            point.y <= y + height);
    }

};

class QuadTree {

    Quad boundary;
    int capacity; // maximum capacity before subdividing
    std::vector<Point> points;
    bool divided;
    bool is_master;

    std::unique_ptr<QuadTree> ne; // northeast
    std::unique_ptr<QuadTree> nw; 
    std::unique_ptr<QuadTree> se; 
    std::unique_ptr<QuadTree> sw; 

    public:
        QuadTree(const Quad &quad, int capacity);
        QuadTree(const Quad &quad, int capacity, bool is_master);

        void Subdivide();
        bool Insert(const Point &point);
        // TODO Traverse function
        void Draw(raylib::Camera2D cam) const;

        const Quad& GetBoundary() const;
};

#endif // QUADTREE_HPP
