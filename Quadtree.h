#pragma once
#include <vector>
#include "Point.h"
#include "Rectangle.h"

// max number of points a node holds before it subdivides
const int QT_CAPACITY = 4;
// Maximum recursion depth to prevent infinite subdivision
const int QT_MAX_DEPTH = 8;

class Quadtree {
public:
    Rectangle boundary; // region covered by this node
    std::vector<Point> points; // Points stored in this node
    bool divided; // Whether this node has been subdivided
    int depth; // Current depth in the tree

    // Four child quadrants
    Quadtree* NE; // North-East (top right)
    Quadtree* NW; // North-West (top left)
    Quadtree* SE; // South-East (bottom right)
    Quadtree* SW; // South-West (bottom left)

    Quadtree(Rectangle boundary, int depth = 0); // Constructor
    ~Quadtree(); // destructor

    // insert a point into the quadtree
    bool insert(const Point& p);

    // find all points within a given rectangle
    void queryRange(const Rectangle& range, std::vector<Point>& found) const;

    // check if an exact point exists
    bool queryPoint(const Point& p) const;

    // remove all points (for reset function)
    void clear();

    // total number of points in this subtree
    int count() const;

    // Collect all boundary rectangles
    void getAllBoundaries(std::vector<Rectangle>& rects) const;

private:
    void subdivide(); // split current node into 4 child quadrants
};
