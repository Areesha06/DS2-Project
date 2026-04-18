#pragma once
#include <vector>
#include "Point.h"
#include "Rectangle.h"

// Maximum number of points a node holds before it subdivides
const int QT_CAPACITY = 4;
// Maximum recursion depth to prevent infinite subdivision
const int QT_MAX_DEPTH = 8;

class Quadtree {
public:
    Rectangle boundary;
    std::vector<Point> points;
    bool divided;
    int depth;

    // Four child quadrants
    Quadtree* NE; // North-East
    Quadtree* NW; // North-West
    Quadtree* SE; // South-East
    Quadtree* SW; // South-West

    Quadtree(Rectangle boundary, int depth = 0);
    ~Quadtree();

    // Insert a point into the quadtree
    bool insert(const Point& p);

    // Range query: find all points within a given rectangle
    void queryRange(const Rectangle& range, std::vector<Point>& found) const;

    // Point query: check if an exact point exists
    bool queryPoint(const Point& p) const;

    // Remove all points and collapse subdivisions
    void clear();

    // Total number of points in this subtree
    int count() const;

    // Collect all boundary rectangles (for visualization)
    void getAllBoundaries(std::vector<Rectangle>& rects) const;

private:
    void subdivide();
};
