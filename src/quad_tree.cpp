#include "quad_tree.hpp"

QuadTree::QuadTree(const Quad &boundary, int capacity) :
    boundary(boundary), 
    capacity(capacity),
    points(),
    divided(false),
    is_master(true) {}

QuadTree::QuadTree(const Quad &boundary, int capacity, bool is_master) :
    boundary(boundary), 
    capacity(capacity),
    points(),
    divided(false),
    is_master(is_master) {}

void QuadTree::Subdivide() {
    
    divided = true;

    int x = boundary.x;
    int y = boundary.y;
    int w = boundary.width;
    int h = boundary.height;

    Quad ne_boundary = Quad(x + w/2, y, w/2, h/2);
    ne = std::make_unique<QuadTree>(ne_boundary, capacity, false);

    Quad nw_boundary = Quad(x, y, w/2, h/2);
    nw = std::make_unique<QuadTree>(nw_boundary, capacity, false);

    Quad se_boundary = Quad(x + w/2, y + h/2, w/2, h/2);
    se = std::make_unique<QuadTree>(se_boundary, capacity, false);

    Quad sw_boundary = Quad(x, y + h/2, w/2, h/2);
    sw = std::make_unique<QuadTree>(sw_boundary, capacity, false);
}

bool QuadTree::Insert(const Point &point) {

    if (!boundary.Contains(point)) {
        return false;
    }

    if (points.size() < capacity) {
        points.push_back(point);
        return true;
    }
    else {
        if (!divided) {
            Subdivide();
        }

        if (ne->Insert(point)) return true;
        if (nw->Insert(point)) return true;
        if (se->Insert(point)) return true;
        if (sw->Insert(point)) return true;
    }

    return false;
}

void QuadTree::Draw(raylib::Camera2D cam) const {

    Rectangle rect = {boundary.x, boundary.y, 
        boundary.width, boundary.height};
    if (!is_master) {
        DrawRectangleLinesEx(rect, 1 / cam.zoom, raylib::Color(0, 255, 0, 255 / 12));
    }
    else {
        DrawRectangleLinesEx(rect, 1 / cam.zoom, raylib::Color(255, 0, 0, 255));
    }

    if (ne) ne->Draw(cam);
    if (nw) nw->Draw(cam);
    if (se) se->Draw(cam);
    if (sw) sw->Draw(cam);
}

const Quad& QuadTree::GetBoundary() const {
    return boundary;
}